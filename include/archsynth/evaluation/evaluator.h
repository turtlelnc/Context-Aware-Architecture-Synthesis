#pragma once
#include "archsynth/conditioning/scenario.h"
#include "archsynth/evaluation/hardware_cost.h"
#include "archsynth/evaluation/proxy_trainer.h"
#include <memory>
namespace archsynth { class Evaluator{public: Evaluator(std::unique_ptr<ProxyTrainer> trainer,Scenario scenario); double evaluate(const Genotype& genotype) const; std::vector<double> evaluate_population(const std::vector<Genotype>& genotypes) const; private: std::unique_ptr<ProxyTrainer> trainer_; Scenario scenario_; double alpha_=1.0,beta_=1.0; double hardware_penalty(const HardwareProfile& hw) const;}; }
