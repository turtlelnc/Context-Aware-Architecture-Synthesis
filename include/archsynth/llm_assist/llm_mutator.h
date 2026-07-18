#pragma once

#include "archsynth/conditioning/scenario.h"
#include "archsynth/core/genotype.h"

#include <functional>
#include <random>
#include <string>

namespace archsynth {
class LLMMutator {
 public:
  using ResponseProvider = std::function<std::string(const std::string& prompt)>;

  explicit LLMMutator(ResponseProvider provider = {});
  std::vector<Genotype> propose_mutations(const Genotype& best,
                                           const Scenario& scenario,
                                           double fitness,
                                           std::mt19937& rng) const;
  bool configured() const { return static_cast<bool>(provider_); }

 private:
  std::string build_prompt(const Genotype&, const Scenario&, double) const;
  ResponseProvider provider_;
};
}  // namespace archsynth
