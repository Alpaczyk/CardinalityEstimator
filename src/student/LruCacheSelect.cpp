//
// Created by micha on 31.01.2025.
//
//
// Created by micha on 30.01.2025.
//

#include "../include/student/LruCacheSelect.h"
#include "../include/student/NodeSelect.h"

LruCacheSelect::LruCacheSelect(const int capacity) {
    oldest = new NodeSelect(std::make_tuple(nullptr, 0, false, false), nullptr);
    latest = new NodeSelect(std::make_tuple(nullptr, 0, false, false), nullptr);
    oldest->next = latest;
    latest->prev = oldest;
    cap = capacity;
}

void LruCacheSelect::remove(const NodeSelect* node) {
    NodeSelect* prev = node->prev;
    NodeSelect* next = node->next;
    prev->next = next;
    next->prev = prev;
}
void LruCacheSelect::insert(NodeSelect* node) const {
    NodeSelect* prev = latest->prev;
    NodeSelect* next = latest;
    prev->next = next->prev = node;
    node->next = next;
    node->prev = prev;
}
std::shared_ptr<SimpleGraph> LruCacheSelect::get(std::tuple<std::shared_ptr<SimpleGraph>, uint16_t, bool, bool> &key) {
    if(cache.find(key) != cache.end()) {
        NodeSelect* node = cache[key];
        remove(node);
        insert(node);
        return node->val;
    }
    return nullptr;
}

void LruCacheSelect::put(std::tuple<std::shared_ptr<SimpleGraph>, uint16_t, bool, bool> &key, std::shared_ptr<SimpleGraph> &value) {
    if(cache.find(key) != cache.end()) {
        remove(cache[key]);
    }
    auto* node = new NodeSelect(key, value);
    cache[key] = node;
    insert(node);

    if(cache.size() > cap) {
        const NodeSelect* lru = oldest->next;
        remove(lru);
        cache.erase(lru->key);
        delete lru;
    }
}
LruCacheSelect::~LruCacheSelect() {
    NodeSelect* curr = oldest;
    while(curr != nullptr) {
        NodeSelect* next = curr->next;
        delete curr;
        curr = next;
    }
}


