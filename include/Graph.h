#ifndef QS_GRAPH_H
#define QS_GRAPH_H

#include <unordered_map>

class Graph {

public:

    virtual uint32_t getNoVertices() const = 0;
    virtual uint32_t getNoEdges() const = 0;
    virtual uint32_t getNoDistinctEdges() const = 0;
    virtual uint32_t getNoLabels() const = 0;

    virtual void addEdge(uint16_t from, uint16_t to, uint8_t edgeLabel) = 0;
    virtual void readFromContiguousFile(const std::string &fileName) = 0;

};


#endif //QS_GRAPH_H
