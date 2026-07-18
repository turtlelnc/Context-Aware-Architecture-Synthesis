#pragma once
#include "archsynth/search/mutations.h"
namespace archsynth { class Population{public: static std::vector<Genotype> initialize(int size,std::mt19937& rng,const ConditionVector& cond={});}; }
