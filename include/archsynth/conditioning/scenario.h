#pragma once
#include <string>
#include <unordered_map>
namespace archsynth { struct Scenario{ std::string task_type; std::string text; std::unordered_map<std::string,double> constraints; std::string target_metric; }; }
