#include "archsynth/core/graph.h"
#include "archsynth/core/genotype.h"

#include <algorithm>
#include <queue>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>

namespace archsynth {
ComputeGraph::ComputeGraph() {
  nodes_[input_node_id] = {input_node_id, "input", {}};
  nodes_[output_node_id] = {output_node_id, "output", {}};
}
void ComputeGraph::add_node(const Node& node) {
  if (node.id.empty()) throw std::invalid_argument("node id cannot be empty");
  if (nodes_.count(node.id)) throw std::invalid_argument("duplicate node id: " + node.id);
  nodes_[node.id] = node;
}
void ComputeGraph::remove_node(const std::string& id) {
  if (id == input_node_id || id == output_node_id) return;
  nodes_.erase(id);
  edges_.erase(std::remove_if(edges_.begin(), edges_.end(), [&](const Edge& edge) {
    return edge.src_id == id || edge.dst_id == id;
  }), edges_.end());
}
void ComputeGraph::add_edge(const Edge& edge) { edges_.push_back(edge); }
void ComputeGraph::remove_edge(const std::string& source, const std::string& destination) {
  edges_.erase(std::remove_if(edges_.begin(), edges_.end(), [&](const Edge& edge) {
    return edge.src_id == source && edge.dst_id == destination;
  }), edges_.end());
}
const Node* ComputeGraph::get_node(const std::string& id) const {
  const auto it = nodes_.find(id);
  return it == nodes_.end() ? nullptr : &it->second;
}
std::vector<std::string> ComputeGraph::get_sources(const std::string& destination) const {
  std::vector<std::string> result;
  for (const auto& edge : edges_) if (edge.dst_id == destination) result.push_back(edge.src_id);
  return result;
}
std::vector<std::string> ComputeGraph::get_destinations(const std::string& source) const {
  std::vector<std::string> result;
  for (const auto& edge : edges_) if (edge.src_id == source) result.push_back(edge.dst_id);
  return result;
}
std::vector<std::string> ComputeGraph::topological_order() const {
  std::unordered_map<std::string, int> indegree;
  std::unordered_map<std::string, std::vector<std::string>> adjacency;
  for (const auto& item : nodes_) indegree[item.first] = 0;
  for (const auto& edge : edges_) {
    if (!nodes_.count(edge.src_id) || !nodes_.count(edge.dst_id))
      throw std::runtime_error("edge references missing node");
    adjacency[edge.src_id].push_back(edge.dst_id);
    ++indegree[edge.dst_id];
  }
  std::queue<std::string> ready;
  for (const auto& item : indegree) if (item.second == 0) ready.push(item.first);
  std::vector<std::string> result;
  while (!ready.empty()) {
    const auto current = ready.front(); ready.pop(); result.push_back(current);
    for (const auto& next : adjacency[current]) if (--indegree[next] == 0) ready.push(next);
  }
  if (result.size() != nodes_.size()) throw std::runtime_error("cycle detected");
  return result;
}

bool ComputeGraph::is_valid(std::string& error) const {
  try {
    if (!nodes_.count(input_node_id) || !nodes_.count(output_node_id))
      throw std::runtime_error("missing input/output node");
    std::set<std::pair<std::string, std::string>> unique_edges;
    for (const auto& edge : edges_) {
      if (!nodes_.count(edge.src_id) || !nodes_.count(edge.dst_id))
        throw std::runtime_error("edge references missing node");
      if (edge.src_id == output_node_id) throw std::runtime_error("output node cannot have outgoing edges");
      if (edge.dst_id == input_node_id) throw std::runtime_error("input node cannot have incoming edges");
      if (!unique_edges.emplace(edge.src_id, edge.dst_id).second)
        throw std::runtime_error("duplicate edge: " + edge.src_id + " -> " + edge.dst_id);
    }
    const auto order = topological_order();
    std::unordered_set<std::string> reachable{input_node_id};
    std::unordered_map<std::string, int64_t> dimensions{{input_node_id, -1}};
    std::unordered_map<std::string, TensorKind> kinds{{input_node_id, TensorKind::TokenIds}};
    for (const auto& id : order) {
      if (id == input_node_id) continue;
      const auto sources = get_sources(id);
      if (id == output_node_id) {
        if (sources.size() != 1) throw std::runtime_error("output node requires exactly one input");
        if (!reachable.count(sources.front())) throw std::runtime_error("output is not reachable from input");
        if (kinds.at(sources.front()) != TensorKind::FloatFeatures)
          throw std::runtime_error("output node requires floating-point features");
        dimensions[id] = dimensions.at(sources.front());
        kinds[id] = TensorKind::FloatFeatures;
        reachable.insert(id);
        continue;
      }
      const auto& node = nodes_.at(id);
      const int minimum = primitive_min_inputs(node.primitive_name);
      const int maximum = primitive_max_inputs(node.primitive_name);
      if (static_cast<int>(sources.size()) < minimum || static_cast<int>(sources.size()) > maximum)
        throw std::runtime_error("node '" + id + "' (" + node.primitive_name + ") requires " +
                                 std::to_string(minimum) + " input(s), got " + std::to_string(sources.size()));
      const auto hyperparams = node.hyperparams_vals.empty()
                                   ? default_hyperparams_for(node.primitive_name)
                                   : node.hyperparams_vals;
      validate_primitive_hyperparams(node.primitive_name, hyperparams);
      const int64_t expected = primitive_input_dim(node.primitive_name, hyperparams);
      const TensorKind expected_kind = primitive_input_kind(node.primitive_name);
      for (const auto& source : sources) {
        if (!reachable.count(source)) throw std::runtime_error("node '" + id + "' is not reachable from input");
        if (kinds.at(source) != expected_kind) {
          const std::string expected_name = expected_kind == TensorKind::TokenIds ? "token IDs" : "floating-point features";
          throw std::runtime_error("tensor type mismatch at node '" + id + "': expected " + expected_name);
        }
        const int64_t actual = dimensions.at(source);
        if (expected > 0 && actual != expected)
          throw std::runtime_error("dimension mismatch at node '" + id + "': expected " +
                                   std::to_string(expected) + ", got " + std::to_string(actual));
      }
      dimensions[id] = primitive_output_dim(node.primitive_name, hyperparams);
      kinds[id] = primitive_output_kind(node.primitive_name);
      reachable.insert(id);
    }
    if (!reachable.count(output_node_id)) throw std::runtime_error("no path from input to output");
    if (reachable.size() != nodes_.size()) throw std::runtime_error("graph contains disconnected nodes");
    error.clear();
    return true;
  } catch (const std::exception& exception) {
    error = exception.what();
    return false;
  }
}

std::string ComputeGraph::to_json() const { return Genotype::from_graph(*this).to_json(); }
ComputeGraph ComputeGraph::from_json(const std::string& text) { return Genotype::from_json(text).to_graph(); }
}  // namespace archsynth
