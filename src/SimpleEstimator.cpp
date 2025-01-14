#include "SimpleGraph.h"
#include "SimpleEstimator.h"
#include "SimpleEvaluator.h"

SimpleEstimator::SimpleEstimator(std::shared_ptr<SimpleGraph> &g){

    graph = g;
}
std::unordered_map<uint32_t, std::vector<std::pair<std::pair<uint32_t, uint32_t>, uint32_t>>> SimpleEstimator::buildEquiDepthHistogram(
    const std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t>>& outVertexCount, uint32_t noOfNodes, uint32_t numBins) {

    // Result structure: {label -> [bins]}
    std::unordered_map<uint32_t, std::vector<std::pair<std::pair<uint32_t, uint32_t>, uint32_t>>> histogram;

    // Loop over each label to create the histogram
    for (const auto& labelEntry : outVertexCount) {
        uint32_t label = labelEntry.first;
        const auto& sourceMap = labelEntry.second;

        // Step 1: Sort sources by their keys (source values)
        std::vector<std::pair<uint32_t, uint32_t>> sortedSources(sourceMap.begin(), sourceMap.end());
        std::sort(sortedSources.begin(), sortedSources.end(),
                  [](const std::pair<uint32_t, uint32_t>& a, const std::pair<uint32_t, uint32_t>& b) {
                      return a.first < b.first; // Ascending order by source value
                  });

        // Step 2: Calculate total count of items
        uint32_t totalItems = 0;
        for (const auto& sourceEntry : sortedSources) {
            totalItems += sourceEntry.second;
        }

        // Step 3: Calculate target bin depth (items per bin)
        binDepth = totalItems / numBins + (totalItems % numBins != 0 ? 1 : 0);

        // Step 4: Assign sources to bins
        std::vector<std::pair<std::pair<uint32_t, uint32_t>, uint32_t>> bins;
        uint32_t currentBinCount = 0;
        uint32_t currentBinStart = 0;
        uint32_t currentBinEnd = 0;

        for (const auto& sourceEntry : sortedSources) {
            uint32_t source = sourceEntry.first;
            uint32_t count = sourceEntry.second;

            // If the bin is empty, set the start value
            if (currentBinCount == 0) {
                currentBinStart = source;
            }

            // Increment the bin's item count
            currentBinCount += count;
            currentBinEnd = source; // Update the end value for the current bin

            // If the bin exceeds the target depth, finalize the bin
            if (currentBinCount >= binDepth) {
                bins.emplace_back(std::make_pair(currentBinStart, currentBinEnd), currentBinCount);
                currentBinCount = 0; // Reset for the next bin
            }
        }

        // If there's any leftover data that doesn't fill a bin, add it as the last bin
        if (currentBinCount > 0) {
            bins.emplace_back(std::make_pair(currentBinStart, currentBinEnd), currentBinCount);
        }

        // Store the bins for this label
        histogram[label] = bins;
    }

    return histogram;
}


void SimpleEstimator::prepare() {
    uint32_t NoVertices = graph->getNoVertices();

    // Containers for counting vertex occurrences
    std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t>> outVertexCount;
    std::unordered_map<uint32_t, std::unordered_map<uint32_t, uint32_t>> inVertexCount;

    // Traverse the adjacency list to populate counts
    for (uint32_t source = 0; source < graph->adj.size(); ++source) {
        for (const auto& edge : graph->adj[source]) {
            uint32_t label = edge.first;
            uint32_t target = edge.second;

            // Increment out-degree counts
            outVertexCount[label][source]++;

            // Increment in-degree counts
            inVertexCount[label][target]++;
        }
    }

    // Number of bins for the histograms
    uint32_t numBins = 10;

    // Build histograms directly
    histogramOutDepth = buildEquiDepthHistogram(outVertexCount, NoVertices, numBins);
    histogramInDepth = buildEquiDepthHistogram(inVertexCount, NoVertices, numBins);
}

cardStat SimpleEstimator::estimate(Triple &q) {

    uint32_t noIn = 0, noPaths = 0, noOut = 0;
    if(q.src == NO_IDENTIFIER && q.trg == NO_IDENTIFIER) {
        uint32_t prevMin = 0;
        for (int i = 0; i < q.path.size(); i++) {
            for (const auto& labelDir : q.path[i].labels) { // Access the labels in each PathEntry
                uint32_t noOutLabel = 0;  // Total out count for the current label
                uint32_t noInLabel = 0;   // Total in count for the current label

                auto label = labelDir.label; // Extract the label value
                for (auto bucket : histogramOutDepth.at(label)) {
                    noOutLabel += bucket.second;
                }
                for (auto bucket : histogramInDepth.at(label)) {
                    noInLabel += bucket.second;
                }
                if (i == 0) {
                    // For the first label, initialize noOut and prevMin
                    noOut = noOutLabel;
                    prevMin = std::min(noOutLabel, noInLabel);
                    if(q.path[i].kleene) {
                        prevMin += noOutLabel;
                        noPaths += noOutLabel;
                    }
                    noPaths += prevMin;
                } else {
                    // For subsequent labels, update prevMin and accumulate into noPaths
                    prevMin = std::min(prevMin, std::min(noOutLabel, noInLabel));
                    if(q.path[i].kleene) {
                        prevMin += noOutLabel;
                        noPaths += noOutLabel;
                    }
                    noPaths += prevMin;
                }
            }
            noIn = prevMin;
        }
    }
    else if (q.src != NO_IDENTIFIER) {
        uint32_t prevMin = 0;
        for (int i = 0; i < q.path.size(); i++) {
            for (const auto& labelDir : q.path[i].labels) { // Access the labels in each PathEntry
                uint32_t noOutLabel = 0;  // Total out count for the current label
                uint32_t noInLabel = 0;   // Total in count for the current label

                uint32_t label = labelDir.label; // Extract the label value
                for (const auto& bucket : histogramOutDepth.at(label)) {
                    uint32_t bucketStart = bucket.first.first;        // Start of the range
                    uint32_t bucketEnd = bucket.first.second; // End of the range (binWidth is known)

                    // Check if q.src lies within this bucket's range
                    if (q.src >= bucketStart && q.src <= bucketEnd) {
                        noOutLabel += bucket.second;
                    }
                }
                for (const auto& bucket : histogramInDepth.at(label)) {
                    uint32_t bucketStart = bucket.first.first;        // Start of the range
                    uint32_t bucketEnd = bucket.first.second; // End of the range (binWidth is known)

                    // Check if q.src lies within this bucket's range
                    if (q.src >= bucketStart && q.src <= bucketEnd) {
                        noInLabel += bucket.second;
                    }
                }
                if (i == 0) {
                    // For the first label, initialize noOut and prevMin
                    noOut = noOutLabel;
                    prevMin = std::min(noOutLabel, noInLabel);
                    if(q.path[i].kleene) {
                        prevMin += noOutLabel;
                        noPaths += noOutLabel;
                    }
                    noPaths += prevMin;
                } else {
                    // For subsequent labels, update prevMin and accumulate into noPaths
                    prevMin = std::min(prevMin, std::min(noOutLabel, noInLabel));
                    if (q.path[i].kleene) {
                        noPaths += noOutLabel * (1 + prevMin); // Accumulate paths correctly
                    } else {
                        noPaths += prevMin;
                    }

                }
            }
            noIn = prevMin;
            }
        }

    else {
        uint32_t prevMin = 0;
        for (int i = 0; i < q.path.size(); i++) {
            for (const auto& labelDir : q.path[i].labels) { // Access the labels in each PathEntry
                uint32_t noOutLabel = 0;  // Total out count for the current label
                uint32_t noInLabel = 0;   // Total in count for the current label

                uint32_t label = labelDir.label; // Extract the label value

                // Check if it's the last label
                bool isLastLabel = (i == q.path[i].labels.size() - 1);

                for (const auto& bucket : histogramOutDepth.at(label)) {
                    uint32_t bucketStart = bucket.first.first;        // Start of the range
                    uint32_t bucketEnd = bucket.first.second; // End of the range (binWidth is known)

                    // For the last label, only sum buckets containing q.trg
                    if (!isLastLabel || (q.trg >= bucketStart && q.trg <= bucketEnd)) {
                        noOutLabel += bucket.second;
                    }
                }

                for (const auto& bucket : histogramInDepth.at(label)) {
                    uint32_t bucketStart = bucket.first.first;        // Start of the range
                    uint32_t bucketEnd = bucket.first.second; // End of the range (binWidth is known)

                    // For the last label, only sum buckets containing q.trg
                    if (!isLastLabel || (q.trg >= bucketStart && q.trg <= bucketEnd)) {
                        noInLabel += bucket.second;
                    }
                }

                if (i == 0) {
                    // For the first label, initialize noOut and prevMin
                    noOut = noOutLabel;
                    prevMin = std::min(noOutLabel, noInLabel);
                    if(q.path[i].kleene) {
                        prevMin += noOutLabel;
                        noPaths += noOutLabel;
                    }
                    noPaths += prevMin;
                } else {
                    // For subsequent labels, update prevMin and accumulate into noPaths
                    prevMin = std::min(prevMin, std::min(noOutLabel, noInLabel));
                    if(q.path[i].kleene) {
                        prevMin += noOutLabel;
                        noPaths += noOutLabel;
                    }
                    noPaths += prevMin;
                }
            }
            noIn = prevMin;
        }
    }


    return cardStat {noOut, noPaths, noIn};
}

uint32_t SimpleEstimator::estimateLabelCardinality(uint32_t label) {
    uint32_t cardinality = 0;
    if (histogramOutDepth.find(label) != histogramOutDepth.end()) {
        for (const auto &bucket : histogramOutDepth[label]) {
            cardinality += bucket.second;
        }
    }
    return cardinality;
}
