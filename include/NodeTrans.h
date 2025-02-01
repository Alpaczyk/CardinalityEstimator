//
// Created by micha on 30.01.2025.
//

#ifndef NODE_H
#define NODE_H
#include <SimpleGraph.h>


class NodeTrans {
public:
    std::shared_ptr<SimpleGraph> key;
    std::shared_ptr<SimpleGraph> val;
    NodeTrans* prev;
    NodeTrans* next;
    NodeTrans(const std::shared_ptr<SimpleGraph> &k, const std::shared_ptr<SimpleGraph> &v);
};



#endif //NODE_H
