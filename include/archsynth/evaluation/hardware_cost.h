#pragma once
#include "archsynth/core/genotype.h"
#include <cstdint>
namespace archsynth { struct HardwareProfile{ double total_flops=0; int64_t total_params=0; int64_t estimated_memory_bytes=0; double estimated_latency_ms=0;}; HardwareProfile compute_hardware_profile(const Genotype& genotype); }
