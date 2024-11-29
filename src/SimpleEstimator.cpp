#include "SimpleGraph.h"
#include "SimpleEstimator.h"
#include "SimpleEvaluator.h"
#include <random>
// Cardinality estimator using sampling without replacement

SimpleEstimator::SimpleEstimator(std::shared_ptr<SimpleGraph> &g){

    // works only with SimpleGraph
    graph = g;
}

auto sample_graph = std::make_shared<SimpleGraph>();

void SimpleEstimator::prepare() {
    uint32_t sampleSize = 5000;
    uint32_t vertices = graph->getNoVertices();

    std::vector<std::vector<std::pair<uint32_t, uint32_t>>> const adj = graph->adj;

    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> edges;
    for (uint32_t s = 0; s < adj.size(); ++s) {
        for (const auto &edge : adj[s]) {
            edges.emplace_back(s, edge.first, edge.second);
        }
    }
    // Select a random sample of sampleSize from the graph on which we will run the query
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> sample;
    std::random_device dev;
    std::mt19937 rng(dev());
    // Blacklist indices of already chosen edges
    std::vector<bool> blacklist(edges.size(), false);
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, edges.size() - 1);
    for (uint32_t i = 0; i < sampleSize; ++i) {
        // Choose a random edge and assign to variable chosen using repeat
        uint32_t chosen;
        do {
            chosen = dist(rng);
        } while (blacklist[chosen]);
        sample.push_back(edges[chosen]);
        // remove chosen edge from edges (no replacement)
        blacklist[chosen] = true;
    }
    // Build sample adjency matrix
    std::vector<std::vector<std::pair<uint32_t, uint32_t>>> sampleAdj(adj.size());
    for (const auto &edge : sample) {
        uint32_t source = std::get<0>(edge);
        uint32_t label = std::get<1>(edge);
        uint32_t target = std::get<2>(edge);
        sampleAdj[source].emplace_back(label, target); // Add edge to the new adjacency list
    }

    // Create new graph instance
//    sample_graph =

    // Set the number of vertices and labels
    sample_graph->setNoVertices(vertices);
    sample_graph->setNoLabels(graph->getNoLabels());
    // Add the edges to the new graph
    for (uint32_t s = 0; s < sampleAdj.size(); ++s) {
        for (const auto &edge : sampleAdj[s]) {
            sample_graph->addEdge(s, edge.second, edge.first);
        }
    }
}

cardStat SimpleEstimator::estimate(Triple &q) {

    // perform evaluation
    auto ev = std::make_unique<SimpleEvaluator>(sample_graph);
    ev->prepare();
    auto result = ev->evaluate(q);
    // Scale result up to the size of the graph
    double scalingFactor = graph->getNoEdges() / sample_graph->getNoEdges();
    return cardStat {
            static_cast<uint32_t>(result.noOut * scalingFactor),
            static_cast<uint32_t>(result.noPaths * scalingFactor),
            static_cast<uint32_t>(result.noIn * scalingFactor)
    };

    // return cardStat {100, 100, 100};
}