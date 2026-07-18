#pragma once
#include "archsynth/core/types.h"
#include <cstdint>
#include <string>
#include <vector>
namespace archsynth {
struct Primitive { std::string name; Hyperparams hyperparams; double flops_per_token=0.0; int64_t param_count=0; int64_t memory_bytes=0; };
void register_primitives();
const Primitive& get_primitive(const std::string& name);
std::vector<std::string> list_primitives();
Hyperparams default_hyperparams_for(const std::string& primitive_name);
}
