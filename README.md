# Cardinality Estimator for Graph Path Queries

Group 3's implementation of the **Quicksilver** graph query engine for the *Engineering Data Systems* course at TU Eindhoven — a query engine for path queries over labelled, directed graphs, focused on **cardinality estimation** and **cache-aware evaluation**.

## What's inside

- **Cardinality estimator** (`SimpleEstimator`) — estimates the result size of path queries using **equi-depth histograms** built over sampled graphs, plus per-label cardinality estimation
- **LRU-cached query operators** — `LruCacheJoin`, `LruCacheSelect` and `LruCacheTrans` cache intermediate results across queries
- **Query language** — path queries over triples with alternation (`|`), inverse labels (`<`), and Kleene closure (`+`)
- **Benchmarking** — execution time and **resident memory (RSS)** tracking over bundled workloads

## Build & run

CMake + a C++17 compiler:

```bash
cmake -B build && cmake --build build
```

Run a single graph + query set:

```bash
./quicksilver <graphFile> <queriesFile>
```

Or benchmark all workloads in a directory (execution time + peak RSS per query):

```bash
./benchmarker <workloaddir>
```

## Workloads

- `workload/real/1` — real graph in N-Triples format (`.nt`) + `queries.csv`
- `workload/syn/1` — synthetic graph + queries

Graphs are stored as N-Triples; queries are comma-separated source / path / target patterns.
