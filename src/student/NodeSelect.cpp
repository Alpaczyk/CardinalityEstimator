//
// Created by micha on 31.01.2025.
//

#include "../include/student/NodeSelect.h"


NodeSelect::NodeSelect(const std::tuple<std::shared_ptr<SimpleGraph>, uint32_t, bool, bool> &k, const std::shared_ptr<SimpleGraph> &v) {
    key = k;
    val = v;
    prev = nullptr;
    next = nullptr;
}
