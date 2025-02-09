#ifndef QS_SIMPLEGRAPH_H
#define QS_SIMPLEGRAPH_H

#include <unordered_set>
#include <vector>
#include <iostream>
#include <regex>
#include <fstream>

#include "Graph.h"

class SimpleGraph : public Graph {
public:
    std::vector<std::vector<std::pair<uint16_t,uint16_t>>> adj;
    std::vector<std::vector<std::pair<uint16_t,uint16_t>>> reverse_adj; // vertex adjacency list
protected:
    uint16_t V;
    uint16_t L;

public:

    SimpleGraph() : V(0), L(0) {};
    SimpleGraph(std::shared_ptr<SimpleGraph> &other);
    ~SimpleGraph() = default;
    explicit SimpleGraph(uint16_t n);

    uint16_t getNoVertices() const override ;
    uint16_t getNoEdges() const override ;
    uint16_t getNoDistinctEdges() const override ;
    uint16_t getNoLabels() const override ;

    void addEdge(uint16_t from, uint16_t to, uint16_t edgeLabel) override ;
    void readFromContiguousFile(const std::string &fileName) override ;

    bool edgeExists(uint16_t from, uint16_t to, uint16_t edgeLabel);

    void setNoVertices(uint16_t n);
    void setNoLabels(uint16_t noLabels);
    
};

#endif //QS_SIMPLEGRAPH_H
