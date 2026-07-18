#include "archsynth/core/graph.h"
#include <algorithm>
#include <queue>
#include <sstream>
#include <stdexcept>
namespace archsynth {
ComputeGraph::ComputeGraph(){ nodes_[input_node_id]={input_node_id,"input",{}}; nodes_[output_node_id]={output_node_id,"output",{}}; }
void ComputeGraph::add_node(const Node& n){ if(n.id.empty()) throw std::invalid_argument("node id cannot be empty"); nodes_[n.id]=n; }
void ComputeGraph::remove_node(const std::string& id){ if(id==input_node_id||id==output_node_id) return; nodes_.erase(id); edges_.erase(std::remove_if(edges_.begin(),edges_.end(),[&](const Edge&e){return e.src_id==id||e.dst_id==id;}),edges_.end()); }
void ComputeGraph::add_edge(const Edge& e){ edges_.push_back(e); }
void ComputeGraph::remove_edge(const std::string& s,const std::string& d){ edges_.erase(std::remove_if(edges_.begin(),edges_.end(),[&](const Edge&e){return e.src_id==s&&e.dst_id==d;}),edges_.end()); }
const Node* ComputeGraph::get_node(const std::string& id) const{ auto it=nodes_.find(id); return it==nodes_.end()?nullptr:&it->second; }
std::vector<std::string> ComputeGraph::get_sources(const std::string& d) const{ std::vector<std::string> v; for(auto&e:edges_) if(e.dst_id==d) v.push_back(e.src_id); return v; }
std::vector<std::string> ComputeGraph::get_destinations(const std::string& s) const{ std::vector<std::string> v; for(auto&e:edges_) if(e.src_id==s) v.push_back(e.dst_id); return v; }
std::vector<std::string> ComputeGraph::topological_order() const{ std::unordered_map<std::string,int> indeg; std::unordered_map<std::string,std::vector<std::string>> adj; for(auto&kv:nodes_) indeg[kv.first]=0; for(auto&e:edges_){ if(!nodes_.count(e.src_id)||!nodes_.count(e.dst_id)) throw std::runtime_error("edge references missing node"); adj[e.src_id].push_back(e.dst_id); indeg[e.dst_id]++; } std::queue<std::string> q; for(auto&kv:indeg) if(kv.second==0) q.push(kv.first); std::vector<std::string> out; while(!q.empty()){ auto u=q.front(); q.pop(); out.push_back(u); for(auto&v:adj[u]) if(--indeg[v]==0) q.push(v); } if(out.size()!=nodes_.size()) throw std::runtime_error("cycle detected"); return out; }
bool ComputeGraph::is_valid(std::string& err) const{ if(!nodes_.count(input_node_id)||!nodes_.count(output_node_id)){err="missing input/output node"; return false;} for(auto&e:edges_) if(!nodes_.count(e.src_id)||!nodes_.count(e.dst_id)){err="edge references missing node"; return false;} try{ auto order=topological_order(); (void)order; }catch(const std::exception& ex){err=ex.what(); return false;} err.clear(); return true; }
std::string ComputeGraph::to_json() const{ std::ostringstream os; os<<"{\"nodes\":"<<nodes_.size()<<",\"edges\":"<<edges_.size()<<"}"; return os.str(); }
ComputeGraph ComputeGraph::from_json(const std::string&){ return ComputeGraph(); }
}
