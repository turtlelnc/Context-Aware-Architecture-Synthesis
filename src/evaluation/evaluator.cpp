#include "archsynth/evaluation/evaluator.h"
#include "archsynth/core/json.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace archsynth {
namespace {
double required_positive_constraint(const Scenario& scenario, const std::string& name) {
  const auto it = scenario.constraints.find(name);
  if (it == scenario.constraints.end()) return 0.0;
  if (!std::isfinite(it->second) || it->second <= 0.0)
    throw std::invalid_argument("constraint '" + name + "' must be finite and positive");
  return it->second;
}
bool is_norm(const std::string& name) { return name == "rms_norm"; }
bool is_attention(const std::string& name) {
  return name == "attention_std" || name == "attention_linear";
}
bool is_linear(const std::string& name) {
  return name == "linear" || name == "linear_lowrank";
}
}  // namespace

std::string FitnessBreakdown::to_json() const {
  std::ostringstream output;
  output << std::setprecision(17);
  output << "{\"quality_score\":" << quality_score
         << ",\"memory_penalty\":" << memory_penalty
         << ",\"latency_penalty\":" << latency_penalty
         << ",\"redundancy_penalty\":" << redundancy_penalty
         << ",\"pattern_penalty\":" << pattern_penalty
         << ",\"depth_penalty\":" << depth_penalty
         << ",\"final_fitness\":" << final_fitness
         << ",\"hardware\":{\"parameters\":" << hardware.total_params
         << ",\"flops_per_token\":" << hardware.total_flops
         << ",\"memory_mb\":" << hardware.estimated_memory_bytes / 1048576.0
         << ",\"latency_ms\":" << hardware.estimated_latency_ms << "},\"findings\":[";
  for (std::size_t i = 0; i < findings.size(); ++i) {
    if (i) output << ',';
    output << '"' << json::escape(findings[i]) << '"';
  }
  output << "]}";
  return output.str();
}

Evaluator::Evaluator(std::unique_ptr<ProxyTrainer> trainer, Scenario scenario)
    : trainer_(std::move(trainer)), scenario_(std::move(scenario)) {
  if (!trainer_) throw std::invalid_argument("evaluator requires a trainer");
  (void)required_positive_constraint(scenario_, "max_memory_mb");
  (void)required_positive_constraint(scenario_, "max_latency_ms");
}

FitnessBreakdown Evaluator::score(const Genotype& genotype) const {
  FitnessBreakdown result;
  std::string graph_error;
  if (!genotype.to_graph().is_valid(graph_error)) {
    result.quality_score = 0.0;
    result.pattern_penalty = 1.0;
    result.final_fitness = -1.0;
    result.findings.push_back("invalid graph: " + graph_error);
    return result;
  }
  result.hardware = compute_hardware_profile(genotype);
  result.quality_score = trainer_->train_and_evaluate(genotype);

  const double memory_limit = required_positive_constraint(scenario_, "max_memory_mb");
  const double latency_limit = required_positive_constraint(scenario_, "max_latency_ms");
  const double memory_mb = result.hardware.estimated_memory_bytes / 1048576.0;
  if (memory_limit > 0.0) {
    const double utilization = memory_mb / memory_limit;
    result.memory_penalty = 0.02 * std::min(1.0, utilization) +
                            0.60 * std::max(0.0, utilization - 1.0);
    if (utilization > 1.0) result.findings.push_back("memory constraint exceeded");
  }
  if (latency_limit > 0.0) {
    const double utilization = result.hardware.estimated_latency_ms / latency_limit;
    result.latency_penalty = 0.05 * std::min(1.0, utilization) +
                             0.80 * std::max(0.0, utilization - 1.0);
    if (utilization > 1.0) result.findings.push_back("latency constraint exceeded");
  }

  std::unordered_map<std::string, std::string> primitive_by_id;
  for (const auto& node : genotype.nodes) primitive_by_id[node.id] = node.primitive_name;
  int attention_count = 0;
  int nonlinearity_count = 0;
  int norm_count = 0;
  for (const auto& node : genotype.nodes) {
    attention_count += is_attention(node.primitive_name) ? 1 : 0;
    nonlinearity_count += (node.primitive_name == "swiglu" || node.primitive_name == "gate") ? 1 : 0;
    norm_count += is_norm(node.primitive_name) ? 1 : 0;
  }
  for (const auto& edge : genotype.edges) {
    const auto source = primitive_by_id.find(edge.first);
    const auto destination = primitive_by_id.find(edge.second);
    if (source == primitive_by_id.end() || destination == primitive_by_id.end()) continue;
    if (source->second == destination->second) {
      result.redundancy_penalty += 0.06;
      result.findings.push_back("repeated adjacent primitive: " + source->second);
    }
    if (is_norm(source->second) && is_norm(destination->second))
      result.redundancy_penalty += 0.08;
    if (is_attention(source->second) && is_attention(destination->second))
      result.redundancy_penalty += 0.05;
    if (is_linear(source->second) && is_linear(destination->second))
      result.redundancy_penalty += 0.025;
  }
  for (const auto& node : genotype.nodes) {
    if (!is_attention(node.primitive_name)) continue;
    bool adjacent_norm = false;
    for (const auto& edge : genotype.edges) {
      if (edge.second == node.id) {
        const auto source = primitive_by_id.find(edge.first);
        adjacent_norm |= source != primitive_by_id.end() && is_norm(source->second);
      }
      if (edge.first == node.id) {
        const auto destination = primitive_by_id.find(edge.second);
        adjacent_norm |= destination != primitive_by_id.end() && is_norm(destination->second);
      }
    }
    if (!adjacent_norm) {
      result.pattern_penalty += 0.06;
      result.findings.push_back("attention layer has no adjacent normalization: " + node.id);
    }
  }
  if (attention_count > 3) {
    result.pattern_penalty += 0.03 * static_cast<double>(attention_count - 3);
    result.findings.push_back("excessive attention layers for the proxy search budget");
  }
  if (nonlinearity_count > std::max(2, attention_count * 2)) {
    result.pattern_penalty += 0.025 * static_cast<double>(nonlinearity_count - std::max(2, attention_count * 2));
    result.findings.push_back("nonlinear layers are imbalanced relative to attention layers");
  }
  if (scenario_.task_type == "code_generation") {
    if (attention_count == 0) {
      result.pattern_penalty += 0.18;
      result.findings.push_back("code-generation architecture has no attention layer");
    }
    if (nonlinearity_count == 0) {
      result.pattern_penalty += 0.12;
      result.findings.push_back("architecture has no nonlinear transformation");
    }
    if (norm_count == 0) {
      result.pattern_penalty += 0.08;
      result.findings.push_back("architecture has no normalization layer");
    }
  }
  const int depth = static_cast<int>(genotype.nodes.size());
  if (depth < 4) result.depth_penalty = 0.03 * static_cast<double>(4 - depth);
  if (depth > 16) result.depth_penalty = 0.025 * static_cast<double>(depth - 16);
  result.redundancy_penalty = std::min(result.redundancy_penalty, 0.50);
  result.pattern_penalty = std::min(result.pattern_penalty, 0.50);
  result.depth_penalty = std::min(result.depth_penalty, 0.50);
  result.final_fitness = result.quality_score - result.memory_penalty -
                         result.latency_penalty - result.redundancy_penalty -
                         result.pattern_penalty - result.depth_penalty;
  if (!std::isfinite(result.final_fitness))
    throw std::runtime_error("non-finite fitness produced");
  return result;
}

double Evaluator::evaluate(const Genotype& genotype) const { return score(genotype).final_fitness; }

std::vector<double> Evaluator::evaluate_population(const std::vector<Genotype>& genotypes) const {
  std::vector<double> result(genotypes.size());
#ifdef _OPENMP
#pragma omp parallel for if(genotypes.size() > 8)
#endif
  for (int i = 0; i < static_cast<int>(genotypes.size()); ++i)
    result[static_cast<std::size_t>(i)] = evaluate(genotypes[static_cast<std::size_t>(i)]);
  return result;
}
}  // namespace archsynth
