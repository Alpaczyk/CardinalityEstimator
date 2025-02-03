//
// Created by micha on 31.01.2025.
//

#ifndef LRUCACHESELECT_H
#define LRUCACHESELECT_H
#include <memory>
#include <unordered_map>

#include "../SimpleGraph.h"
#include "NodeSelect.h"

// Custom hash function for std::tuple
template <>
struct std::hash<std::tuple<std::shared_ptr<SimpleGraph>, uint32_t,bool, bool>> {
    size_t operator()(const std::tuple<std::shared_ptr<SimpleGraph>, uint32_t, bool, bool>& key) const noexcept {
        auto& [graph, label,join, inverse] = key;
        size_t seed = 0;

        // Hash the shared pointer's address
        seed ^= std::hash<void*>()(graph.get()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        // Hash the label
        seed ^= std::hash<uint32_t>()(label) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        // Hash the boolean value
        seed ^= std::hash<bool>()(join) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        // Hash the boolean value
        seed ^= std::hash<bool>()(inverse) + 0x9e3779b9 + (seed << 6) + (seed >> 2);

        return seed;
    }
};

class LruCacheSelect {

    int cap;
    std::unordered_map<std::tuple<std::shared_ptr<SimpleGraph>, uint32_t, bool, bool>, NodeSelect*> cache;
    NodeSelect* oldest;
    NodeSelect* latest;

public: explicit LruCacheSelect(int capacity);
private:
    static void remove(const NodeSelect* node);
    void insert(NodeSelect* node) const;
public:
    std::shared_ptr<SimpleGraph> get(std::tuple<std::shared_ptr<SimpleGraph>, uint32_t, bool, bool> &key);
    void put(std::tuple<std::shared_ptr<SimpleGraph>, uint32_t, bool, bool> &key, std::shared_ptr<SimpleGraph> &value);
    ~LruCacheSelect();
};
#endif //LRUCACHESELECT_H
