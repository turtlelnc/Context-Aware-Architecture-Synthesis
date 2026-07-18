#pragma once
#include "archsynth/core/genotype.h"
#include <random>
namespace archsynth { Genotype crossover(const Genotype& a,const Genotype& b,std::mt19937& rng); }
