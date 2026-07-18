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
  mismatch.nodes[0].primitive_name = "linear";
  mismatch.nodes[0].hyperparams = {{"in_dim", 128}, {"out_dim", 256}};
  assert(!mismatch.to_graph().is_valid(error));
  assert(error.find("tensor type mismatch") != std::string::npos);

  auto chained_lookup = archsynth::make_minimal_genotype();
  chained_lookup.nodes.push_back({"embedding2", "lookup_table", {}});
  chained_lookup.edges = {{"input", "n0"}, {"n0", "embedding2"}, {"embedding2", "output"}};
  assert(!chained_lookup.to_graph().is_valid(error));
  assert(error.find("tensor type mismatch") != std::string::npos);

  archsynth::Genotype valid_residual{
      {{"embedding", "lookup_table", {}},
       {"left", "linear", {{"in_dim", 256}, {"out_dim", 256}}},
       {"right", "rms_norm", {{"dim", 256}}},
       {"add", "residual_add", {{"dim", 256}}}},
      {{"input", "embedding"}, {"embedding", "left"}, {"embedding", "right"}, {"left", "add"},
       {"right", "add"}, {"add", "output"}}};
  assert(valid_residual.to_graph().is_valid(error));

  auto invalid_attention = archsynth::make_minimal_genotype();
  invalid_attention.nodes.push_back({"attention", "attention_std", {{"dim", 256}, {"heads", 3}, {"seq_len", 128}}});
  invalid_attention.edges = {{"input", "n0"}, {"n0", "attention"}, {"attention", "output"}};
  assert(!invalid_attention.to_graph().is_valid(error));
  assert(error.find("divisible by heads") != std::string::npos);
}
