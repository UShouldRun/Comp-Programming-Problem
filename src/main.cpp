#include <iostream>
#include <vector>
#include <queue>
#include <functional>
#include <iomanip>

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <climits>

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

struct FlowCostGraph {
  std::vector<std::vector<i32>> cap;
  std::vector<std::vector<Weight>> weight;
  std::vector<std::vector<u32>> adj;
};

struct FlowCostGraphState {
  u32 vertex;
  i32 flow;
  Weight weight;
  
  bool operator>(const FlowCostGraphState& other) const {
    return weight > other.weight; // Min-heap
  }
  bool operator<(const FlowCostGraphState& other) const {
    return weight < other.weight; // Min-heap
  }
};

FlowCostGraph flow_cost_graph_init(const u32 n);
u32 flow_cost_graph_size(const FlowCostGraph& g);
void flow_cost_graph_add_edge(FlowCostGraph& graph, const u32 v, const u32 u, const i32 c, const Weight& w);
i32 flow_cost_graph_dijkstra(const FlowCostGraph& graph, std::vector<u32>& parent, const u32 source, const u32 target);
i32 flow_cost_graph_max_flow(FlowCostGraph& graph, const u32 source, const u32 target);

Graph read(
  std::vector<u32>& warehouses, std::vector<u32>& stores,
  std::vector<u32>& warehouse_capacity, std::vector<std::pair<i32, i32>>& time_constraint,
  u32& s_warehouses, u32& s_stores
) {
  u32 
    s_vertices = 0,
    s_edges = 0;
  std::cin >> s_vertices >> s_edges;
  std::cin >> s_warehouses >> s_stores;

  Graph graph = graph_init(s_vertices + 1);
  for (u32 i = 0; i < s_edges; i++) {
    u32 v = 0, w = 0;
    i32 cost = 0, time = 0;

    std::cin >> v >> w >> cost >> time;

    graph[v].push_back(
      Edge{
        .to = w,
        .weight = {
          .cost = cost,
          .time = time
        }
      }
    );
  }

  warehouses         = std::vector<u32>(s_warehouses, 0);
  warehouse_capacity = std::vector<u32>(s_warehouses, 0);

  for (u32 i = 0; i < s_warehouses; i++) {
    u32 w = 0, cap = 0;
    std::cin >> w >> cap;
    warehouses[i] = w;
    warehouse_capacity[i] = cap;
  }

  stores = std::vector<u32>(s_stores, 0);
  time_constraint = std::vector<std::pair<i32, i32>>(s_stores, {0, 0});
  for (u32 i = 0; i < s_stores; i++) {
    u32 s = 0;
    i32 t_start = 0, t_end = 0;
    std::cin >> s >> t_start >> t_end;
    stores[i] = s;
    time_constraint[i] = { t_start, t_end };
  }

  return graph;
}

void print_min(
  const std::vector<std::vector<Weight>>& min, 
  const std::vector<u32>& warehouses, const std::vector<u32>& stores,
  const u32 s_warehouses, const u32 s_stores
) {
  std::vector<u32> col_widths(s_stores, 0);
  for (u32 j = 0; j < s_stores; j++) {
    col_widths[j] = (u32)std::to_string(stores[j]).size();
    for (u32 i = 0; i < s_warehouses; i++) {
      std::string cell = "(" + std::to_string(min[i][j].cost) + ", " + std::to_string(min[i][j].time) + ")";
      col_widths[j] = std::max(col_widths[j], (u32)cell.size());
    }
  }

  u32 row_label_width = 0;
  for (u32 i = 0; i < s_warehouses; i++)
    row_label_width = std::max(row_label_width, (u32)std::to_string(warehouses[i]).size());

  // Print header
  std::cout << std::string(row_label_width + 1, ' ');
  for (u32 j = 0; j < s_stores; j++)
    std::cout << std::setw(col_widths[j]) << stores[j] << " ";
  std::cout << "\n";

  // Print rows
  for (u32 i = 0; i < s_warehouses; i++) {
    std::cout << std::setw(row_label_width) << warehouses[i] << " ";
    for (u32 j = 0; j < s_stores; j++) {
      std::string cell = "(" + std::to_string(min[i][j].cost) + ", " + std::to_string(min[i][j].time) + ")";
      std::cout << std::setw(col_widths[j]) << cell << " ";
    }
    std::cout << "\n";
  }
}

void solve() {
  u32 
    s_warehouses = 0,
    s_stores     = 0;

  std::vector<u32> warehouse_capacity;
  std::vector<std::pair<i32, i32>> time_constraint;
  std::vector<u32> warehouses, stores;
  Graph graph = read(warehouses, stores, warehouse_capacity, time_constraint, s_warehouses, s_stores);

  std::vector<std::vector<Weight>>
    min = std::vector(s_warehouses, std::vector<Weight>(s_stores, WEIGHT_MAX));

  for (u32 i = 0; i < s_warehouses; i++) {
    const u32 w = warehouses[i];
    std::vector<Weight> d = dijkstra(graph, w);

    for (u32 j = 0; j < s_stores; j++) {
      const u32 s = stores[j];
      min[i][j] = std::min(min[i][j], d[s]);
    }
  }

  // print_min(min, warehouses, stores, s_warehouses, s_stores);

  const u32
    source = 1,
    target = 1 + s_warehouses + s_stores + 1;
  FlowCostGraph flow_cost_graph = flow_cost_graph_init(target);

  for (u32 i = 0; i < s_warehouses; i++) {
    const u32 warehouse = source + i + 1;
    flow_cost_graph_add_edge(
      flow_cost_graph,
      source, warehouse,
      warehouse_capacity[i],
      WEIGHT_ZERO
    );
    
    for (u32 j = 0; j < s_stores; j++) {
      const std::pair<i32, i32> store_time = time_constraint[j];
      const i32 min_time = min[i][j].time;

      if (store_time.first <= min_time && min_time <= store_time.second) {
        const u32 store = source + s_warehouses + 1 + j;
        flow_cost_graph_add_edge(
            flow_cost_graph,
            warehouse, store,
            1,
            min[i][j]
        );
      }
    }
  }

  for (u32 i = 0; i < s_stores; i++) {
    const u32 store = source + s_warehouses + 1 + i;
    flow_cost_graph_add_edge(
      flow_cost_graph,
      store, target,
      1,
      WEIGHT_ZERO
    );
  }

  const u32 count = flow_cost_graph_max_flow(flow_cost_graph, source, target);
  std::cout << count << std::endl;

  for (u32 i = 0; i < s_stores; i++) {
    const u32 s = source + s_warehouses + 1 + i;

    for (u32 j = 0; j < s_warehouses; j++) {
      const u32 w = source + 1 + j;
      const i32 cap = flow_cost_graph.cap[s][w];
      const Weight weight = flow_cost_graph.weight[s][w];

      if (cap == 1) {
        std::cout << warehouses[j] << " " << stores[i] << " "
                  << weight.cost << " " << weight.time
                  << std::endl;
        break;
      }
    }
  }
}

i32 main() {
  solve();
  return 0;
}

Graph graph_init(const u32 s_nodes) {
  return std::vector(s_nodes + 1, std::vector<Edge>());
}

u32 graph_size(const Graph& g) {
  return (u32)g.size() - 1;
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

FlowCostGraph flow_cost_graph_init(const u32 n) {
  FlowCostGraph g;
  g.adj.resize(n + 1);
  g.weight.resize(n + 1, std::vector<Weight>(n + 1, WEIGHT_ZERO));
  g.cap.resize(n + 1, std::vector<i32>(n + 1, 0));
  return g;
}

u32 flow_cost_graph_size(const FlowCostGraph& g) {
  return (u32)g.adj.size() - 1;
}

void flow_cost_graph_add_edge(FlowCostGraph& graph, const u32 from, const u32 to, const i32 c, const Weight& w) {
  graph.adj[from].push_back(to);
  graph.adj[to].push_back(from);

  graph.weight[from][to] = w;
  graph.weight[to][from] = w;

  graph.cap[from][to]    = c;
  graph.cap[to][from]    = 0;
}

i32 flow_cost_graph_dijkstra(const FlowCostGraph& graph, std::vector<u32>& parent, const u32 source, const u32 target) {
  const u32 n = flow_cost_graph_size(graph);

  std::vector<Weight> dist(n + 1, WEIGHT_MAX);
  std::vector<i32> min_flow(n + 1, 0);

  dist[source]     = WEIGHT_ZERO;
  min_flow[source] = INT_MAX;

  std::priority_queue<
    FlowCostGraphState,
    std::vector<FlowCostGraphState>,
    std::greater<FlowCostGraphState>
  > pq;

  pq.push({ source, INT_MAX, WEIGHT_ZERO });

  while (!pq.empty()) {
    const auto [vertex, flow, weight] = pq.top();
    pq.pop();

    if (weight > dist[vertex]) // Lazy skip, same as your graph Dijkstra
      continue;

    if (vertex == target)
      return flow;

    for (const u32 neighbour : graph.adj[vertex]) {
      const i32 capacity = graph.cap[vertex][neighbour];

      if (capacity <= 0)
        continue;

      const Weight new_weight = weight + graph.weight[vertex][neighbour];

      if (new_weight < dist[neighbour]) {
        parent[neighbour]   = vertex;
        dist[neighbour]     = new_weight;
        min_flow[neighbour] = std::min(flow, capacity);

        pq.push({ neighbour, min_flow[neighbour], new_weight });
      }
    }
  }

  return 0;
}

i32 flow_cost_graph_max_flow(FlowCostGraph& graph, const u32 source, const u32 target) {
  const u32 n = flow_cost_graph_size(graph);

  i32 
    flow = 0,
    new_flow;

  std::vector<u32> parent(n + 1);

  while ((new_flow = flow_cost_graph_dijkstra(graph, parent, source, target)) > 0) {
    flow += new_flow;
    u32 curr = target;

    while (curr != source) {
      const u32 prev = parent[curr];
      
      graph.cap[prev][curr] -= new_flow;
      graph.cap[curr][prev] += new_flow;

      curr = prev;
    }
  }

  return flow;
}
