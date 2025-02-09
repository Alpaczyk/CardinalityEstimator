#ifndef QS_SIMPLEESTIMATOR_H
#define QS_SIMPLEESTIMATOR_H

#include "Estimator.h"
#include "SimpleGraph.h"

class SimpleEstimator : public Estimator {

    std::shared_ptr<SimpleGraph> graph;
    std::shared_ptr<SimpleGraph> sampledGraph;
    std::unordered_map<unsigned, std::vector<std::pair<unsigned, unsigned>>> histogramOutVertex;
    std::unordered_map<unsigned, std::vector<std::pair<unsigned, unsigned>>> histogramInVertex;
    std::unordered_map<uint16_t, std::vector<std::pair<std::pair<uint16_t, uint16_t>, uint16_t>>> histogramOutDepth;
    std::unordered_map<uint16_t, std::vector<std::pair<std::pair<uint16_t, uint16_t>, uint16_t>>> histogramInDepth;
    uint16_t binDepth{};

public:
    explicit SimpleEstimator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEstimator() = default;

    std::unordered_map<uint16_t, std::vector<std::pair<std::pair<uint16_t, uint16_t>, uint16_t>>> buildEquiDepthHistogram(
    const std::unordered_map<uint16_t, std::unordered_map<uint16_t, uint16_t>>& outVertexCount, uint16_t noOfNodes, uint16_t numBins);

    void prepare() override ;
    cardStat estimate(Triple &q) override ;

    uint16_t estimateLabelCardinality(uint16_t label);

};


#endif //QS_SIMPLEESTIMATOR_H
