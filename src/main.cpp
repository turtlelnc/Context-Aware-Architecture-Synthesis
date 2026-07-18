#include "archsynth/conditioning/scenario_encoder.h"
#include "archsynth/evaluation/evaluator.h"
#include "archsynth/export/builder.h"
#include "archsynth/search/evolution.h"
#include <fstream>
#include <iostream>
int main(){ archsynth::register_primitives(); archsynth::Scenario scenario{"code_generation","mobile code model under 50ms",{{"max_memory_mb",2048},{"max_latency_ms",50}},"proxy"}; archsynth::ScenarioEncoder enc; auto cond=enc.encode(scenario); auto trainer=std::make_unique<archsynth::ProxyTrainer>(); auto evaluator=std::make_unique<archsynth::Evaluator>(std::move(trainer),scenario); archsynth::EvolutionConfig cfg; cfg.population_size=12; cfg.max_generations=5; std::mt19937 rng(42); archsynth::Evolution evo(cfg,std::move(evaluator),archsynth::create_default_mutations()); auto [best,fit]=evo.run(cond,rng); std::ofstream("best_genotype.json")<<best.to_json(); auto model=archsynth::ModelBuilder::build(best); std::cout<<"Best fitness: "<<fit<<"\n"<<model->summary()<<"\n"; }
