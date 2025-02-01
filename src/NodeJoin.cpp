//
// Created by micha on 31.01.2025.
//
#include "NodeJoin.h"


NodeJoin::NodeJoin(const std::tuple<std::shared_ptr<SimpleGraph>, std::shared_ptr<SimpleGraph>> &k, const std::shared_ptr<SimpleGraph> &v) {
    key = k;
    val = v;
    prev = nullptr;
    next = nullptr;
}