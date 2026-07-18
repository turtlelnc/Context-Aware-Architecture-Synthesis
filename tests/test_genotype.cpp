#include "archsynth/core/genotype.h"
#include <cassert>
int main(){ auto gt=archsynth::make_minimal_genotype(); auto round=archsynth::Genotype::from_graph(gt.to_graph()); assert(round.nodes.size()==1); assert(!round.to_json().empty()); }
