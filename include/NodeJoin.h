//
// Created by micha on 31.01.2025.
//

#ifndef NODEJOIN_H
#define NODEJOIN_H
#include <memory>

#include "SimpleGraph.h"

class NodeJoin {
public:
    std::tuple<std::shared_ptr<SimpleGraph>, std::shared_ptr<SimpleGraph>> key;
    std::shared_ptr<SimpleGraph> val;

    NodeJoin* prev;
    NodeJoin* next;
    NodeJoin(const std::tuple<std::shared_ptr<SimpleGraph>, std::shared_ptr<SimpleGraph>> &k, const std::shared_ptr<SimpleGraph> &v);
};
#endif //NODEJOIN_H
