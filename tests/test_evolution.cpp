#include "archsynth/search/evolution.h"
#include <cassert>
int main(){ archsynth::Scenario s{"code_generation","mobile",{{"max_memory_mb",2048},{"max_latency_ms",50}},"proxy"}; archsynth::EvolutionConfig c; c.population_size=8; c.max_generations=3; std::mt19937 rng(1); archsynth::Evolution evo(c,std::make_unique<archsynth::Evaluator>(std::make_unique<archsynth::ProxyTrainer>(),s),archsynth::create_default_mutations()); auto r=evo.run({},rng); assert(r.second>-1000); }
