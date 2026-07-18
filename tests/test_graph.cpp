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

  auto residual = archsynth::make_minimal_genotype();
  residual.nodes[0].primitive_name = "residual_add";
  residual.nodes[0].hyperparams = {{"dim", 256}};
  assert(!residual.to_graph().is_valid(error));
  assert(error.find("requires 2 input") != std::string::npos);

  auto mismatch = archsynth::make_minimal_genotype();
  mismatch.nodes[0].hyperparams = {{"in_dim", 128}, {"out_dim", 256}};
  assert(!mismatch.to_graph().is_valid(error));
  assert(error.find("dimension mismatch") != std::string::npos);

  archsynth::Genotype valid_residual{
      {{"left", "linear", {{"in_dim", 256}, {"out_dim", 256}}},
       {"right", "rms_norm", {{"dim", 256}}},
       {"add", "residual_add", {{"dim", 256}}}},
      {{"input", "left"}, {"input", "right"}, {"left", "add"},
       {"right", "add"}, {"add", "output"}}};
  assert(valid_residual.to_graph().is_valid(error));
}
