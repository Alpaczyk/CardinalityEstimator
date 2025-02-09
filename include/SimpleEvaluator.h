#ifndef QS_SIMPLEEVALUATOR_H
#define QS_SIMPLEEVALUATOR_H

#include <memory>
#include <cmath>
#include "SimpleGraph.h"
#include "Query.h"
#include "Evaluator.h"
#include "Graph.h"



class SimpleEvaluator : public Evaluator {

    std::shared_ptr<SimpleGraph> graph;
    std::shared_ptr<SimpleEstimator> est;
    static std::unordered_map<std::tuple<std::shared_ptr<SimpleGraph>, uint16_t, bool, bool>, std::shared_ptr<SimpleGraph>> selectLabelCache;
    static std::unordered_map<std::shared_ptr<SimpleGraph>, std::shared_ptr<SimpleGraph>> transitiveClosureCache;
    static std::unordered_map<std::tuple<std::shared_ptr<SimpleGraph>, std::shared_ptr<SimpleGraph>, bool>, std::shared_ptr<SimpleGraph>> joinCache;


public:

    explicit SimpleEvaluator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEvaluator() = default;

    void prepare() override ;
    cardStat evaluate(Triple &query) override ;

    void attachEstimator(std::shared_ptr<SimpleEstimator> &e);

    std::shared_ptr<SimpleGraph> evaluateConcat(Identifier &s, std::vector<PathEntry> &path, Identifier &t);
    std::shared_ptr<SimpleGraph> evaluateUnionKleene(PathEntry &pe);
    static std::shared_ptr<SimpleGraph> crank(std::shared_ptr<SimpleGraph> &delta, std::shared_ptr<SimpleGraph> &cumulativeTC);
    static std::shared_ptr<SimpleGraph> reduce(std::shared_ptr<SimpleGraph> &newDelta, std::shared_ptr<SimpleGraph> &cumulativeTC);

    static std::shared_ptr<SimpleGraph> selectLabel(uint16_t projectLabel, uint16_t outLabel, bool join, bool inverse, std::shared_ptr<SimpleGraph> &in);
    static std::shared_ptr<SimpleGraph> join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right);
    static std::shared_ptr<SimpleGraph> transitiveClosure( std::shared_ptr<SimpleGraph> &base);
    static uint16_t unionDistinct(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right);


    static cardStat computeStats(std::shared_ptr<SimpleGraph> &g);

};


#endif //QS_SIMPLEEVALUATOR_H
