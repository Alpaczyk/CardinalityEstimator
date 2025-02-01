//
// Created by micha on 30.01.2025.
//

#include "../include/student/LruCacheTrans.h"


LruCacheTrans::LruCacheTrans(const int capacity) {
    oldest = new NodeTrans(nullptr, nullptr);
    latest = new NodeTrans(nullptr, nullptr);
    oldest -> next = latest;
    latest -> prev = oldest;
    cap = capacity;
}
void LruCacheTrans::remove(const NodeTrans* node) {
    NodeTrans* prev = node->prev;
    NodeTrans* next = node->next;
    prev->next = next;
    next->prev = prev;
}
void LruCacheTrans::insert(NodeTrans* node) const {
    NodeTrans* prev = latest->prev;
    NodeTrans* next = latest;
    prev->next = next->prev = node;
    node->next = next;
    node->prev = prev;
}
std::shared_ptr<SimpleGraph> LruCacheTrans::get(const std::shared_ptr<SimpleGraph> &key) {
    if(cache.find(key) != cache.end()) {
        NodeTrans* node = cache[key];
        remove(node);
        insert(node);
        return node->val;
    }
    return nullptr;
}

void LruCacheTrans::put(const std::shared_ptr<SimpleGraph> &key, const std::shared_ptr<SimpleGraph> &value) {
    if(cache.find(key) != cache.end()) {
        remove(cache[key]);
    }
    auto* node = new NodeTrans(key, value);
    cache[key] = node;
    insert(node);

    if(cache.size() > cap) {
        const NodeTrans* lru = oldest->next;
        remove(lru);
        cache.erase(lru->key);
        delete lru;
    }
}
LruCacheTrans::~LruCacheTrans() {
    const NodeTrans* curr = oldest;
    while(curr != nullptr) {
        const NodeTrans* next = curr->next;
        delete curr;
        curr = next;
    }
}


