#pragma once
#include "archsynth/core/genotype.h"
#include <memory>
namespace archsynth { struct BuiltModel{ Genotype genotype; std::string summary() const; }; class ModelBuilder{public: static std::shared_ptr<BuiltModel> build(const Genotype& genotype);}; }
