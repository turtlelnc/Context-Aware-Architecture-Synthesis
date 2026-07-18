#include "archsynth/core/genotype.h"
#include <sstream>
namespace archsynth {
ComputeGraph Genotype::to_graph() const{ ComputeGraph g; for(const auto& n:nodes){ if(n.id==g.input_node_id||n.id==g.output_node_id) continue; auto hp=n.hyperparams.empty()?default_hyperparams_for(n.primitive_name):n.hyperparams; g.add_node({n.id,n.primitive_name,hp}); } for(auto&e:edges) g.add_edge({e.first,e.second,0}); return g; }
Genotype Genotype::from_graph(const ComputeGraph& g){ Genotype gt; for(auto&kv:g.nodes()) if(kv.first!=g.input_node_id&&kv.first!=g.output_node_id) gt.nodes.push_back({kv.second.id,kv.second.primitive_name,kv.second.hyperparams_vals}); for(auto&e:g.edges()) gt.edges.push_back({e.src_id,e.dst_id}); return gt; }
std::string Genotype::to_json() const{ std::ostringstream os; os<<"{\"nodes\":["; for(size_t i=0;i<nodes.size();++i){ if(i) os<<","; os<<"{\"id\":\""<<nodes[i].id<<"\",\"primitive\":\""<<nodes[i].primitive_name<<"\"}"; } os<<"],\"edges\":["; for(size_t i=0;i<edges.size();++i){ if(i) os<<","; os<<"[\""<<edges[i].first<<"\",\""<<edges[i].second<<"\"]"; } os<<"]}"; return os.str(); }
Genotype Genotype::from_json(const std::string&){ return make_minimal_genotype(); }
Genotype make_minimal_genotype(){ return {{{"n0","linear",default_hyperparams_for("linear")}},{{"input","n0"},{"n0","output"}}}; }
}
