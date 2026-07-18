#include "archsynth/evaluation/evaluator.h"

#include <algorithm>
#include <stdexcept>

namespace archsynth {
Evaluator::Evaluator(std::unique_ptr<ProxyTrainer> trainer, Scenario scenario)
    : trainer_(std::move(trainer)), scenario_(std::move(scenario)) {
  if (!trainer_) throw std::invalid_argument("evaluator requires a trainer");
}

double Evaluator::hardware_penalty(const HardwareProfile& hardware) const {
  double penalty = 0.0;
  const auto memory = scenario_.constraints.find("max_memory_mb");
  if (memory != scenario_.constraints.end())
    penalty += std::max(0.0, hardware.estimated_memory_bytes / 1048576.0 - memory->second);
  const auto latency = scenario_.constraints.find("max_latency_ms");
  if (latency != scenario_.constraints.end())
    penalty += std::max(0.0, hardware.estimated_latency_ms - latency->second);
  return penalty / 1000.0;
}

double Evaluator::evaluate(const Genotype& genotype) const {
  return alpha_ * trainer_->train_and_evaluate(genotype) -
         beta_ * hardware_penalty(compute_hardware_profile(genotype));
}

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
