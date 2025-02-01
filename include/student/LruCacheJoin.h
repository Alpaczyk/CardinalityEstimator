//
// Created by micha on 31.01.2025.
//

#ifndef LRUCACHEJOIN_H
#define LRUCACHEJOIN_H
#include <memory>

#include "NodeJoin.h"
#include "SimpleGraph.h"

template <>
struct std::hash<std::tuple<std::shared_ptr<SimpleGraph>, std::shared_ptr<SimpleGraph>>> {
    size_t operator()(const std::tuple<std::shared_ptr<SimpleGraph>, std::shared_ptr<SimpleGraph>>& key) const noexcept {
        auto& [graph, graph2] = key;
        size_t seed = 0;

        // Hash the shared pointer's address
        seed ^= std::hash<void*>()(graph.get()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

        seed ^= std::hash<void*>()(graph2.get()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};
class LruCacheJoin {

    int cap;
    std::unordered_map<std::tuple<std::shared_ptr<SimpleGraph>, std::shared_ptr<SimpleGraph>>, NodeJoin*> cache;
    NodeJoin* oldest;
    NodeJoin* latest;

public: explicit LruCacheJoin(int capacity);
private:
    static void remove(const NodeJoin* node);
    void insert(NodeJoin* node) const;
public:
    std::shared_ptr<SimpleGraph> get(std::tuple<std::shared_ptr<SimpleGraph>, std::shared_ptr<SimpleGraph>> &key);
    void put(std::tuple<std::shared_ptr<SimpleGraph>, std::shared_ptr<SimpleGraph>> &key, std::shared_ptr<SimpleGraph> &value);
    ~LruCacheJoin();
};
#endif //LRUCACHEJOIN_H
