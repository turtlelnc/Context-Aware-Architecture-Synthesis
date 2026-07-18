#include "archsynth/conditioning/scenario_encoder.h"
#include "archsynth/core/json.h"
#include "archsynth/evaluation/evaluator.h"
#include "archsynth/export/builder.h"
#include "archsynth/search/evolution.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace {
archsynth::Scenario read_scenario(const std::string& path) {
  std::ifstream input(path);
  if (!input) throw std::runtime_error("cannot open scenario file: " + path);
  const std::string text((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
  const auto root = archsynth::json::parse(text);
  archsynth::Scenario scenario;
  scenario.task_type = archsynth::json::string(root.at("task_type"), "task_type");
  scenario.text = archsynth::json::string(root.at("text"), "text");
  scenario.target_metric = archsynth::json::string(root.at("target_metric"), "target_metric");
  const auto& constraints = root.at("constraints");
  if (constraints.type != archsynth::json::Value::Type::Object)
    throw std::invalid_argument("constraints must be an object");
  for (const auto& item : constraints.object)
    scenario.constraints.emplace(item.first, archsynth::json::number(item.second, "constraint"));
  return scenario;
}

int positive_int(const char* value, const std::string& option) {
  const int parsed = std::stoi(value);
  if (parsed <= 0) throw std::invalid_argument(option + " must be positive");
  return parsed;
}

void usage(const char* program) {
  std::cout << "Usage: " << program << " [--input scenario.json] [--output genotype.json]"
            << " [--population N] [--generations N] [--seed N]\n";
}
}  // namespace

int main(int argc, char** argv) {
  try {
    std::string input_path = "examples/mobile_code_generation.json";
    std::string output_path = "best_genotype.json";
    archsynth::EvolutionConfig config;
    config.population_size = 12;
    config.max_generations = 5;
    unsigned seed = 42;

    for (int i = 1; i < argc; ++i) {
      const std::string option = argv[i];
      if (option == "--help" || option == "-h") { usage(argv[0]); return 0; }
      if (i + 1 >= argc) throw std::invalid_argument("missing value for " + option);
      const char* value = argv[++i];
      if (option == "--input") input_path = value;
      else if (option == "--output") output_path = value;
      else if (option == "--population") config.population_size = positive_int(value, option);
      else if (option == "--generations") config.max_generations = positive_int(value, option);
      else if (option == "--seed") seed = static_cast<unsigned>(positive_int(value, option));
      else throw std::invalid_argument("unknown option: " + option);
    }

    archsynth::register_primitives();
    const auto scenario = read_scenario(input_path);
    archsynth::ScenarioEncoder encoder;
    auto evaluator = std::make_unique<archsynth::Evaluator>(
        std::make_unique<archsynth::ProxyTrainer>(), scenario);
    std::mt19937 rng(seed);
    archsynth::Evolution evolution(
        config, std::move(evaluator), archsynth::create_default_mutations());
    auto [best, fitness] = evolution.run(encoder.encode(scenario), rng);

    std::ofstream output(output_path);
    if (!output) throw std::runtime_error("cannot write output file: " + output_path);
    output << best.to_json() << '\n';
    std::cout << "Best fitness: " << fitness << "\n"
              << archsynth::ModelBuilder::build(best)->summary() << "\n"
              << "Saved genotype: " << output_path << "\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "archsynth: " << error.what() << '\n';
    return 1;
  }
}
