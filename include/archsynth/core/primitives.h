#pragma once
#include "archsynth/core/types.h"
#include <cstdint>
#include <string>
#include <vector>
namespace archsynth {
enum class TensorKind { TokenIds, FloatFeatures };
struct Primitive { std::string name; Hyperparams hyperparams; double flops_per_token=0.0; int64_t param_count=0; int64_t memory_bytes=0; };
void register_primitives();
const Primitive& get_primitive(const std::string& name);
std::vector<std::string> list_primitives();
Hyperparams default_hyperparams_for(const std::string& primitive_name);
int primitive_min_inputs(const std::string& primitive_name);
int primitive_max_inputs(const std::string& primitive_name);
int64_t primitive_input_dim(const std::string& primitive_name,const Hyperparams& hyperparams);
int64_t primitive_output_dim(const std::string& primitive_name,const Hyperparams& hyperparams);
TensorKind primitive_input_kind(const std::string& primitive_name);
TensorKind primitive_output_kind(const std::string& primitive_name);
void validate_primitive_hyperparams(const std::string& primitive_name,const Hyperparams& hyperparams);
}
