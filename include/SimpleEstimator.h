#ifndef QS_SIMPLEESTIMATOR_H
#define QS_SIMPLEESTIMATOR_H

#include "Estimator.h"
#include "SimpleGraph.h"

class SimpleEstimator : public Estimator {

    std::shared_ptr<SimpleGraph> graph;
    std::shared_ptr<SimpleGraph> sampledGraph;
    std::unordered_map<unsigned, std::vector<std::pair<unsigned, unsigned>>> histogramOutVertex;
    std::unordered_map<unsigned, std::vector<std::pair<unsigned, unsigned>>> histogramInVertex;
    std::unordered_map<uint32_t, std::vector<std::pair<std::pair<uint32_t, uint32_t>, uint32_t>>> histogramOutDepth;
    std::unordered_map<uint32_t, std::vector<std::pair<std::pair<uint32_t, uint32_t>, uint32_t>>> histogramInDepth;
    uint32_t binDepth{};

public:
    explicit SimpleEstimator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEstimator() = default;

    std::unordered_map<uint32_t, std::vector<std::pair<std::pair<uint32_t, uint32_t>, uint32_t>>> buildEquiDepthHistogram(
    const std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t>>& outVertexCount, uint32_t noOfNodes, uint32_t numBins);

    void prepare() override ;
    cardStat estimate(Triple &q) override ;

    uint32_t estimateLabelCardinality(uint32_t label);

};


#endif //QS_SIMPLEESTIMATOR_H


// Basic implementation
// #ifndef QS_SIMPLEESTIMATOR_H
// #define QS_SIMPLEESTIMATOR_H

// #include "Estimator.h"
// #include "SimpleGraph.h"

// // Add pair hash function in SimpleEstimator.h
// struct pair_hash {
//     template <class T1, class T2>
//     std::size_t operator () (const std::pair<T1,T2> &p) const {
//         auto h1 = std::hash<T1>{}(p.first);
//         auto h2 = std::hash<T2>{}(p.second);
//         return h1 ^ (h2 << 1);
//     }
// };

// class SimpleEstimator : public Estimator {

//     std::shared_ptr<SimpleGraph> graph;

// public:
//     explicit SimpleEstimator(std::shared_ptr<SimpleGraph> &g);
//     ~SimpleEstimator() = default;

//     void prepare() override ;
//     cardStat estimate(Triple &q) override ;

// private:
//     uint32_t noLabels; // number of distinct labels
//     uint32_t noVertices; // number of vertices
//     uint32_t noEdges; // number of edges
//     void calculateLabelVertexEdgeCounts(); // New method to calculate label and vertex counts

//     std::unordered_map<uint32_t, uint32_t> labelCountsIn; // map to store label counts
//     std::unordered_map<uint32_t, uint32_t> labelCountsOut; // map to store label counts
//     void calculateLabelCounts(); // New method to calculate label counts
    

//     uint32_t distinctSource; // number of source nodes
//     uint32_t distinctTarget; // number of target nodes
//     void calculateSourceTargetCounts(); // New method to calculate source and target counts

//     std::unordered_map<uint32_t, uint32_t> distinctSourcesPerLabel; // map to store distinct sources per label
//     std::unordered_map<uint32_t, uint32_t> distinctTargetsPerLabel; // map to store distinct targets per label
//     void calculateSourceTargetCountsPerLabel(); // New method to calculate source and target counts per label

//     int32_t calculateSum(std::vector<uint32_t> &vec); // New method to calculate the sum of a vector

//     // Map of (inLabel, outLabel) -> count of nodes with this combination
//     std::unordered_map<std::pair<uint32_t, uint32_t>, uint32_t, pair_hash> labelTransitionCounts;
//     void calculateLabelTransitions();
// };

// #endif //QS_SIMPLEESTIMATOR_H

