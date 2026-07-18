#include "archsynth/conditioning/scenario_encoder.h"
#include <functional>
namespace archsynth { ScenarioEncoder::ScenarioEncoder(std::string p):onnx_model_path_(std::move(p)){} std::vector<float> ScenarioEncoder::encode(const Scenario& s) const{ std::vector<float> v(128,0.0f); auto put=[&](const std::string& x,float val){ v[std::hash<std::string>{}(x)%v.size()]+=val; }; put(s.task_type,1); for(size_t i=0;i<s.text.size();++i) put(s.text.substr(i,1),0.01f); int k=0; for(auto&kv:s.constraints) if(k<16) v[k++]=static_cast<float>(kv.second/1000.0); return v; } }
