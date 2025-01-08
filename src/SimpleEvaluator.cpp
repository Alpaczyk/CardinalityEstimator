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
 * A naive transitive closure (TC) computation.
 * @param in Input graph
 * @return
 */
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
    // Handle Kleene closure
    if (pe.kleene) {
        pe.kleene = false;  // Temporarily disable Kleene to evaluate the base case
        auto base = evaluateUnionKleene(pe); // Recursively evaluate the base path
        return transitiveClosure(base); // Compute transitive closure
    }

    // Handle single label case
    if (pe.labels.size() == 1) {
        const auto &labelDir = pe.labels[0];
        return selectLabel(labelDir.label, 0, labelDir.reverse, graph);
    }

    // Handle union of multiple labels
    auto combinedGraph = selectLabel(pe.labels[0].label, 0, pe.labels[0].reverse, graph);
    for (size_t i = 1; i < pe.labels.size(); ++i) {
        auto partialGraph = selectLabel(pe.labels[i].label, 0, pe.labels[i].reverse, graph);
        unionDistinct(combinedGraph, partialGraph); // Combine graphs efficiently
    }

    return combinedGraph;
}


/**
 * Given an AST, evaluate the query and produce an answer graph.
 * @param q Parsed AST.
 * @return Solution as a graph.
 */
#include <algorithm>
#include <vector>

// Updated evaluateConcat function in SimpleEvaluator
std::shared_ptr<SimpleGraph> SimpleEvaluator::evaluateConcat(std::vector<PathEntry> &path) {
    // Start with the first path entry
    auto resultGraph = evaluateUnionKleene(path[0]);

    // Perform joins incrementally for remaining path entries
    for (size_t i = 1; i < path.size(); ++i) {
        auto nextGraph = evaluateUnionKleene(path[i]);
        resultGraph = join(resultGraph, nextGraph); // Perform a join for concatenation
    }

    return resultGraph;
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

    if (s < in->adj.size() && !in->adj[s].empty()) {
        for (const auto &labelTarget : in->adj[s]) {
            out->addEdge(s, labelTarget.second, labelTarget.first);
        }
    }

    return out;
}

std::shared_ptr<SimpleGraph> selectTarget(Identifier &t, std::shared_ptr<SimpleGraph> &in) {
    auto out = std::make_shared<SimpleGraph>(in->getNoVertices());
    out->setNoLabels(in->getNoLabels());

    if (t < in->reverse_adj.size() && !in->reverse_adj[t].empty()) {
        for (const auto &labelSource : in->reverse_adj[t]) {
            out->addEdge(labelSource.second, t, labelSource.first);
        }
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