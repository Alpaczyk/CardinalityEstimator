//
// Created by micha on 30.01.2025.
//

#ifndef LRUCACHE_H
#define LRUCACHE_H
#include <memory>
#include <unordered_map>

#include "NodeTrans.h"
#include "../SimpleGraph.h"


class LruCacheTrans {

    int cap;
    std::unordered_map<std::shared_ptr<SimpleGraph>, NodeTrans*> cache;
    NodeTrans* oldest;
    NodeTrans* latest;

public: explicit LruCacheTrans(int capacity);
private:
    static void remove(const NodeTrans* node);
    void insert(NodeTrans* node) const;
public:
    std::shared_ptr<SimpleGraph> get(const std::shared_ptr<SimpleGraph> &key);
    void put(const std::shared_ptr<SimpleGraph> &key, const std::shared_ptr<SimpleGraph> &value);
    ~LruCacheTrans();
};



#endif //LRUCACHE_H
