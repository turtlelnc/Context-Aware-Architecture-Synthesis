#include "archsynth/evaluation/proxy_trainer.h"

#include <algorithm>
#include <set>

namespace archsynth {
ProxyTrainer::ProxyTrainer(std::string dataset_path, int batch_size, int epochs)
    : dataset_path_(std::move(dataset_path)), batch_size_(batch_size), epochs_(epochs) {}

double ProxyTrainer::train_and_evaluate(const Genotype& genotype) const {
  std::string error;
  if (!genotype.to_graph().is_valid(error)) return 0.0;
  std::set<std::string> primitives;
  bool has_attention = false;
  bool has_nonlinearity = false;
  bool has_norm = false;
  bool has_efficient_linear = false;
  bool has_residual = false;
  for (const auto& node : genotype.nodes) {
    primitives.insert(node.primitive_name);
    has_attention |= node.primitive_name == "attention_std" || node.primitive_name == "attention_linear";
    has_nonlinearity |= node.primitive_name == "swiglu" || node.primitive_name == "gate";
    has_norm |= node.primitive_name == "rms_norm";
    has_efficient_linear |= node.primitive_name == "linear_lowrank";
    has_residual |= node.primitive_name == "residual_add";
  }
  double quality = 0.30;
  quality += std::min(0.15, 0.025 * static_cast<double>(primitives.size()));
  if (has_attention) quality += 0.18;
  if (has_nonlinearity) quality += 0.12;
  if (has_norm) quality += 0.08;
  if (has_efficient_linear) quality += 0.08;
  if (has_residual) quality += 0.05;
  return std::clamp(quality, 0.0, 1.0);
}
}  // namespace archsynth
