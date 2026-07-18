#include "archsynth/core/genotype.h"

#include <cassert>

int main() {
  auto graph = archsynth::make_minimal_genotype().to_graph();
  std::string error;
  assert(graph.is_valid(error));
  const auto restored = archsynth::ComputeGraph::from_json(graph.to_json());
  assert(restored.is_valid(error));
  assert(restored.nodes().size() == graph.nodes().size());
  assert(restored.edges().size() == graph.edges().size());
  graph.add_edge({"output", "input", 0});
  assert(!graph.is_valid(error));
}
