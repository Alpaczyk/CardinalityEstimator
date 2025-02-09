#include "SimpleEstimator.h"
#include "SimpleEvaluator.h"

#include "../include/student/LruCacheJoin.h"
#include "../include/student/LruCacheSelect.h"
#include "../include/student/LruCacheTrans.h"
LruCacheSelect selectCache = LruCacheSelect(4);
LruCacheTrans transCache = LruCacheTrans(4);
LruCacheJoin joiningCache = LruCacheJoin(4);

bool caching = true;
bool joining = false;
bool trans = true;
// int counterJoin = 0;
// int counterSelect = 0;
// int counterTrans = 0;



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
std::shared_ptr<SimpleGraph> SimpleEvaluator::selectLabel(uint16_t projectLabel, uint16_t outLabel, bool join, bool inverse, std::shared_ptr<SimpleGraph> &in) {

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
        for(uint16_t source = 0; source < in->getNoVertices(); source++) {
            for (auto labelTarget : in->adj[source]) {

                auto label = labelTarget.first;
                auto target = labelTarget.second;

                if (label == projectLabel)
                    out->addEdge(source, target, outLabel);
            }
        }
    } else {
        // going backward
        for(uint16_t source = 0; source < in->getNoVertices(); source++) {
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

    uint16_t numNewAdded = 1;
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
uint16_t SimpleEvaluator::unionDistinct(std::shared_ptr<SimpleGraph> &left, std::shared_ptr<SimpleGraph> &right) {

    uint16_t numNewAdded = 0;

    for(uint16_t source = 0; source < right->getNoVertices(); source++) {
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

    for(uint16_t leftSource = 0; leftSource < left->getNoVertices(); leftSource++) {
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
std::shared_ptr<SimpleGraph> SimpleEvaluator::evaluateConcat(Identifier &s, std::vector<PathEntry> &path, Identifier &t) {
    size_t numSegments = path.size();

    std::cout << std::endl;

    // Case 1: Only one segment, return directly
    if (numSegments == 1) {
        return evaluateUnionKleene(path[0]);
    }

    // Case 2: Exactly two segments, join immediately
    if (numSegments == 2) {
        auto left = evaluateUnionKleene(path[0]);
        auto right = evaluateUnionKleene(path[1]);
        return join(left, right);
    }

    // Case 3: More than two segments, use full cardinality estimation
    std::vector<uint64_t> estimatedCardinalities; // Stores estimated cardinalities of path segments
    std::vector<std::shared_ptr<SimpleGraph>> partialResults; // Store intermediate results of path segments
    std::vector<std::vector<PathEntry>> remainingPaths; // Store sequences of path segments

    // Step 1: Compute estimated cardinalities for individual path segments
    if (est != nullptr) {
        for (int i = 0; i < path.size(); i++) {
            Triple q;
            q.src = (i == 0) ? s : NO_IDENTIFIER;
            q.trg = (i == path.size() - 1) ? t : NO_IDENTIFIER;

            if (i == 0 && s != NO_IDENTIFIER) { // If source is specified, use it
                q.src = s;
            }
            if (i == path.size() - 1 && t != NO_IDENTIFIER) { // If target is specified, use it
                q.trg = t;
            }

            q.path = {path[i]};
            cardStat estimate = est->estimate(q);

            estimatedCardinalities.push_back(estimate.noPaths); // Store estimated cardinality
            partialResults.push_back(evaluateUnionKleene(path[i])); // Store partial result

            remainingPaths.push_back({path[i]}); // Store path segment separately
        }
    }

    // Step 2: Iteratively join the adjacent segments with the lowest cost
    while (partialResults.size() > 1) {
        size_t bestPairIndex = 0;
        uint64_t bestCost = UINT64_MAX;

        for (size_t i = 0; i < estimatedCardinalities.size() - 1; i++) {
            uint64_t joinCost = estimatedCardinalities[i] * estimatedCardinalities[i + 1];
            if (joinCost < bestCost) {
                bestCost = joinCost;
                bestPairIndex = i;
            }
        }

        // Join the best pair
        auto joinedGraph = join(partialResults[bestPairIndex], partialResults[bestPairIndex + 1]);

        // Step 3: Recalculate cardinality of the new joined segment
        uint64_t newCardinality = joinedGraph->getNoEdges();
    
        // Update estimated cardinalities and partial results
        estimatedCardinalities[bestPairIndex] = newCardinality;
        partialResults[bestPairIndex] = joinedGraph;

        // Merge paths by simply appending the right segment to the left one
        remainingPaths[bestPairIndex].insert(
            remainingPaths[bestPairIndex].end(),
            remainingPaths[bestPairIndex + 1].begin(),
            remainingPaths[bestPairIndex + 1].end()
        );

        // Remove the second element from lists
        estimatedCardinalities.erase(estimatedCardinalities.begin() + bestPairIndex + 1);
        partialResults.erase(partialResults.begin() + bestPairIndex + 1);
        remainingPaths.erase(remainingPaths.begin() + bestPairIndex + 1);
    }

    return partialResults[0];
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
    auto res = evaluateConcat(query.src, query.path, query.trg);
    if(query.src != NO_IDENTIFIER) res = selectSource(query.src, res);
    else if(query.trg != NO_IDENTIFIER) res = selectTarget(query.trg, res);
    return SimpleEvaluator::computeStats(res);
}


//C:\Users\micha\CLionProjects\quicksilverSecond\workload\real\1\graph.nt C:\Users\micha\CLionProjects\quicksilverSecond\workload\real\1\queries.csv
//C:\Users\micha\CLionProjects\quickSilverProject\workload\syn\1\graph.nt C:\Users\micha\CLionProjects\quickSilverProject\workload\syn\1\queries.csv
//C:\Users\kamil\CLionProjects\quicksilver-main\workload\real\1\graph.nt C:\Users\kamil\CLionProjects\quicksilver-main\workload\real\1\queries.csv
//C:\Users\kamil\CLionProjects\quicksilver-main\workload\syn\1\graph.nt C:\Users\kamil\CLionProjects\quicksilver-main\workload\syn\1\queries.csv