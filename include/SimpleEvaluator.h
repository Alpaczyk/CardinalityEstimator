#ifndef QS_SIMPLEEVALUATOR_H
#define QS_SIMPLEEVALUATOR_H

#include <memory>
#include <cmath>
#include "SimpleGraph.h"
#include "Query.h"
#include "Evaluator.h"
#include "Graph.h"
#include <tuple>
#include <functional>

namespace std {
    // Custom hash function for std::tuple
    template <>
    struct hash<std::tuple<std::shared_ptr<SimpleGraph>, uint32_t,bool, bool>> {
        size_t operator()(const std::tuple<std::shared_ptr<SimpleGraph>, uint32_t, bool, bool>& key) const {
            auto& [graph, label,join, inverse] = key;
            size_t seed = 0;

            // Hash the shared pointer's address
            seed ^= std::hash<void*>()(graph.get()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            // Hash the label
            seed ^= std::hash<uint32_t>()(label) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            // Hash the boolean value
            seed ^= std::hash<bool>()(join) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            // Hash the boolean value
            seed ^= std::hash<bool>()(inverse) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            return seed;
        }
    };
    template <>
     struct hash<std::tuple<std::shared_ptr<SimpleGraph>, std::shared_ptr<SimpleGraph>>> {
        size_t operator()(const std::tuple<std::shared_ptr<SimpleGraph>, std::shared_ptr<SimpleGraph>>& key) const {
            auto& [graph, graph2] = key;
            size_t seed = 0;

            // Hash the shared pointer's address
            seed ^= std::hash<void*>()(graph.get()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            seed ^= std::hash<void*>()(graph2.get()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

            return seed;
        }
    };
}
class SimpleEvaluator : public Evaluator {

    std::shared_ptr<SimpleGraph> graph;
    std::shared_ptr<SimpleEstimator> est;
    static std::unordered_map<std::tuple<std::shared_ptr<SimpleGraph>, uint32_t, bool, bool>, std::shared_ptr<SimpleGraph>> selectLabelCache;
    static std::unordered_map<std::shared_ptr<SimpleGraph>, std::shared_ptr<SimpleGraph>> transitiveClosureCache;
    static std::unordered_map<std::tuple<std::shared_ptr<SimpleGraph>, std::shared_ptr<SimpleGraph>>, std::shared_ptr<SimpleGraph>> joinCache;


public:

    explicit SimpleEvaluator(std::shared_ptr<SimpleGraph> &g);
    ~SimpleEvaluator() = default;

    void prepare() override ;
    cardStat evaluate(Triple &query) override ;

    void attachEstimator(std::shared_ptr<SimpleEstimator> &e);

    std::shared_ptr<SimpleGraph> evaluateConcat(std::vector<PathEntry> &path);
    std::shared_ptr<SimpleGraph> evaluateUnionKleene(PathEntry &pe);

    static std::shared_ptr<SimpleGraph> selectLabel(uint32_t projectLabel, uint32_t outLabel,bool join, bool inverse, std::shared_ptr<SimpleGraph> &in);
    static std::shared_ptr<SimpleGraph> join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right);
    static std::shared_ptr<SimpleGraph> transitiveClosure( std::shared_ptr<SimpleGraph> &base);
    static uint32_t unionDistinct(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right);
    static void invalidateCaches(std::shared_ptr<SimpleGraph> &g);

    static cardStat computeStats(std::shared_ptr<SimpleGraph> &g);

};


#endif //QS_SIMPLEEVALUATOR_H
