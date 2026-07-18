#include "archsynth/search/mutations.h"
#include <cassert>
int main(){ std::mt19937 rng(7); auto g=archsynth::make_minimal_genotype(); auto m=archsynth::create_default_mutations(); for(int i=0;i<50;++i){ auto old=g; if(!m->apply(g,rng,{})) g=old; std::string err; assert(g.to_graph().is_valid(err)); } }
