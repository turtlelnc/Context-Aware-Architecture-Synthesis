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

The CLI writes `best_genotype.json` and prints the best proxy fitness.
