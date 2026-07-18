#include "archsynth/core/primitives.h"
#include <cassert>
#include <algorithm>
int main(){ archsynth::register_primitives(); auto n=archsynth::list_primitives(); for(auto req: {"linear","linear_lowrank","lookup_table","attention_std","attention_linear","swiglu","gate","residual_add","rms_norm"}) assert(std::find(n.begin(),n.end(),req)!=n.end()); assert(archsynth::get_primitive("linear").param_count>0); }
