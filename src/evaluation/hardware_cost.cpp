#include "archsynth/evaluation/hardware_cost.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace archsynth {
namespace {
int64_t integer(const Hyperparams& values, const std::string& key) {
  const auto it = values.find(key);
  if (it == values.end() || it->second <= 0.0 || !std::isfinite(it->second) ||
      it->second != static_cast<int64_t>(it->second))
    throw std::invalid_argument("invalid hardware-cost hyperparameter: " + key);
  return static_cast<int64_t>(it->second);
}
int64_t checked_int64(long double value, const std::string& name) {
  if (!std::isfinite(value) || value < 0.0L ||
      value > static_cast<long double>(std::numeric_limits<int64_t>::max()))
    throw std::overflow_error(name + " exceeds int64 range");
  return static_cast<int64_t>(value);
}
}  // namespace

HardwareProfile compute_hardware_profile(const Genotype& genotype) {
  long double total_flops = 0.0L;
  long double total_params = 0.0L;
  for (const auto& node : genotype.nodes) {
    const auto hp = node.hyperparams.empty()
                        ? default_hyperparams_for(node.primitive_name)
                        : node.hyperparams;
    validate_primitive_hyperparams(node.primitive_name, hp);
    const std::string& name = node.primitive_name;
    long double params = 0.0L;
    long double flops = 0.0L;
    if (name == "lookup_table") {
      params = static_cast<long double>(integer(hp, "vocab_size")) * integer(hp, "dim");
      flops = integer(hp, "dim");
    } else if (name == "linear") {
      params = static_cast<long double>(integer(hp, "in_dim")) * integer(hp, "out_dim");
      flops = 2.0L * params;
    } else if (name == "linear_lowrank") {
      params = static_cast<long double>(integer(hp, "rank")) *
               (integer(hp, "in_dim") + integer(hp, "out_dim"));
      flops = 2.0L * params;
    } else if (name == "attention_std") {
      const long double dim = integer(hp, "dim");
      const long double sequence = integer(hp, "seq_len");
      params = 4.0L * dim * dim;
      flops = 8.0L * dim * dim + 4.0L * sequence * dim;
    } else if (name == "attention_linear") {
      const long double dim = integer(hp, "dim");
      params = 4.0L * dim * dim;
      flops = 10.0L * dim * dim;
    } else if (name == "swiglu") {
      const long double dim = integer(hp, "dim");
      const long double hidden = integer(hp, "hidden_dim");
      params = 3.0L * dim * hidden;
      flops = 6.0L * dim * hidden;
    } else if (name == "gate") {
      const long double dim = integer(hp, "dim");
      params = dim * dim;
      flops = 2.0L * params;
    } else if (name == "rms_norm") {
      params = integer(hp, "dim");
      flops = 4.0L * params;
    } else if (name == "residual_add") {
      flops = integer(hp, "dim");
    } else {
      throw std::invalid_argument("unsupported primitive in hardware model: " + name);
    }
    total_params += params;
    total_flops += flops;
  }
  HardwareProfile result;
  result.total_params = checked_int64(total_params, "parameter count");
  result.total_flops = static_cast<double>(total_flops);
  result.estimated_memory_bytes = checked_int64(total_params * 4.0L, "parameter memory");
  constexpr long double effective_mobile_flops = 20.0e9L;
  constexpr long double effective_memory_bandwidth = 8.0e9L;
  result.estimated_latency_ms = static_cast<double>(
      total_flops / effective_mobile_flops * 1000.0L +
      static_cast<long double>(result.estimated_memory_bytes) /
          effective_memory_bandwidth * 1000.0L);
  return result;
}
}  // namespace archsynth
