#include "archsynth/llm_assist/llm_mutator.h"

#include <stdexcept>

namespace archsynth {
LLMMutator::LLMMutator(ResponseProvider provider) : provider_(std::move(provider)) {}

std::string LLMMutator::build_prompt(const Genotype& genotype,
                                     const Scenario& scenario,
                                     double fitness) const {
  return "Return only one complete ArchSynth genotype JSON object. Scenario: " +
         scenario.text + ". Current fitness: " + std::to_string(fitness) +
         ". Current genotype: " + genotype.to_json();
}

std::vector<Genotype> LLMMutator::propose_mutations(const Genotype& best,
                                                     const Scenario& scenario,
                                                     double fitness,
                                                     std::mt19937& rng) const {
  (void)rng;
  if (!provider_)
    throw std::logic_error("LLM assist is enabled but no response provider is configured");
  const std::string response = provider_(build_prompt(best, scenario, fitness));
  return {Genotype::from_json(response)};
}
}  // namespace archsynth
