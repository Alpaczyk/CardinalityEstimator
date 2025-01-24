#include "SimpleGraph.h"
#include "SimpleEstimator.h"
#include "QueryParser.h"

SimpleEstimator::SimpleEstimator(std::shared_ptr<SimpleGraph> &g){

    // works only with SimpleGraph
    graph = g;
}

void SimpleEstimator::prepare() {
    
    // do your prep here

    // calculate the number of labels and vertices
    calculateLabelVertexEdgeCounts();

    // calculate the label counts
    calculateLabelCounts();

    // calculate distinct source and target counts
    calculateSourceTargetCounts();

    // calculate the distinct source and target counts per label
    calculateSourceTargetCountsPerLabel();

    // calculate the label transitions
    calculateLabelTransitions();

}

void SimpleEstimator::calculateLabelVertexEdgeCounts() {
    // calculate the number of labels and vertices
    noLabels = graph->getNoLabels();
    noVertices = graph->getNoVertices();
    noEdges = graph->getNoEdges();
}

void SimpleEstimator::calculateLabelCounts() {
    const auto& adjacencyMatrix = graph->adj;
    const auto& reverseAdjacencyMatrix = graph->reverse_adj;

    for (const auto& row : adjacencyMatrix) {
        for (const auto& edge : row) {
            uint32_t label = edge.first;
            labelCountsIn[label]++;
        }
    }

    for (const auto& row : reverseAdjacencyMatrix) {
        for (const auto& edge : row) {
            uint32_t label = edge.first;
            labelCountsOut[label]++;
        }
    }
}

void SimpleEstimator::calculateSourceTargetCounts() {
    // calculate the number of sources and targets
    const auto& adjacencyMatrix = graph->adj;
    const auto& reverseAdjacencyMatrix = graph->reverse_adj;

    // define an unordered set to store the sources and targets
    std::unordered_set<uint32_t> sources;
    std::unordered_set<uint32_t> targets;

    // for each row in the adjacency matrix
    for (const auto& row : adjacencyMatrix) {
        // for each edge in the row
        for (const auto& edge : row) {
            // get the source and target
            uint32_t source = edge.second;
            // add the source and target to the sets
            sources.insert(source);
        }
    }
    for (const auto& row : reverseAdjacencyMatrix) {
        for (const auto& edge : row) {
            uint32_t target = edge.second;
            targets.insert(target);
        }
    }

    distinctSource = sources.size();
    distinctTarget = targets.size();
}

void SimpleEstimator::calculateSourceTargetCountsPerLabel() {
    // calculate the number of sources and targets per label
    const auto& adjacencyMatrix = graph->adj;
    const auto& reverseAdjacencyMatrix = graph->reverse_adj;

    // define an unordered map to store the sources and targets per label
    std::unordered_map<uint32_t, std::unordered_set<uint32_t>> targetsPerLabel;
    std::unordered_map<uint32_t, std::unordered_set<uint32_t>> sourcesPerLabel;

    // for each row in the adjacency matrix
    for (const auto& row : adjacencyMatrix) {
        // for each edge in the row
        for (const auto& edge : row) {
            uint32_t label = edge.first;
            uint32_t target = edge.second;
            targetsPerLabel[label].insert(target);
        }
    }
    for (const auto& row : reverseAdjacencyMatrix) {
        for (const auto& edge : row) {
            uint32_t label = edge.first;
            uint32_t source = edge.second;
            sourcesPerLabel[label].insert(source);
        }
    }

    // Store counts
    for (const auto& [label, targets] : targetsPerLabel) {
        distinctTargetsPerLabel[label] = targets.size();
    }
    for (const auto& [label, sources] : sourcesPerLabel) {
        distinctSourcesPerLabel[label] = sources.size();
    }
}

void SimpleEstimator::calculateLabelTransitions() {
    const auto& adj = graph->adj;
    const auto& rev_adj = graph->reverse_adj;
    
    // For each vertex
    for (size_t vertex = 0; vertex < adj.size(); vertex++) {
        // Get its incoming and outgoing labels
        std::unordered_set<uint32_t> inLabels;
        std::unordered_set<uint32_t> outLabels;
        
        // Collect incoming labels
        for (const auto& edge : rev_adj[vertex]) {
            inLabels.insert(edge.first);
        }
        
        // Collect outgoing labels
        for (const auto& edge : adj[vertex]) {
            outLabels.insert(edge.first);
        }
        
        // Count all combinations of in/out labels
        for (uint32_t inLabel : inLabels) {
            for (uint32_t outLabel : outLabels) {
                labelTransitionCounts[{inLabel, outLabel}]++;
            }
        }
    }
}

cardStat SimpleEstimator::estimate(Triple &q) {
    uint32_t noOut = 0;
    uint32_t noPaths = 0;
    uint32_t noIn = 0;

    // parse the query
    std::string queryStr = q.toString();
    QueryParser parser(queryStr);
    std::vector<Triple> triples = parser.many();

    // for each triple in the query <- why is this necessary? there should only be one triple...
    for(auto &triple : triples) {
        Identifier src = triple.src;
        Identifier trg = triple.trg;

        // get the path from the triple
        std::vector<PathEntry> path = triple.path;

        std::vector<uint32_t> noPathsPerEntry;
        std::vector<uint32_t> noOutPerEntry;
        std::vector<uint32_t> noInPerEntry;

        // std::cout << "\nNoEntries: " << path.size() << std::endl;

        // if source or target is specified, then estimate 1 and the average degree
        if(src != NO_IDENTIFIER) {
            noOut = 1;
            noPaths = static_cast<int>(static_cast<double>(noEdges) / static_cast<double>(noVertices));
            noIn = static_cast<int>(static_cast<double>(noEdges) / static_cast<double>(noVertices));

            return cardStat {noOut, noPaths, noIn};
        } 
        if(trg != NO_IDENTIFIER) {
            noIn = 1;
            noPaths = static_cast<int>(static_cast<double>(noEdges) / static_cast<double>(noVertices));
            noOut = static_cast<int>(static_cast<double>(noEdges) / static_cast<double>(noVertices));

            return cardStat {noOut, noPaths, noIn};
        }

        // if only one entry, then we have a simple path or a simple path with Kleene star
        if(path.size() == 1) {
            // get the entry
            PathEntry entry = path[0];

            bool kleene = entry.kleene;
            std::vector<LabelDir> labels = entry.labels;

            // std::cout << "\nLabels: " << labels.size() << std::endl;

            // TODO: add support for Kleene star and multiple labels
            if (kleene) {
                for(auto &label : labels) {
                    uint32_t edgeLabel = label.label;
                    uint32_t reverse = label.reverse;

                    if (reverse == 0) {
                        noPaths += labelCountsIn[edgeLabel];
                        noOut += distinctSourcesPerLabel[edgeLabel];
                        noIn += distinctTargetsPerLabel[edgeLabel];
                    } else {
                        noPaths += labelCountsOut[edgeLabel];
                        noOut += distinctTargetsPerLabel[edgeLabel];
                        noIn += distinctSourcesPerLabel[edgeLabel];
                    }
                }
            }
            else {
                // if the label is size 1, then we have a simple path
                if(labels.size() == 1) {
                    uint32_t edgeLabel = labels[0].label;
                    uint32_t reverse = labels[0].reverse;

                    if (reverse == 0) {
                        noPaths = labelCountsIn[edgeLabel];
                        noOut = distinctSourcesPerLabel[edgeLabel];
                        noIn = distinctTargetsPerLabel[edgeLabel];
                    } else {
                        noPaths = labelCountsOut[edgeLabel];
                        noOut = distinctTargetsPerLabel[edgeLabel];
                        noIn = distinctSourcesPerLabel[edgeLabel];
                    }
                }
            }
        }
        else {
            float reductionFactor = 1.0;
            noPaths = distinctSource * distinctTarget;

            // for each path entry
            for(auto &entry : path) {
                bool kleene = entry.kleene;
                std::vector<LabelDir> labels = entry.labels;

                // std::cout << "\nKleene: " << kleene << std::endl;
                // std::cout << "\nNoLabels: " << labels.size() << std::endl;

                std::vector<uint32_t> noOutPerLabel;
                std::vector<uint32_t> noInPerLabel;

                for(auto &label : labels) {
                    uint32_t edgeLabel = label.label;
                    uint32_t reverse = label.reverse;

                    reductionFactor *= (float) distinctSourcesPerLabel[edgeLabel] / (float) distinctSource;
                    reductionFactor *= (float) distinctTargetsPerLabel[edgeLabel] / (float) distinctTarget;

                    if (reverse == 0) {
                        noOutPerLabel.push_back(distinctSourcesPerLabel[edgeLabel]);
                        noInPerLabel.push_back(distinctTargetsPerLabel[edgeLabel]);
                    } else {
                        noOutPerLabel.push_back(distinctTargetsPerLabel[edgeLabel]);
                        noInPerLabel.push_back(distinctSourcesPerLabel[edgeLabel]);
                    }

                }

                // calculate the sum of paths, out and in for the entry
                noOutPerEntry.push_back(calculateSum(noOutPerLabel));
                noInPerEntry.push_back(calculateSum(noInPerLabel));
            }

        // perform the estimation using the final reduction factor
        noPaths = noPaths * reductionFactor;

        // std::cout << "\nNoPaths:" << noPaths << std::endl;

        noOut = noOutPerEntry[0] / 2;
        noIn = noInPerEntry[noInPerEntry.size() - 1] / 2;
        }


    }

    // try to return something meaningful, else just return three 100s
    if(noOut == 0) noOut = 100;
    if(noPaths == 0) noPaths = 100;
    if(noIn == 0) noIn = 100;
    
    return cardStat {noOut, noPaths, noIn};
}

int32_t SimpleEstimator::calculateSum(std::vector<uint32_t> &vec) {
    int32_t sum = 0;
    for(auto &val : vec) {
        sum += val;
    }
    return sum;
}