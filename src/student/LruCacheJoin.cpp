//
// Created by micha on 31.01.2025.
//
//
// Created by micha on 30.01.2025.
//

// #include "LruCacheJoin.h"
#include "../include/student/LruCacheJoin.h"

LruCacheJoin::LruCacheJoin(const int capacity) {
    oldest = new NodeJoin(std::make_tuple(nullptr, nullptr), nullptr);
    latest = new NodeJoin(std::make_tuple(nullptr, nullptr), nullptr);
    oldest->next = latest;
    latest->prev = oldest;
    cap = capacity;
}

void LruCacheJoin::remove(const NodeJoin* node) {
    NodeJoin* prev = node->prev;
    NodeJoin* next = node->next;
    prev->next = next;
    next->prev = prev;
}
void LruCacheJoin::insert(NodeJoin* node) const {
    NodeJoin* prev = latest->prev;
    NodeJoin* next = latest;
    prev->next = next->prev = node;
    node->next = next;
    node->prev = prev;
}
std::shared_ptr<SimpleGraph> LruCacheJoin::get(std::tuple<std::shared_ptr<SimpleGraph>, std::shared_ptr<SimpleGraph>> &key) {
    if(cache.find(key) != cache.end()) {
        NodeJoin* node = cache[key];
        remove(node);
        insert(node);
        return node->val;
    }
    return nullptr;
}

void LruCacheJoin::put(std::tuple<std::shared_ptr<SimpleGraph>, std::shared_ptr<SimpleGraph>> &key, std::shared_ptr<SimpleGraph> &value) {
    if(cache.find(key) != cache.end()) {
        remove(cache[key]);
    }
    auto* node = new NodeJoin(key, value);
    cache[key] = node;
    insert(node);

    if(cache.size() > cap) {
        const NodeJoin* lru = oldest->next;
        remove(lru);
        cache.erase(lru->key);
        delete lru;
    }
}
LruCacheJoin::~LruCacheJoin() {
    NodeJoin* curr = oldest;
    while(curr != nullptr) {
        NodeJoin* next = curr->next;
        delete curr;
        curr = next;
    }
}


