#include "archsynth/core/genotype.h"

#include <cassert>
#include <cmath>
#include <stdexcept>

int main() {
  auto genotype = archsynth::make_minimal_genotype();
  genotype.nodes[0].id = "node\"one";
  genotype.nodes[0].hyperparams["custom"] = 12.5;
  genotype.edges = {{"input", "node\"one"}, {"node\"one", "output"}};
  const auto restored = archsynth::Genotype::from_json(genotype.to_json());
  assert(restored.nodes.size() == 1);
  assert(restored.nodes[0].id == "node\"one");
  assert(std::abs(restored.nodes[0].hyperparams.at("custom") - 12.5) < 1e-9);
  assert(restored.edges == genotype.edges);

  bool rejected = false;
  try { (void)archsynth::Genotype::from_json("{\"nodes\":[],\"edges\":[]}"); }
  catch (const std::invalid_argument&) { rejected = true; }
  assert(rejected);
}
