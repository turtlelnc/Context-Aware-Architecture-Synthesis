#include "archsynth/search/population.h"
namespace archsynth { std::vector<Genotype> Population::initialize(int size,std::mt19937& rng,const ConditionVector& cond){ std::vector<Genotype> pop; auto mut=create_default_mutations(); for(int i=0;i<size;++i){ auto g=make_minimal_genotype(); for(int j=0;j<1+(i%4);++j) mut->apply(g,rng,cond); pop.push_back(g);} return pop; } }
