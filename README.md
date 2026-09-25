# LDPC Girth Switching

Reference implementation and experiment driver for the paper
**Polynomial-Time Degree-Preserving Switching for Tanner Graphs of LDPC Codes**.

The program works with simple bipartite `(dv,dc)`-regular Tanner graphs,
computes the guaranteed switching threshold

`g_sw = 2 floor(log_q(n(q-1)/(2 dc))) + 2`,  where `q=(dv-1)(dc-1)`,

and applies degree-preserving bipartite 2-switches while requiring that the
girth never decrease.

## Build

A C++17 compiler is sufficient:

```bash
g++ -O3 -std=c++17 -DNDEBUG ldpc_girth_switching.cpp -o ldpc_girth_switching
```

## Run

```bash
./ldpc_girth_switching --n 1944 --dv 3 --dc 6 --seed 1
```

To run the experiment set:

```bash
bash run_experiments.sh
```

The program prints the input girth, target `g_sw`, output girth, number of
accepted switches, and wall-clock time.

## Reproducibility note

Runtime depends on processor, compiler, and system load. Graph generation is
seeded. The implementation uses a deterministic scan of candidate switches
after the seeded input graph has been generated.

## Files

- `ldpc_girth_switching.cpp`: graph generator, girth computation, and switching routine.
- `run_experiments.sh`: commands for the parameter sets used in the numerical illustration.
- `reported_results.csv`: values reported in the manuscript table.

## Important

The CSV records the manuscript's reported measurements. Re-running on a
different machine can give different wall-clock times. Before final
submission, the authors should verify that the reported rows are reproduced
by the archived code/version and record the machine/compiler used.

## License

MIT.
