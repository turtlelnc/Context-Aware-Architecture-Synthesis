#include "archsynth/evaluation/proxy_trainer.h"
#include <limits>
namespace archsynth { ProxyTrainer::ProxyTrainer(std::string p,int b,int e):dataset_path_(std::move(p)),batch_size_(b),epochs_(e){} double ProxyTrainer::train_and_evaluate(const Genotype& g) const{ std::string err; if(!g.to_graph().is_valid(err)) return -std::numeric_limits<double>::infinity(); return 0.5 + 0.03*static_cast<double>(g.nodes.size()) - 0.005*static_cast<double>(g.edges.size()); } }
