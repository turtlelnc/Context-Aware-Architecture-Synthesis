#pragma once
#include "archsynth/conditioning/scenario.h"
#include <string>
#include <vector>
namespace archsynth { class ScenarioEncoder{public: explicit ScenarioEncoder(std::string onnx_model_path=""); std::vector<float> encode(const Scenario& scenario) const; private: std::string onnx_model_path_;}; }
