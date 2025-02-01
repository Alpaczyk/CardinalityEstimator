#include "SimpleEstimator.h"
#include "SimpleEvaluator.h"

#include "LruCacheJoin.h"
#include "../include/student/LruCacheSelect.h"
#include "../include/student/LruCacheTrans.h"
LruCacheSelect selectCache = LruCacheSelect(10);
LruCacheTrans transCache = LruCacheTrans(10);
LruCacheJoin joiningCache = LruCacheJoin(10);

bool caching = true;
bool joining = false;
bool trans = true;



SimpleEvaluator::SimpleEvaluator(std::shared_ptr<SimpleGraph> &g) {

    // works only with SimpleGraph
    graph = g;
    est = nullptr; // estimator not attached by default
}

void SimpleEvaluator::attachEstimator(std::shared_ptr<SimpleEstimator> &e) {
    est = e;
}

void SimpleEvaluator::prepare() {

    // if attached, prepare the estimator
    if(est != nullptr) est->prepare();

    // prepare other things here.., if necessary

}

cardStat SimpleEvaluator::computeStats(std::shared_ptr<SimpleGraph> &g) {

    cardStat stats {};
    for(int source = 0; source < g->getNoVertices(); source++) {
        if(!g->adj[source].empty()) stats.noOut++;
    }

    stats.noPaths = g->getNoDistinctEdges();

    for(int target = 0; target < g->getNoVertices(); target++) {
        if(!g->reverse_adj[target].empty()) stats.noIn++;
    }

    return stats;
}

/**
 * Select all edges from a graph with a given edge label.
 * @param projectLabel Label to select.
 * @param outLabel Label to rename the selected edge labels to (used in the TC).
 * @param inverse Follow the edges in inverse direction.
 * @param in The graph to select from.
 * @return The graph which only has edges with the specified edge label.
 */
std::shared_ptr<SimpleGraph> SimpleEvaluator::selectLabel(uint32_t projectLabel, uint32_t outLabel, bool join, bool inverse, std::shared_ptr<SimpleGraph> &in) {

    auto cacheKey = std::make_tuple(in, projectLabel,join, inverse);
    if(caching) {
        std::shared_ptr<SimpleGraph> pos = selectCache.get(cacheKey);
        if(pos != nullptr) {
            return pos;
        }
    }
    // std::cout << selectLabelCache.size() << "\n";
    auto out = std::make_shared<SimpleGraph>(in->getNoVertices());
    out->setNoLabels(in->getNoLabels());

    if(!inverse) {
        // going forward
        for(uint32_t source = 0; source < in->getNoVertices(); source++) {
            for (auto labelTarget : in->adj[source]) {

                auto label = labelTarget.first;
                auto target = labelTarget.second;

                if (label == projectLabel)
                    out->addEdge(source, target, outLabel);
            }
        }
    } else {
        // going backward
        for(uint32_t source = 0; source < in->getNoVertices(); source++) {
            for (auto labelTarget : in->reverse_adj[source]) {

                auto label = labelTarget.first;
                auto target = labelTarget.second;

                if (label == projectLabel)
                    out->addEdge(source, target, outLabel);
            }
        }
    }
    selectCache.put(cacheKey, out);
    return out;
}

/**
 * A naive transitive closure (TC) computation.
 * @param in Input graph
 * @return
 */
std::shared_ptr<SimpleGraph> SimpleEvaluator::transitiveClosure(std::shared_ptr<SimpleGraph> &base) {
    std::shared_ptr<SimpleGraph> pos = transCache.get(base);
    if(pos != nullptr) {
        return pos;
    }
    auto out = std::make_shared<SimpleGraph>(base);

    uint32_t numNewAdded = 1;
    trans = false;
    while (numNewAdded) {
        auto delta = join(out, base);
        numNewAdded = unionDistinct(out, delta);
    }
    transCache.put(base, out);
    return out;
}

/**
 * Merges a graph into another graph.
 * @param left A graph to be merged into.
 * @param right A graph to be merged from.
 * @return A number of distinct new edges added from the "right" graph into the "left" graph.
 */
uint32_t SimpleEvaluator::unionDistinct(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right) {

    uint32_t numNewAdded = 0;

    for(uint32_t source = 0; source < right->getNoVertices(); source++) {
        for (auto labelTarget : right->adj[source]) {

            auto label = labelTarget.first;
            auto target = labelTarget.second;

            if(!left->edgeExists(source, target, label)) {
                left->addEdge(source, target, label);
                numNewAdded++;
            }
        }
    }

    return numNewAdded;
}

/**
 * Simple implementation of a join of two graphs.
 * @param left A graph to be joined.
 * @param right Another graph to join with.
 * @return Answer graph for a join. Note that all labels in the answer graph are "0".
 */
std::shared_ptr<SimpleGraph> SimpleEvaluator::join(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right) {

    auto cacheKey = std::make_tuple(left, right);
    if(trans) {
        std::shared_ptr<SimpleGraph> pos = joiningCache.get(cacheKey);
        if(pos != nullptr) {
            return pos;
        }
    }
    auto out = std::make_shared<SimpleGraph>(left->getNoVertices());
    out->setNoLabels(1);

    for(uint32_t leftSource = 0; leftSource < left->getNoVertices(); leftSource++) {
        for (auto labelTarget : left->adj[leftSource]) {
            int leftTarget = labelTarget.second;
            // try to join the left target with right s
            for (auto rightLabelTarget : right->adj[leftTarget]) {

                auto rightTarget = rightLabelTarget.second;
                out->addEdge(leftSource, rightTarget, 0);
            }
        }
    }
    joiningCache.put(cacheKey, out);
    return out;
}

std::shared_ptr<SimpleGraph> SimpleEvaluator::evaluateUnionKleene(PathEntry &pe) {

    if(pe.kleene) {
        // evaluate closure
        joining = false;
        caching = true;
        pe.kleene = false;
        auto base = evaluateUnionKleene(pe);
        return transitiveClosure(base);
    } else {
        // not Kleene
        if (pe.labels.size() == 1) {
            // base label selection
            joining = false;
            caching = true;
            auto labelDir = pe.labels[0];
            return selectLabel(labelDir.label, 0, joining, labelDir.reverse, graph);
        } else {
            // (left-deep) union
            joining =true;
            caching = false;
            auto out = selectLabel(pe.labels[0].label, 0, joining, pe.labels[0].reverse, graph);
            for (int i = 1; i < pe.labels.size(); i++) {
                auto rg = selectLabel(pe.labels[i].label, 0, joining, pe.labels[i].reverse, graph);
                unionDistinct(out, rg);
            }
            return out;
        }
    }
}

/**
 * Given an AST, evaluate the query and produce an answer graph.
 * @param q Parsed AST.
 * @return Solution as a graph.
 */
std::shared_ptr<SimpleGraph> SimpleEvaluator::evaluateConcat(std::vector<PathEntry> &path) {

    auto out  = evaluateUnionKleene(path[0]);
    // evaluate according to the AST top-down
    // 1. evaluate concatenations
    if(path.size() > 1) {
        // (left-deep) join
        trans = true;
        for(int i = 1; i < path.size(); i++) {
            auto rg  = evaluateUnionKleene(path[i]);
            out = join(out, rg);
        }
    }

    return out;
}

/**
 * Perform a selection on a source constant.
 * @param s A source constant.
 * @param in A graph to select from.
 * @return An answer graph as a result of the given selection.
 */
std::shared_ptr<SimpleGraph> selectSource(Identifier &s, std::shared_ptr<SimpleGraph> &in) {

    auto out = std::make_shared<SimpleGraph>(in->getNoVertices());
    out->setNoLabels(in->getNoLabels());

    for (auto labelTarget : in->adj[s]) {

        auto label = labelTarget.first;
        auto target = labelTarget.second;

        out->addEdge(s, target, label);
    }

    return out;
}

/**
 * Perform a selection on a target constant.
 * @param s A target constant.
 * @param in A graph to select from.
 * @return An answer graph as a result of the given selection.
 */
std::shared_ptr<SimpleGraph> selectTarget(Identifier &t, std::shared_ptr<SimpleGraph> &in) {

    auto out = std::make_shared<SimpleGraph>(in->getNoVertices());
    out->setNoLabels(in->getNoLabels());

    for (auto labelSource : in->reverse_adj[t]) {

        auto label = labelSource.first;
        auto source = labelSource.second;
        out->addEdge(source, t, label);
    }

    return out;
}

/**
 * Evaluate a path query. Produce a cardinality of the answer graph.
 * @param query Query to evaluate.
 * @return A cardinality statistics of the answer graph.
 */
cardStat SimpleEvaluator::evaluate(Triple &query) {
    auto res = evaluateConcat(query.path);
    if(query.src != NO_IDENTIFIER) res = selectSource(query.src, res);
    else if(query.trg != NO_IDENTIFIER) res = selectTarget(query.trg, res);
    return SimpleEvaluator::computeStats(res);
}


//C:\Users\micha\CLionProjects\quicksilverSecond\workload\real\1\graph.nt C:\Users\micha\CLionProjects\quicksilverSecond\workload\real\1\queries.csv
//C:\Users\micha\CLionProjects\quickSilverProject\workload\syn\1\graph.nt C:\Users\micha\CLionProjects\quickSilverProject\workload\syn\1\queries.csv
//C:\Users\kamil\CLionProjects\quicksilver-main\workload\real\1\graph.nt C:\Users\kamil\CLionProjects\quicksilver-main\workload\real\1\queries.csv
//C:\Users\kamil\CLionProjects\quicksilver-main\workload\syn\1\graph.nt C:\Users\kamil\CLionProjects\quicksilver-main\workload\syn\1\queries.csv