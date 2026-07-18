#include "archsynth/core/genotype.h"
#include <cassert>
int main(){ auto g=archsynth::make_minimal_genotype().to_graph(); std::string err; assert(g.is_valid(err)); g.add_edge({"output","input",0}); assert(!g.is_valid(err)); }
