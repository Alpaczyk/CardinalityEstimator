//
// Created by micha on 31.01.2025.
//

#ifndef NODESELECT_H
#define NODESELECT_H
#include "../SimpleGraph.h"


class NodeSelect {
public:
    std::tuple<std::shared_ptr<SimpleGraph>, uint32_t, bool, bool> key;
    std::shared_ptr<SimpleGraph> val;
    NodeSelect* prev;
    NodeSelect* next;
    NodeSelect(const std::tuple<std::shared_ptr<SimpleGraph>, uint32_t, bool, bool> &k, const std::shared_ptr<SimpleGraph> &v);
};
#endif //NODESELECT_H
