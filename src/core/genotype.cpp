#include "archsynth/core/genotype.h"
#include "archsynth/core/json.h"

#include <sstream>
#include <stdexcept>

namespace archsynth {
ComputeGraph Genotype::to_graph() const {
  ComputeGraph graph;
  for (const auto& node : nodes) {
    if (node.id == graph.input_node_id || node.id == graph.output_node_id) continue;
    const auto hyperparams = node.hyperparams.empty()
                                 ? default_hyperparams_for(node.primitive_name)
                                 : node.hyperparams;
    graph.add_node({node.id, node.primitive_name, hyperparams});
  }
  for (const auto& edge : edges) graph.add_edge({edge.first, edge.second, 0});
  return graph;
}

Genotype Genotype::from_graph(const ComputeGraph& graph) {
  Genotype result;
  for (const auto& item : graph.nodes()) {
    if (item.first != graph.input_node_id && item.first != graph.output_node_id)
      result.nodes.push_back({item.second.id, item.second.primitive_name,
                              item.second.hyperparams_vals});
  }
  for (const auto& edge : graph.edges()) result.edges.emplace_back(edge.src_id, edge.dst_id);
  return result;
}

std::string Genotype::to_json() const {
  std::ostringstream output;
  output << "{\"nodes\":[";
  for (std::size_t i = 0; i < nodes.size(); ++i) {
    if (i) output << ',';
    output << "{\"id\":\"" << json::escape(nodes[i].id)
           << "\",\"primitive\":\"" << json::escape(nodes[i].primitive_name)
           << "\",\"hyperparams\":{";
    bool first = true;
    for (const auto& hyperparam : nodes[i].hyperparams) {
      if (!first) output << ',';
      first = false;
      output << '"' << json::escape(hyperparam.first) << "\":" << hyperparam.second;
    }
    output << "}}";
  }
  output << "],\"edges\":[";
  for (std::size_t i = 0; i < edges.size(); ++i) {
    if (i) output << ',';
    output << "[\"" << json::escape(edges[i].first) << "\",\""
           << json::escape(edges[i].second) << "\"]";
  }
  output << "]}";
  return output.str();
}

Genotype Genotype::from_json(const std::string& text) {
  const auto root = json::parse(text);
  const auto& nodes_value = root.at("nodes");
  const auto& edges_value = root.at("edges");
  if (nodes_value.type != json::Value::Type::Array ||
      edges_value.type != json::Value::Type::Array)
    throw std::invalid_argument("nodes and edges must be arrays");
  if (nodes_value.array.empty())
    throw std::invalid_argument("genotype must contain at least one node");

  Genotype result;
  for (const auto& item : nodes_value.array) {
    GenotypeNode node;
    node.id = json::string(item.at("id"), "node.id");
    node.primitive_name = json::string(item.at("primitive"), "node.primitive");
    const auto& hyperparams = item.at("hyperparams");
    if (hyperparams.type != json::Value::Type::Object)
      throw std::invalid_argument("node.hyperparams must be an object");
    for (const auto& value : hyperparams.object)
      node.hyperparams.emplace(value.first, json::number(value.second, "hyperparameter"));
    result.nodes.push_back(std::move(node));
  }
  for (const auto& item : edges_value.array) {
    if (item.type != json::Value::Type::Array || item.array.size() != 2)
      throw std::invalid_argument("each edge must contain source and destination");
    result.edges.emplace_back(json::string(item.array[0], "edge source"),
                              json::string(item.array[1], "edge destination"));
  }
  std::string error;
  if (!result.to_graph().is_valid(error))
    throw std::invalid_argument("invalid genotype: " + error);
  return result;
}

Genotype make_minimal_genotype() {
  return {{{"n0", "linear", default_hyperparams_for("linear")}},
          {{"input", "n0"}, {"n0", "output"}}};
}
}  // namespace archsynth
