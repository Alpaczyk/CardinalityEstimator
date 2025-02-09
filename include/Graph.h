#ifndef QS_GRAPH_H
#define QS_GRAPH_H

#include <unordered_map>
#include <cstdint>

class Graph {

public:

    virtual uint16_t getNoVertices() const = 0;
    virtual uint16_t getNoEdges() const = 0;
    virtual uint16_t getNoDistinctEdges() const = 0;
    virtual uint16_t getNoLabels() const = 0;

    virtual void addEdge(uint16_t from, uint16_t to, uint16_t edgeLabel) = 0;
    virtual void readFromContiguousFile(const std::string &fileName) = 0;

};


#endif //QS_GRAPH_H
