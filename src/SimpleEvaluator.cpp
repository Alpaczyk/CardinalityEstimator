#include "SimpleEstimator.h"
#include "SimpleEvaluator.h"

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
std::shared_ptr<SimpleGraph> SimpleEvaluator::selectLabel(uint32_t projectLabel, uint32_t outLabel, bool inverse, std::shared_ptr<SimpleGraph> &in) {

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

std::shared_ptr<SimpleGraph> SimpleEvaluator::transitiveClosure(std::shared_ptr<SimpleGraph> &base) {

    auto out = std::make_shared<SimpleGraph>(base);

    uint32_t numNewAdded = 1;
    while (numNewAdded) {
        auto delta = join(out, base);
        numNewAdded = unionDistinct(out, delta);
    }

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

    return out;
}

std::shared_ptr<SimpleGraph> SimpleEvaluator::evaluateUnionKleene(PathEntry &pe) {

    if(pe.kleene) {
        // evaluate closure
        pe.kleene = false;
        auto base = evaluateUnionKleene(pe);
        return transitiveClosure(base);
    } else {
        // not Kleene
        if (pe.labels.size() == 1) {
            // base label selection
            auto labelDir = pe.labels[0];
            return selectLabel(labelDir.label, 0, labelDir.reverse, graph);
        } else {
            // (left-deep) union
            auto out = selectLabel(pe.labels[0].label, 0, pe.labels[0].reverse, graph);
            for (int i = 1; i < pe.labels.size(); i++) {
                auto rg = selectLabel(pe.labels[i].label, 0, pe.labels[i].reverse, graph);
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
    // Vector to store subpath, corresponding graph, and cardinality
    std::vector<std::tuple<PathEntry, std::shared_ptr<SimpleGraph>, uint32_t>> subpathGraphs;

    // Step 1: Evaluate each path entry individually and store its cardinality
    for (size_t i = 0; i < path.size(); i++) {
        auto pe = path[i];
        auto graphForPath = evaluateUnionKleene(const_cast<PathEntry&>(pe));  // Evaluate graph

        // Apply source filter to the first path entry
        if (i == 0 && s != NO_IDENTIFIER) {
            graphForPath = selectSource(s, graphForPath);
        }

        // Apply target filter to the last path entry
        if (i == path.size() - 1 && t != NO_IDENTIFIER) {
            graphForPath = selectTarget(t, graphForPath);
        }

        uint32_t cardinality = graphForPath->getNoEdges();  // Get edge count (cardinality)
        subpathGraphs.emplace_back(pe, graphForPath, cardinality);
    }

    // Step 2: Iteratively join graphs based on the lowest cardinality result of adjacent segments
    while (subpathGraphs.size() > 1) {
        size_t minIndex = 0;
        uint64_t minCost = UINT64_MAX;

        // Find the pair of adjacent subpaths with the smallest join cost
        for (size_t i = 0; i < subpathGraphs.size() - 1; i++) {
            uint32_t leftCardinality = std::get<2>(subpathGraphs[i]);
            uint32_t rightCardinality = std::get<2>(subpathGraphs[i + 1]);
            uint64_t joinCost = static_cast<uint64_t>(leftCardinality) * rightCardinality;

            if (joinCost < minCost) {
                minCost = joinCost;
                minIndex = i;  // Store the index of the left graph in the pair
            }
        }

        // Join the two graphs with the smallest cost
        auto leftGraph = std::get<1>(subpathGraphs[minIndex]);
        auto rightGraph = std::get<1>(subpathGraphs[minIndex + 1]);
        auto joinedGraph = join(leftGraph, rightGraph);

        // Calculate the new cardinality of the joined graph
        uint32_t newCardinality = joinedGraph->getNoEdges();

        // Create a new combined PathEntry
        PathEntry combinedPath;
        combinedPath.labels.insert(combinedPath.labels.end(),
                                   std::get<0>(subpathGraphs[minIndex]).labels.begin(),
                                   std::get<0>(subpathGraphs[minIndex]).labels.end());
        combinedPath.labels.insert(combinedPath.labels.end(),
                                   std::get<0>(subpathGraphs[minIndex + 1]).labels.begin(),
                                   std::get<0>(subpathGraphs[minIndex + 1]).labels.end());

        // Replace the two adjacent subpaths with the new combined graph and path entry
        subpathGraphs[minIndex] = std::make_tuple(combinedPath, joinedGraph, newCardinality);
        subpathGraphs.erase(subpathGraphs.begin() + minIndex + 1);
    }

    // Return the final joined graph
    return std::get<1>(subpathGraphs[0]);
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
