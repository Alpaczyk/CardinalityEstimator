#ifndef QS_SIMPLEESTIMATOR_H
#define QS_SIMPLEESTIMATOR_H

#include "Estimator.h"
#include "SimpleGraph.h"

class SimpleEstimator : public Estimator {

    std::shared_ptr<SimpleGraph> graph;
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

};


#endif //QS_SIMPLEESTIMATOR_H