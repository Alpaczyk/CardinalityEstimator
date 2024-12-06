#include "SimpleGraph.h"
#include "SimpleEstimator.h"
#include "SimpleEvaluator.h"
#include <random>
#include <unordered_set>

SimpleEstimator::SimpleEstimator(std::shared_ptr<SimpleGraph> &g) {
    graph = g;
}

void SimpleEstimator::prepare() {
    const uint32_t NoEdges = graph->getNoEdges();
    const uint32_t sampleSize = NoEdges / 5; // Determine sample size

    // Get original adjacency list
    std::vector<std::vector<std::pair<uint32_t, uint32_t>>> const originalAdj = graph->adj;

    // Create new vector with all edges
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> allEdges;
    for (uint32_t s = 0; s < originalAdj.size(); ++s) {
        for (const auto &edge : originalAdj[s]) {
            allEdges.emplace_back(s, edge.first, edge.second);
        }
    }

    // Randomly sample edges without replacement
    std::vector<std::tuple<uint32_t, uint32_t, uint32_t>> sampledEdges;
    std::srand(std::time(nullptr)); // Seed for randomness
    // We shuffle the edges to get a random sample
    std::shuffle(allEdges.begin(), allEdges.end(), std::default_random_engine(std::rand()));
    // Take the first sampleSize edges
    sampledEdges.assign(allEdges.begin(), allEdges.begin() + sampleSize);

    // Build the new adjacency matrix
    std::vector<std::vector<std::pair<uint32_t, uint32_t>>> sampledAdj(originalAdj.size());
    for (const auto &edge : sampledEdges) {
        uint32_t source = std::get<0>(edge);
        uint32_t label = std::get<1>(edge);
        uint32_t target = std::get<2>(edge);
        sampledAdj[source].emplace_back(label, target); // Add edge to the new adjacency list
    }

    // Create new graph instance
    sampledGraph = std::make_shared<SimpleGraph>();

    // Set the number of vertices and labels
    sampledGraph->setNoVertices(graph->getNoVertices());
    sampledGraph->setNoLabels(graph->getNoLabels());

    // Add the edges to the new graph
    for (uint32_t s = 0; s < sampledAdj.size(); ++s) {
        for (const auto &edge : sampledAdj[s]) {
            sampledGraph->addEdge(s, edge.second, edge.first);
        }
    }

    // Set up evaluator instance
    sampleEvaluator = std::make_shared<SimpleEvaluator>(sampledGraph);
    sampleEvaluator->prepare();
}

cardStat SimpleEstimator::estimate(Triple &q) {
    // Perform evaluation on the pre-sampled graph
    auto [noOut, noPaths, noIn] = sampleEvaluator->evaluate(q);

    // Scale result up to the size of the graph
    constexpr uint32_t scalingFactor = 5;
    return cardStat {
            noOut * scalingFactor,
            noPaths * scalingFactor,
            noIn * scalingFactor
    };
}