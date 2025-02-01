//
// Created by micha on 30.01.2025.
//

#include "NodeTrans.h"


NodeTrans::NodeTrans(const std::shared_ptr<SimpleGraph> &k, const std::shared_ptr<SimpleGraph> &v) {
    key = k;
    val = v;
    prev = nullptr;
    next = nullptr;
}