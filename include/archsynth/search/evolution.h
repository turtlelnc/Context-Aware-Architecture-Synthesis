#pragma once
#include "archsynth/evaluation/evaluator.h"
#include "archsynth/llm_assist/llm_mutator.h"
#include "archsynth/search/mutations.h"
#include "archsynth/search/population.h"
namespace archsynth { struct EvolutionConfig{ int population_size=100; int max_generations=100; int tournament_size=5; double elite_ratio=0.1; int stagnation_patience=10; bool use_llm_assist=false;}; struct EvolutionState{ std::vector<Genotype> population; std::vector<double> fitness; Genotype best_genotype; double best_fitness=-1e300; int generations_no_improvement=0;}; class Evolution{public: Evolution(EvolutionConfig config,std::unique_ptr<Evaluator> evaluator,std::unique_ptr<Mutation> mutation,std::unique_ptr<LLMMutator> llm_mutator=nullptr); std::pair<Genotype,double> run(const ConditionVector& cond,std::mt19937& rng); private: EvolutionConfig config_; std::unique_ptr<Evaluator> evaluator_; std::unique_ptr<Mutation> mutation_; std::unique_ptr<LLMMutator> llm_mutator_;}; }
