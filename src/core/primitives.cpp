#include "archsynth/core/primitives.h"
#include <algorithm>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
namespace archsynth { namespace { std::unordered_map<std::string, Primitive>& reg(){ static std::unordered_map<std::string, Primitive> r; return r; } std::once_flag flag; void add(Primitive p){ reg()[p.name]=std::move(p); } }
void register_primitives(){ std::call_once(flag, [](){
 add({"linear",{{"in_dim",256},{"out_dim",256}},131072,65536,262144});
 add({"linear_lowrank",{{"in_dim",256},{"out_dim",256},{"rank",16}},16384,8192,32768});
 add({"lookup_table",{{"vocab_size",32000},{"dim",256}},256,8192000,32768000});
 add({"attention_std",{{"dim",256},{"heads",4},{"seq_len",128}},16777216,262144,1048576});
 add({"attention_linear",{{"dim",256},{"heads",4},{"seq_len",128}},1048576,262144,1048576});
 add({"swiglu",{{"dim",256},{"hidden_dim",1024}},1572864,786432,3145728});
 add({"gate",{{"dim",256}},131072,65536,262144});
 add({"residual_add",{{"dim",256}},256,0,0});
 add({"rms_norm",{{"dim",256}},512,256,1024}); }); }
const Primitive& get_primitive(const std::string& name){ register_primitives(); auto it=reg().find(name); if(it==reg().end()) throw std::out_of_range("unknown primitive: "+name); return it->second; }
std::vector<std::string> list_primitives(){ register_primitives(); std::vector<std::string> names; for(auto& kv:reg()) names.push_back(kv.first); std::sort(names.begin(), names.end()); return names; }
Hyperparams default_hyperparams_for(const std::string& primitive_name){ return get_primitive(primitive_name).hyperparams; }
}
