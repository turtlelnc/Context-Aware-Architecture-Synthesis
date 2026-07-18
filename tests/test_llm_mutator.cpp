#include "archsynth/llm_assist/llm_mutator.h"

#include <cassert>
#include <stdexcept>

int main() {
  const auto original = archsynth::make_minimal_genotype();
  archsynth::LLMMutator mutator([&](const std::string& prompt) {
    assert(prompt.find("Current genotype") != std::string::npos);
    return original.to_json();
  });
  assert(mutator.configured());
  std::mt19937 rng(1);
  const auto proposals = mutator.propose_mutations(original, {"code", "mobile", {}, "proxy"}, 0.5, rng);
  assert(proposals.size() == 1);
  assert(proposals[0].nodes.size() == original.nodes.size());

  bool rejected = false;
  try { archsynth::LLMMutator{}.propose_mutations(original, {}, 0.0, rng); }
  catch (const std::logic_error&) { rejected = true; }
  assert(rejected);
}
