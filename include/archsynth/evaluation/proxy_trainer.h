#pragma once
#include "archsynth/core/genotype.h"
#include <string>
namespace archsynth { class ProxyTrainer{public: ProxyTrainer(std::string dataset_path="",int batch_size=8,int epochs=2); double train_and_evaluate(const Genotype& genotype) const; private: std::string dataset_path_; int batch_size_; int epochs_;}; }
