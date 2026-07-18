#include "archsynth/export/builder.h"
#include "archsynth/evaluation/hardware_cost.h"
#include <stdexcept>
namespace archsynth { std::string BuiltModel::summary() const{ auto hw=compute_hardware_profile(genotype); return "BuiltModel(nodes="+std::to_string(genotype.nodes.size())+", params="+std::to_string(hw.total_params)+")"; } std::shared_ptr<BuiltModel> ModelBuilder::build(const Genotype& g){ std::string e; if(!g.to_graph().is_valid(e)) throw std::invalid_argument("invalid genotype: "+e); return std::make_shared<BuiltModel>(BuiltModel{g}); } }
