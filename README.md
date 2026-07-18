# ArchSynth

ArchSynth is a C++ MVP for context-aware neural architecture synthesis. It accepts a scenario, encodes constraints into a condition vector, evolves genotype-encoded DAG architectures, estimates proxy fitness with hardware penalties, and exports the best genotype.

## Build

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Run

```bash
./build/archsynth_cli
```

With no arguments, the CLI reads `examples/mobile_code_generation.json`, writes
`best_genotype.json`, and prints the best proxy fitness. To use another scenario:

```bash
./build/archsynth_cli \
  --input my_scenario.json \
  --output result.json \
  --population 100 \
  --generations 50 \
  --seed 42
```

The scenario file must contain:

```json
{
  "task_type": "code_generation",
  "text": "mobile code model under 50ms",
  "constraints": {
    "max_memory_mb": 2048,
    "max_latency_ms": 50
  },
  "target_metric": "proxy"
}
```

Run `./build/archsynth_cli --help` to list command-line options.

## LLM assist integration

`LLMMutator` accepts a response-provider callback instead of silently pretending
to call an API. Applications can use their preferred HTTP client in that callback.
The callback receives a prompt and must return one complete genotype JSON object.
