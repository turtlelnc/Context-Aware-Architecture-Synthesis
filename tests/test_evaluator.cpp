#include "archsynth/evaluation/evaluator.h"
#include <cassert>
int main(){ archsynth::Scenario s{"code_generation","mobile",{{"max_memory_mb",2048},{"max_latency_ms",50}},"proxy"}; auto e=archsynth::Evaluator(std::make_unique<archsynth::ProxyTrainer>(),s); auto fit=e.evaluate(archsynth::make_minimal_genotype()); assert(fit>-100); }
