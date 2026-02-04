#include <iostream>
#include <random>
#include <algorithm>

#include <vector>
#include <set>
#include <map>
#include <queue>

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <climits>
#include <cassert>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef float    f32;
typedef double   f64;

struct Weight {
  i32 cost, time;

  bool operator<(const Weight& other) const {
    return 
      cost < other.cost || 
      (cost == other.cost && time < other.time)
    ;
  }

  bool operator==(const Weight& other) const {
    return cost == other.cost && time == other.time;
  }

  bool operator!=(const Weight& other) const {
    return cost != other.cost || time != other.time;
  }

  bool operator>(const Weight& other) const {
    return
      cost > other.cost || 
      (cost == other.cost && time > other.time)
    ;
  }

  Weight operator+(const Weight& other) const {
    return { cost + other.cost, time + other.time };
  }
};
#define WEIGHT_MAX  Weight{ INT_MAX, INT_MAX }
#define WEIGHT_ZERO Weight{ 0, 0 }

struct Edge {
  u32 to;
  Weight weight;
};

using Graph = std::vector<std::vector<Edge>>;
// s_nodes + 1, range nodes 1 to n

Graph graph_init(const u32 s_nodes);
u32 graph_size(const Graph& g);
std::vector<Weight> dijkstra(const Graph& graph, const u32 start);

void dfs_tarjan(
  const Graph& graph, std::vector<u32>& ids, std::vector<u32>& low,
  std::vector<bool>& on_stack, std::vector<u32>& stack,
  u32& id, u32& scc_count, const u32 vertex
);

std::vector<u32> graph_scc_tarjan(const Graph& graph, u32& s_sccs);

struct Match {
  u32 w, s, cost, time;
};

struct Warehouse {
  u32 w, cap;
};

struct Store {
  u32 s, time_start, time_end;
};

struct Test {
  u32 s_edges;
  Graph graph;
  std::vector<Warehouse> warehouses;
  std::vector<Store> stores;
  std::vector<Match> solutions;
};

void create_edge(
  Graph& graph, const u32 v, const u32 u,
  const i32 max_cost, const i32 min_cost,
  const i32 max_time, const i32 min_time
) {
  graph[v].push_back(Edge{
    .to   = u,
    .weight = {
      .cost = (rand() % (max_cost - min_cost)) + min_cost,
      .time = (rand() % (max_time - min_time)) + min_time
    }
  });
}

u32 create_random_edge(
  Graph& graph, std::set<std::pair<u32, u32>>& used_edges,
  const i32 max_cost, const i32 min_cost,
  const i32 max_time, const i32 min_time
) {
  const u32 s_vertices = graph_size(graph);
  u32 v = 0, u = 0;

  do {
    u32 offset = ((rand() & 1) ? ((rand() % s_vertices / 4) + 1) : ((rand() % s_vertices / 3) + 1));

    v = (rand() % s_vertices) + 1;
    u = ((v + offset) % s_vertices) + 1;

  } while (
    v == u ||
    used_edges.find({ v, u }) != used_edges.end() ||
    used_edges.find({ u, v }) != used_edges.end()
  );

  assert(v != u);

  if (rand() & 1) {
    create_edge(graph, v, u, max_cost, min_cost, max_time, min_time);
    used_edges.insert({ v, u });
    return 1;

  } else if (rand() & 1) {
    create_edge(graph, u, v, max_cost, min_cost, max_time, min_time);
    used_edges.insert({ u, v });
    return 1;

  } else {
    create_edge(graph, v, u, max_cost, min_cost, max_time, min_time);
    create_edge(graph, u, v, max_cost, min_cost, max_time, min_time);

    used_edges.insert({ v, u });
    used_edges.insert({ u, v });
    return 2;
  }
}

void build_test(
  Test& test, std::mt19937& gen,
  const u32 max_cap,  const u32 min_cap,
  const i32 max_cost, const i32 min_cost,
  const i32 max_time, const i32 min_time
) {
  const u32 s_vertices = graph_size(test.graph);

  u32 s_sccs = 0; // strongly connected components
  test.s_edges = 0;

  std::set<std::pair<u32, u32>> used_edges;
  for (u32 i = 0; i < s_vertices / 2; i++)
    test.s_edges += create_random_edge(test.graph, used_edges, max_cost, min_cost, max_time, min_time);

  while (true) {
    std::vector<u32> sccs = graph_scc_tarjan(test.graph, s_sccs);
    if (s_sccs == 1)
      break;

    // graph_print(test.graph);

    std::vector<std::vector<u32>> v_by_scc(s_sccs);

    std::map<u32, u32> id_map;
    u32 next_id = 0;

    for (u32 v = 1; v <= s_vertices; v++) {
      if (id_map.find(sccs[v]) == id_map.end())
        id_map[sccs[v]] = next_id++;
      v_by_scc[id_map[sccs[v]]].push_back(v);
    }

    const u32 s_edges = std::max((u32)1, rand() % (s_sccs - 1));
    for (u32 i = 0; i < s_edges; i++) {
      const u32 
        scc_v = rand() % s_sccs,
        scc_u = (scc_v + rand() % (s_sccs - 1) + 1) % s_sccs;

      const u32
        v = v_by_scc[scc_v][rand() % v_by_scc[scc_v].size()],
        u = v_by_scc[scc_u][rand() % v_by_scc[scc_u].size()];

      assert(s_sccs > 1);
      assert(scc_v != scc_u);
      assert(v != u);

      if (used_edges.insert({v, u}).second) {
        create_edge(test.graph, v, u, max_cost, min_cost, max_time, min_time);
        test.s_edges++;
      } else {
        i--; // Retry this iteration
      }
    }
  }

  std::vector<u32> pool(s_vertices);
  for (u32 i = 0; i < s_vertices; i++)
    pool[i] = i + 1;

  // Shuffle using the random engine
  std::shuffle(pool.begin(), pool.end(), gen);

  // Take first s_warehouses elements
  const u32 s_warehouses = (u32)test.warehouses.size();
  for (u32 i = 0; i < s_warehouses; i++) {
    test.warehouses[i].w   = pool[i];
    test.warehouses[i].cap = (rand() % (max_cap - min_cap)) + min_cap;
  }

  const u32 s_stores = (u32)test.stores.size();
  for (u32 i = 0; i < s_stores; i++) {
    test.stores[i].s          = pool[s_warehouses + i];

    u32 time_start = rand() % (4 * min_time);
    test.stores[i].time_start = time_start;
    test.stores[i].time_end   = 4 * max_time - rand() % (6 * min_time);
  }
}

void print_test(const Test& test) {
  const u32 
    s_vertices   = graph_size(test.graph),
    s_warehouses = (u32)test.warehouses.size(),
    s_stores     = (u32)test.stores.size();

  std::cout << s_vertices << " " << test.s_edges << std::endl;
  std::cout << s_warehouses << " " << s_stores << std::endl;

  for (u32 i = 1; i <= s_vertices; i++) {
    const u32 s_neighbours = (u32)test.graph[i].size();

    for (u32 j = 0; j < s_neighbours; j++) {
      std::cout << i << " " << test.graph[i][j].to << " "
                << test.graph[i][j].weight.cost << " " << test.graph[i][j].weight.time
                << std::endl;
    }
  }

  for (u32 i = 0; i < s_warehouses; i++)
    std::cout << test.warehouses[i].w << " " << test.warehouses[i].cap << std::endl;

  for (u32 i = 0; i < s_stores; i++)
    std::cout << test.stores[i].s << " " << test.stores[i].time_start << " " << test.stores[i].time_end << std::endl;
}

i32 main(const i32 argc, const char* argv[]) {
  if (argc < 2) {
    std::cerr << "[ERROR]: need argument with the ammount of tests to be generated" << std::endl;
    exit(1);
  }

  srand((u32)time(nullptr));

  std::random_device rd;
  std::mt19937 gen(rd());
 
  const u32 s_tests = (u32)std::stoi(argv[1]);
  std::vector<Test> tests(s_tests);
  for (u32 i = 0; i < s_tests; i++) {
    u32 
      s_vertices   = 0,
      s_warehouses = 0,
      s_stores     = 0,
      max_cap      = 0,
      min_cap      = 0;
    i32 
      max_time     = 0,
      min_time     = 0,
      max_cost     = 0,
      min_cost     = 0;

    std::cin >> s_vertices >> s_warehouses >> s_stores;

    if (s_warehouses + s_stores > s_vertices / 2) {
      std::cerr << "[ERROR]: s_warehouses + s_stores > s_vertices / 2" << std::endl;
      exit(1);
    }

    std::cin >> max_cap >> min_cap;
    assert(max_cap > min_cap && min_cap > 0);

    std::cin >> max_cost >> min_cost;
    assert(max_cost > min_cost && min_cost > 0);

    std::cin >> max_time >> min_time;
    assert(max_time > min_time && min_time > 0);

    /*
    std::cout << "s_vertices = " << s_vertices
              << " s_warehouses = " << s_warehouses
              << " s_stores = " << s_stores << std::endl
              << "max_cap = " << max_cap
              << " min_cap = " << min_cap << std::endl
              << "max_cost = " << max_cost
              << " min_cost = " << min_cost << std::endl
              << "max_time = " << max_time
              << " min_time = " << min_time << std::endl;

     * */
    
    tests[i].graph = graph_init(s_vertices);
    tests[i].warehouses.resize(s_warehouses, { 0, 0 });
    tests[i].stores.resize(s_stores, { 0, 0, 0 });

    build_test(
      tests[i], gen,
      max_cap, min_cap,
      max_cost, min_cost,
      max_time, min_time
    );
    print_test(tests[i]);
  }

  return 0;
}

Graph graph_init(const u32 s_nodes) {
  return std::vector(s_nodes + 1, std::vector<Edge>());
}

u32 graph_size(const Graph& g) {
  return (u32)g.size() - 1;
}

void graph_print(const Graph& g) {
  const u32 s_vertices = graph_size(g);
  for (u32 v = 1; v <= s_vertices; v++) {
    std::cout << v << ": ";
    for (Edge edge : g[v])
      std::cout << "(" << edge.to << ", "
                << edge.weight.cost << ", "
                << edge.weight.time << ") ";
    std::cout << std::endl;
  }
}

std::vector<Weight> dijkstra(const Graph& graph, const u32 start) {
  std::vector<Weight> dist(graph_size(graph) + 1, WEIGHT_MAX);
  dist[start] = WEIGHT_ZERO;

  auto comp = [](const std::pair<u32, Weight>& a, const std::pair<u32, Weight>& b) -> bool {
    return a.second > b.second; // min heap comparison
  };

  std::priority_queue<
    std::pair<u32, Weight>,
    std::vector<std::pair<u32, Weight>>,
    decltype(comp)> q(comp);
  q.push({ start, WEIGHT_ZERO });

  while (!q.empty()) {
    const u32    vertex = q.top().first;
    const Weight d      = q.top().second;
    q.pop();

    if (d > dist[vertex])
      continue;

    const u32 s_neighbours = (u32)graph[vertex].size();
    for (u32 i = 0; i < s_neighbours; i++) {
      const Edge   edge      = graph[vertex][i];
      const u32    neighbour = edge.to;
      const Weight weight    = edge.weight;

      if (dist[neighbour] > dist[vertex] + weight) {
        dist[neighbour] = dist[vertex] + weight;
        q.push({ neighbour, dist[neighbour] }); // lazy approach, still efficient
      }
    }
  }

  return dist;
}

void dfs_tarjan(
  const Graph& graph, std::vector<u32>& ids, std::vector<u32>& low,
  std::vector<bool>& on_stack, std::vector<u32>& stack,
  u32& id, u32& scc_count, const u32 vertex
) {
  ids[vertex] = low[vertex] = id++;
  stack.push_back(vertex);
  on_stack[vertex] = true;
  
  const u32 s_neighbours = (u32)graph[vertex].size();
  for (u32 i = 0; i < s_neighbours; i++) {
    const u32 neighbour = graph[vertex][i].to;

    if (ids[neighbour] == 0)
      dfs_tarjan(graph, ids, low, on_stack, stack, id, scc_count, neighbour);

    if (on_stack[neighbour])
      low[vertex] = std::min(low[vertex], low[neighbour]);
  }
  
  // Found SCC root
  if (ids[vertex] == low[vertex]) {
    while (true) {
      u32 node = stack.back();
      stack.pop_back();

      on_stack[node] = false;
      low[node] = ids[vertex]; // Assign SCC id
      
      if (node == vertex)
        break;
    }

    scc_count++;
  }
}

std::vector<u32> graph_scc_tarjan(const Graph& graph, u32& s_scc) {
  const u32 n = graph_size(graph);
  std::vector<u32> ids(n + 1, 0);      // Node ids
  std::vector<u32> low(n + 1, 0);       // Low-link values (also used for SCC id)
  std::vector<bool> on_stack(n + 1, false);
  std::vector<u32> stack;

  u32 id = 1;
  s_scc = 0;
  
  for (u32 i = 1; i <= n; i++)
    if (ids[i] == 0)
      dfs_tarjan(graph, ids, low, on_stack, stack, id, s_scc, i);
  
  return low; // low[vertex] = SCC id
}
