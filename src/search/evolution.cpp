#include "archsynth/search/evolution.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace archsynth {
Evolution::Evolution(EvolutionConfig config, std::unique_ptr<Evaluator> evaluator,
                     std::unique_ptr<Mutation> mutation,
                     std::unique_ptr<LLMMutator> llm_mutator)
    : config_(config), evaluator_(std::move(evaluator)), mutation_(std::move(mutation)),
      llm_mutator_(std::move(llm_mutator)) {
  if (config_.population_size <= 0) throw std::invalid_argument("population_size must be positive");
  if (config_.max_generations <= 0) throw std::invalid_argument("max_generations must be positive");
  if (!std::isfinite(config_.elite_ratio) || config_.elite_ratio < 0.0 || config_.elite_ratio > 1.0)
    throw std::invalid_argument("elite_ratio must be between 0 and 1");
  if (!evaluator_ || !mutation_) throw std::invalid_argument("evaluator and mutation are required");
  if (config_.use_llm_assist && (!llm_mutator_ || !llm_mutator_->configured()))
    throw std::invalid_argument("LLM assist requires a configured LLM mutator");
}

std::pair<Genotype, double> Evolution::run(const ConditionVector& condition, std::mt19937& rng) {
  EvolutionState state;
  state.population = Population::initialize(config_.population_size, rng, condition);
  if (state.population.empty()) throw std::runtime_error("population initialization returned no candidates");
  for (int generation = 0; generation < config_.max_generations; ++generation) {
    state.fitness = evaluator_->evaluate_population(state.population);
    std::vector<std::size_t> indices(state.population.size());
    for (std::size_t i = 0; i < indices.size(); ++i) indices[i] = i;
    std::sort(indices.begin(), indices.end(), [&](std::size_t a, std::size_t b) {
      return state.fitness[a] > state.fitness[b];
    });
    if (state.fitness[indices.front()] > state.best_fitness) {
      state.best_fitness = state.fitness[indices.front()];
      state.best_genotype = state.population[indices.front()];
      state.generations_no_improvement = 0;
    } else {
      ++state.generations_no_improvement;
    }
    std::vector<Genotype> next;
    const int elite_count = std::max(1, static_cast<int>(config_.elite_ratio * config_.population_size));
    for (int i = 0; i < elite_count && i < static_cast<int>(indices.size()); ++i)
      next.push_back(state.population[indices[static_cast<std::size_t>(i)]]);
    std::uniform_int_distribution<int> parent(0, static_cast<int>(state.population.size()) - 1);
    while (static_cast<int>(next.size()) < config_.population_size) {
      auto child = state.population[static_cast<std::size_t>(parent(rng))];
      (void)mutation_->apply(child, rng, condition);
      next.push_back(std::move(child));
    }
    if (config_.use_llm_assist && state.generations_no_improvement > config_.stagnation_patience) {
      auto extras = llm_mutator_->propose_mutations(state.best_genotype, {}, state.best_fitness, rng);
      if (!extras.empty() && !next.empty()) next.back() = std::move(extras.front());
    }
    state.population = std::move(next);
  }
  return {state.best_genotype, state.best_fitness};
}
}  // namespace archsynth
