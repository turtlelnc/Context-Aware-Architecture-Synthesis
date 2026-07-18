#pragma once
#include "archsynth/conditioning/scenario.h"
#include "archsynth/core/genotype.h"
#include <random>
namespace archsynth { class LLMMutator{public: LLMMutator(std::string api_key="",std::string endpoint="",std::string model="gpt-4o"); std::vector<Genotype> propose_mutations(const Genotype& best,const Scenario& scenario,double fitness,std::mt19937& rng) const; private: std::string build_prompt(const Genotype&,const Scenario&,double) const; std::string api_key_,endpoint_,model_;}; }
