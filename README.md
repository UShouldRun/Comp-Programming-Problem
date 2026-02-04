# Warehouse Delivery Optimization

## Table of Contents
- [Problem Description](#problem-description)
- [Input Format](#input-format)
- [Output Format](#output-format)
- [Examples](#examples)
  - [Example 1: Single Store Served](#example-1-single-store-served)
  - [Example 2: Both Warehouses Used](#example-2-both-warehouses-used)
  - [Example 3: Cost Minimization with Single Store](#example-3-cost-minimization-with-single-store)
- [Solution Approach](#solution-approach)
  - [1. Shortest Path Computation (Dijkstra's Algorithm)](#1-shortest-path-computation-dijkstras-algorithm)
  - [2. Bipartite Graph Construction](#2-bipartite-graph-construction)
  - [3. Min-Cost Max-Flow using Successive Shortest Paths](#3-min-cost-max-flow-using-successive-shortest-paths)
  - [4. Flow Reconstruction](#4-flow-reconstruction)
- [Total Complexity](#total-complexity)
- [Tests](#tests)
- [Makefile Usage](#makefile-usage)
- [Author](#author)

## Problem Description

Given a graph G with three types of nodes:
- **Warehouses** (w₁, w₂, ..., wₖ): Source locations with limited truck capacity
- **Stores** (s₁, s₂, ..., sₙ): Destination locations with time window constraints
- **Regular vertices** (v₁, v₂, ..., vₘ): Intermediate nodes in the graph

Each edge has two weights:
- **Cost**: The monetary cost of traversing the edge
- **Time**: The time required to traverse the edge

Each warehouse has:
- A maximum capacity (number of trucks available)

Each store has:
- A time window [L, R] during which it can receive deliveries

**Important Constraint**: All trucks depart from their warehouses at time t = 0. Warehouses cannot delay departures to meet time windows. This means a warehouse W can serve store S only if the travel time from W to S falls within S's time window: `L[S] ≤ travel_time[W][S] ≤ R[S]`.

This represents a just-in-time delivery system where all warehouses operate on a synchronized schedule and cannot hold inventory for delayed dispatch.

**Objective**: Find the minimum-cost assignment of warehouses to stores such that:
1. Each store is served by at most one warehouse
2. No warehouse exceeds its truck capacity
3. The delivery time from warehouse to store falls within the store's time window (with all trucks departing at t = 0)
4. The total cost is minimized

**Output**:
- The number of stores that can be served
- For each served store: warehouse ID, store ID, delivery cost, delivery time

## Input Format

```
V E                         # Number of vertices and edges
W S                         # Number of warehouses and stores
v₁ u₁ cost₁ time₁          # Edge from v₁ to u₁
v₂ u₂ cost₂ time₂
...
vₑ uₑ costₑ timeₑ
w₁ cap₁                     # Warehouse w₁ with capacity cap₁
w₂ cap₂
...
wₖ capₖ
s₁ L₁ R₁                    # Store s₁ with time window [L₁, R₁]
s₂ L₂ R₂
...
sₙ Lₙ Rₙ
```

## Output Format

```
N                           # Number of stores served
warehouse₁ store₁ cost₁ time₁
warehouse₂ store₂ cost₂ time₂
...
warehouseₙ storeₙ costₙ timeₙ
```

---

## Examples

### Example 1: Single Store Served

**Input:**
```
5 4
2 2
1 2 10 5
1 4 20 2
2 3 15 3
4 3 5 4
1 1
4 1
2 2 7
3 5 8
```

**Explanation:**
- Graph is **directed** with 5 vertices and 4 edges
- Warehouse 1 (capacity 1) can reach:
  - Store 2: cost=10, time=5 ✅ (5 ∈ [2,7])
  - Store 3: cost=25, time=8 ✅ (via path 1→2→3: time=5+3=8 ∈ [5,8])
- Warehouse 4 (capacity 1) can reach:
  - Store 3: cost=5, time=4 ❌ (4 ∉ [5,8])

Since warehouse 1 has capacity 1, it can only serve one store.
The algorithm chooses to serve store 2, leaving store 3 unserved,
as the cost of serving store 2 is the minimum between the two.

**Output:**
```
1
1 2 10 5
```

---

### Example 2: Both Warehouses Used

**Input:**
```
5 4
2 2
1 2 10 5
1 4 20 2
2 3 15 3
4 3 5 6
1 1
4 1
2 2 7
3 5 8
```

**Explanation:**
- Graph is **directed** with 5 vertices and 4 edges
- Note: Edge 4→3 now has **time=6** (changed from Example 1)
- Warehouse 1 (capacity 1) can reach:
  - Store 2: cost=10, time=5 ✅ (5 ∈ [2,7])
  - Store 3: cost=25, time=8 ✅ (8 ∈ [5,8])
- Warehouse 4 (capacity 1) can reach:
  - Store 3: cost=5, time=6 ✅ (6 ∈ [5,8])

Both stores can now be served:
- Warehouse 1 → Store 2 (cost=10)
- Warehouse 4 → Store 3 (cost=5, cheaper than warehouse 1's path with cost=25)

**Output:**
```
2
1 2 10 5
4 3 5 6
```

---

### Example 3: Cost Minimization with Single Store

**Input:**
```
5 4
2 1
1 2 10 5
1 4 20 2
2 3 15 3
4 3 5 6
1 1
4 1
3 5 8
```

**Explanation:**
- Only one store (store 3) with time window [5,8]
- Warehouse 1 can reach store 3: cost=25, time=8 ✅ (via 1→2→3)
- Warehouse 4 can reach store 3: cost=5, time=6 ✅ (direct edge)

The **min-cost max-flow** algorithm chooses warehouse 4 because it provides the cheapest path (cost=5 vs cost=25).

**Output:**
```
1
4 3 5 6
```

**Key Insight:** This demonstrates that when multiple warehouses can serve the same store,
the algorithm selects the one with minimum cost.

---

**Note:** The graph is **directed**, so paths must follow edge directions.
The solution uses min-cost max-flow (successive shortest paths) to maximize the number of stores
served while minimizing total delivery cost.

---


## Solution Approach

The solution combines multiple algorithmic techniques:

### 1. Shortest Path Computation (Dijkstra's Algorithm)
For each warehouse w:
- Run Dijkstra's algorithm on the graph using **lexicographic weight ordering**:
  - Weights are compared by cost first, then by time as a tiebreaker
  - This finds paths that minimize cost primarily, with time as a secondary consideration
  - Result: `min[w][s]` contains the minimum cost (and associated time) from warehouse w to store s

**Complexity**: O(k × |E| log |V|) where k is the number of warehouses

### 2. Bipartite Graph Construction
Build a bipartite flow network:
- **Left side**: Source node → Warehouses (capacity = warehouse capacity)
- **Right side**: Stores → Sink node (capacity = 1 per store)
- **Middle edges**: Warehouse w → Store s exists if and only if:
  ```
  L[s] ≤ min[w][s].time ≤ R[s]
  ```
  (i.e., the minimum-cost path's travel time falls within the store's time window when departing at t = 0)


### 3. Min-Cost Max-Flow using Successive Shortest Paths

After constructing the flow network:

* **Edges:**

  * Source → Warehouses: capacity = warehouse truck capacity, cost = 0
  * Warehouse → Store: capacity = 1 (if time-feasible), cost = delivery cost (time recorded separately)
  * Store → Sink: capacity = 1, cost = 0

* **Algorithm:**

  1. While there exists a path from source to sink with remaining capacity:

     * Find the **shortest path in terms of total cost**, using Dijkstra with lexicographic comparison `(cost, time)`
     * Push the maximum possible flow along this path (min of capacities along path)
     * Update residual capacities
  2. Repeat until no augmenting path exists

* This ensures that:

  * The **maximum number of stores** are served
  * Among all feasible maximum flows, the **total delivery cost is minimized**

**Complexity:**

* Each augmenting path is found using Dijkstra: `O(E log V)`
* If the maximum flow is `F`, total complexity is `O(F × E log V)`

---

### 4. Flow Reconstruction

After computing the min-cost max-flow:

1. Examine the **residual graph** to determine which warehouse sends flow to which store
2. For each store that receives flow, output:

   ```
   warehouse_id store_id delivery_cost delivery_time
   ```
3. This guarantees:

   * No warehouse exceeds its capacity
   * Each store is served by at most one warehouse
   * Delivery times respect the store time windows
   * Total cost is minimized

## Total Complexity

The overall complexity of the program is:

$$
O(\|W\| \cdot E \log V + F \cdot \|W\| \cdot \|S\| \log(\|W\| + \|S\|))
$$

Where:

- **|W|** = number of warehouses  
- **|S|** = number of stores  
- **E** = number of edges in the original graph  
- **V** = number of vertices in the original graph  
- **F** = maximum flow (≤ sum of warehouse capacities)

**Notes:**

1. The first term, $O(\|W\| \cdot E \log V)$, comes from running **Dijkstra's algorithm** from each warehouse to compute minimum-cost paths.  
2. The second term, $O(F \cdot \|W\| \cdot \|S\| \log(\|W\| + \|S\|))$, comes from the **successive shortest paths min-cost max-flow** algorithm.
3. If the graph is sparse ($E \approx V$), the first term is roughly:

$$
O(\|W\| \cdot V \log V)
$$

## Tests

**Test file format**: (in the `test/` directory)
- Place input files with `.in` extension.
- Place output files with `.out` extension.

The first 5 tests served to address the algorithm, in simple and immediate cases, where it was going wrong.
They were hand crafted and thats where the solution came from.
The following 5 tests were done by `test.cpp` with input parameters and then solved by hand to reach its solution
and then compared to the output of the program.
With confidence in the algorithm, the following 5 test cases are for other solutions to be compared with the current on
more complex graphs.
`test.cpp` constructs a graph with no edges initially with the input, then connects
random edges picked from the strongly connected sub-components of the graph until it is a single
strongly connect component. This way, the solution program has more paths to visit, just like in a real
implementation of this algorithm to serve a company would go through.

I thought of trying to make a test generator from a given input solution, however, it seemed to complex and prone to failure
in test cases. Because this problem is, while with some complexity, straight forward, I did opt to just hand craft most test
cases.

---

### Makefile Usage

```bash
# Compile the program
make

# Compile and run
make run

# Compile with debug flags and sanitizers
make debug

# Run all test cases from test/ directory
make test

# Clean build artifacts
make clean
```

---

## Author

**Henrique Teixeira**

Competitive Programming - L.IACD - FCUP 25/26
