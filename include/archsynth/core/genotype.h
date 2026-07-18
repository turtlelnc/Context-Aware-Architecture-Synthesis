#pragma once
#include "archsynth/core/graph.h"
#include <utility>
namespace archsynth {
struct GenotypeNode { std::string id; std::string primitive_name; Hyperparams hyperparams; };
struct Genotype { std::vector<GenotypeNode> nodes; std::vector<std::pair<std::string,std::string>> edges; ComputeGraph to_graph() const; static Genotype from_graph(const ComputeGraph& graph); std::string to_json() const; static Genotype from_json(const std::string& json); };
Genotype make_minimal_genotype();
}
