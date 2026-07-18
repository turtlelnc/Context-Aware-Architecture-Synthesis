#pragma once
#include "archsynth/core/genotype.h"
#include <memory>
#include <random>
namespace archsynth { using ConditionVector=std::vector<float>;
class Mutation{public: virtual ~Mutation()=default; virtual bool apply(Genotype&,std::mt19937&,const ConditionVector& cond={})=0;};
class AddNodeMutation: public Mutation{public: bool apply(Genotype&,std::mt19937&,const ConditionVector& cond = {}) override;};
class RemoveNodeMutation: public Mutation{public: bool apply(Genotype&,std::mt19937&,const ConditionVector& cond = {}) override;};
class ChangePrimitiveMutation: public Mutation{public: bool apply(Genotype&,std::mt19937&,const ConditionVector& cond = {}) override;};
class ChangeHyperparamMutation: public Mutation{public: bool apply(Genotype&,std::mt19937&,const ConditionVector& cond = {}) override;};
class AddEdgeMutation: public Mutation{public: bool apply(Genotype&,std::mt19937&,const ConditionVector& cond = {}) override;};
class RemoveEdgeMutation: public Mutation{public: bool apply(Genotype&,std::mt19937&,const ConditionVector& cond = {}) override;};
class CompositeMutation: public Mutation{public: void add_mutation(std::unique_ptr<Mutation> mut,double weight=1.0); bool apply(Genotype&,std::mt19937&,const ConditionVector& cond={}) override; private: std::vector<std::unique_ptr<Mutation>> muts_; std::vector<double> weights_;};
std::unique_ptr<CompositeMutation> create_default_mutations(); }
