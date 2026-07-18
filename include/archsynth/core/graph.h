#pragma once
#include "archsynth/core/primitives.h"
#include <string>
#include <unordered_map>
#include <vector>
namespace archsynth {
struct Node { std::string id; std::string primitive_name; Hyperparams hyperparams_vals; };
struct Edge { std::string src_id; std::string dst_id; int src_port=0; };
class ComputeGraph {
public:
 ComputeGraph(); void add_node(const Node& node); void remove_node(const std::string& id); void add_edge(const Edge& edge); void remove_edge(const std::string& src_id,const std::string& dst_id);
 const Node* get_node(const std::string& id) const; std::vector<std::string> get_sources(const std::string& dst_id) const; std::vector<std::string> get_destinations(const std::string& src_id) const;
 std::vector<std::string> topological_order() const; std::string to_json() const; static ComputeGraph from_json(const std::string& json); bool is_valid(std::string& error_msg) const;
 const std::unordered_map<std::string, Node>& nodes() const { return nodes_; } const std::vector<Edge>& edges() const { return edges_; }
 std::string input_node_id="input"; std::string output_node_id="output";
private: std::unordered_map<std::string, Node> nodes_; std::vector<Edge> edges_; };
}
