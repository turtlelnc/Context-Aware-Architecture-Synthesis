#pragma once

#include "archsynth/conditioning/scenario.h"
#include "archsynth/evaluation/hardware_cost.h"
#include "archsynth/evaluation/proxy_trainer.h"

#include <memory>
#include <string>
#include <vector>

namespace archsynth {
struct FitnessBreakdown {
  double quality_score = 0.0;
  double memory_penalty = 0.0;
  double latency_penalty = 0.0;
  double redundancy_penalty = 0.0;
  double pattern_penalty = 0.0;
  double depth_penalty = 0.0;
  double final_fitness = 0.0;
  HardwareProfile hardware;
  std::vector<std::string> findings;
  std::string to_json() const;
};

class Evaluator {
 public:
  Evaluator(std::unique_ptr<ProxyTrainer> trainer, Scenario scenario);
  FitnessBreakdown score(const Genotype& genotype) const;
  double evaluate(const Genotype& genotype) const;
  std::vector<double> evaluate_population(const std::vector<Genotype>& genotypes) const;

 private:
  std::unique_ptr<ProxyTrainer> trainer_;
  Scenario scenario_;
};
}  // namespace archsynth
