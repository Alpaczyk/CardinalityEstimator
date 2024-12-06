#ifndef QS_SIMPLEESTIMATOR_H
#define QS_SIMPLEESTIMATOR_H

#include "Estimator.h"
#include "SimpleEvaluator.h"
#include "SimpleGraph.h"

class SimpleEstimator : public Estimator {

    std::shared_ptr<SimpleGraph> graph;
    std::shared_ptr<SimpleGraph> sampledGraph;
    std::shared_ptr<SimpleEvaluator> sampleEvaluator;

public:
    explicit SimpleEstimator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEstimator() = default;

    void prepare() override ;
    cardStat estimate(Triple &q) override ;

};


#endif //QS_SIMPLEESTIMATOR_H
