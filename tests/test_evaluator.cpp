#include "archsynth/evaluation/evaluator.h"

#include <cassert>
#include <cmath>
#include <stdexcept>

int main() {
  const archsynth::Scenario scenario{
      "code_generation", "mobile", {{"max_memory_mb", 3072}, {"max_latency_ms", 15}}, "proxy"};
  archsynth::Evaluator evaluator(std::make_unique<archsynth::ProxyTrainer>(), scenario);
  const auto minimal = archsynth::make_minimal_genotype();
  const auto score = evaluator.score(minimal);
  assert(std::isfinite(score.final_fitness));
  assert(score.quality_score >= 0.0 && score.quality_score <= 1.0);
  assert(score.pattern_penalty > 0.0);
  assert(std::abs(evaluator.evaluate(minimal) - score.final_fitness) < 1e-12);
  assert(score.hardware.total_params == 32000 * 256);
  assert(!score.to_json().empty());

  auto repeated = minimal;
  repeated.nodes.push_back({"norm1", "rms_norm", {}});
  repeated.nodes.push_back({"norm2", "rms_norm", {}});
  repeated.edges = {{"input", "n0"}, {"n0", "norm1"}, {"norm1", "norm2"}, {"norm2", "output"}};
  const auto repeated_score = evaluator.score(repeated);
  assert(repeated_score.redundancy_penalty >= 0.14);
  assert(repeated_score.final_fitness < repeated_score.quality_score);

  auto unnormalized_attention = minimal;
  unnormalized_attention.nodes.push_back({"attention", "attention_linear", {}});
  unnormalized_attention.edges = {{"input", "n0"}, {"n0", "attention"}, {"attention", "output"}};
  const auto attention_score = evaluator.score(unnormalized_attention);
  assert(attention_score.pattern_penalty >= 0.06);

  bool rejected = false;
  try {
    archsynth::Evaluator invalid(std::make_unique<archsynth::ProxyTrainer>(),
                                 {"code_generation", "", {{"max_latency_ms", 0}}, "proxy"});
  } catch (const std::invalid_argument&) { rejected = true; }
  assert(rejected);

  archsynth::Evaluator constrained(
      std::make_unique<archsynth::ProxyTrainer>(),
      {"code_generation", "", {{"max_memory_mb", 1}, {"max_latency_ms", 0.1}}, "proxy"});
  const auto constrained_score = constrained.score(minimal);
  assert(constrained_score.memory_penalty > 0.0);
  assert(constrained_score.latency_penalty > 0.0);
  assert(constrained_score.findings.size() >= 2);

  auto larger_embedding = minimal;
  larger_embedding.nodes[0].hyperparams = {{"vocab_size", 64000}, {"dim", 256}};
  const auto larger_hardware = archsynth::compute_hardware_profile(larger_embedding);
  assert(larger_hardware.total_params == 2 * score.hardware.total_params);
  assert(larger_hardware.estimated_memory_bytes == 2 * score.hardware.estimated_memory_bytes);
}
