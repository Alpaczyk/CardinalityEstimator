#include "SimpleGraph.h"

SimpleGraph::SimpleGraph(uint32_t n)   {
    setNoVertices(n);
}

SimpleGraph::SimpleGraph(std::shared_ptr<SimpleGraph> &other)   {
    adj = other->adj;
    reverse_adj = other->reverse_adj;

    cell_value_array = other->cell_value_array;
    column_index_array = other->column_index_array;
    row_position_array = other->row_position_array;

    V = other->getNoVertices();
    L = other->getNoLabels();

    lastSource = other->lastSource;
    highestTarget = other->highestTarget;
}

uint32_t SimpleGraph::getNoVertices() const {
    return V;
}

void SimpleGraph::setNoVertices(uint32_t n) {
    V = n;

    adj.resize(V);
    reverse_adj.resize(V);
}

uint32_t SimpleGraph::getNoEdges() const {
    return cell_value_array.size();
}

// sort on the second item in the pair, then on the first (ascending order)
bool sortPairs(const std::pair<uint32_t,uint32_t> &a, const std::pair<uint32_t,uint32_t> &b) {
    if (a.second < b.second) return true;
    if (a.second == b.second) return a.first < b.first;
    return false;
}

uint32_t SimpleGraph::getRowEndIndex(uint32_t source) const {
        if (source + 1 < row_position_array.size()) {
            return row_position_array[source + 1];
        } else {
            return column_index_array.size();
        }
    }

uint32_t SimpleGraph::getNoDistinctEdges() const {

    uint32_t sum = 0;

    for (uint32_t source = 0; source < row_position_array.size(); ++source) {
        uint32_t prevTarget = 0;
        uint32_t prevLabel = 0;
        bool first = true;

        // iterate over the edges of the source vertex
        for (uint32_t i = row_position_array[source]; i < getRowEndIndex(source); ++i) {
            if (first || !(prevTarget == column_index_array[i] && prevLabel == cell_value_array[i])) {
                first = false;
                sum++;
                prevTarget = column_index_array[i];
                prevLabel = cell_value_array[i];
            }
        }
    }

    return sum;
}

uint32_t SimpleGraph::getNoLabels() const {
    return L;
}

void SimpleGraph::setNoLabels(uint32_t noLabels) {
    L = noLabels;
}

void SimpleGraph::addEdge(uint32_t from, uint32_t to, uint32_t edgeLabel) {

    // construction of the adjacency list
    if(from >= V || to >= V || edgeLabel >= L)
        throw std::runtime_error(std::string("Edge data out of bounds: ") +
                                         "(" + std::to_string(from) + "," + std::to_string(to) + "," +
                                         std::to_string(edgeLabel) + ")");
    adj[from].emplace_back(std::make_pair(edgeLabel, to));
    reverse_adj[to].emplace_back(std::make_pair(edgeLabel, from));

    // store the edge data in the CSR format
    // for the first edge, set the row_position_array to 0
    if (cell_value_array.size() == 0) {
        // insertion of the first edge
        cell_value_array.push_back(edgeLabel);
        column_index_array.push_back(to);
        
        for (uint32_t i = 0; i <= from; ++i) {
            row_position_array.push_back(0);
        }

        lastSource = from;
        highestTarget = to;
    } else {
        // if the source vertex is the same as the last source, append the edge data
        if (from == lastSource) {
            if (to > highestTarget || (to == highestTarget && edgeLabel > cell_value_array.back())) {
                cell_value_array.push_back(edgeLabel);
                column_index_array.push_back(to);

                highestTarget = to;
            } else {
                // find the right position to insert the edge data
                uint32_t i = row_position_array[from];

                while (i < cell_value_array.size() && column_index_array[i] < to) {
                    i++;
                }

                while (i < cell_value_array.size() && column_index_array[i] == to && cell_value_array[i] < edgeLabel) {
                    i++;
                }

                // insert the edge data
                cell_value_array.insert(cell_value_array.begin() + i, edgeLabel);
                column_index_array.insert(column_index_array.begin() + i, to);
            }
        } else if (from > lastSource) {
            // if the source vertex is greater than the last source, append the row_position_array
            cell_value_array.push_back(edgeLabel);
            column_index_array.push_back(to);

            for (uint32_t i = lastSource + 1; i <= from; ++i) {
                row_position_array.push_back(cell_value_array.size() - 1); 
            }

            lastSource = from;
            highestTarget = to;
        } else {
            // if the source vertex is less than the last source, find the correct position to insert the edge data
            uint32_t i = row_position_array[from];

            while (i < row_position_array[from + 1] && column_index_array[i] < to) {
                i++;
            }

            while (i < cell_value_array.size() && column_index_array[i] == to && cell_value_array[i] < edgeLabel) {
                i++;
            }

            // insert the edge data
            cell_value_array.insert(cell_value_array.begin() + i, edgeLabel);
            column_index_array.insert(column_index_array.begin() + i, to);

            // update the row_position_array
            for (uint32_t i = from + 1; i <= lastSource; ++i) {
                row_position_array[i]++;
            }

        }
    }
}

bool SimpleGraph::edgeExists(uint32_t from, uint32_t to, uint32_t edgeLabel) {
    // adjacency list representation
    auto it = std::find(adj[from].begin(), adj[from].end(), std::make_pair(edgeLabel, to));
    return (it != adj[from].end());
    
    // CSR representation
    for (uint32_t i = row_position_array[from]; i < getRowEndIndex(from); ++i) {
        if (column_index_array[i] == to && cell_value_array[i] == edgeLabel) {
            return true;
        }
    }
    return false;
}

void SimpleGraph::readFromContiguousFile(const std::string &fileName) {
    
    std::string line;
    std::ifstream graphFile { fileName };

    std::regex edgePat (R"((\d+)\s(\d+)\s(\d+)\s\.)"); // subject predicate object .
    std::regex headerPat (R"((\d+),(\d+),(\d+))"); // noNodes,noEdges,noLabels

    // parse the header (1st line)
    std::getline(graphFile, line);
    std::smatch matches;
    if(std::regex_search(line, matches, headerPat)) {
        uint32_t noNodes = (uint32_t) std::stoul(matches[1]);
        uint32_t noLabels = (uint32_t) std::stoul(matches[3]);

        setNoVertices(noNodes);
        setNoLabels(noLabels);
    } else {
        throw std::runtime_error(std::string("Invalid graph header!"));
    }

    // parse edge data
    while(std::getline(graphFile, line)) {

        if(std::regex_search(line, matches, edgePat)) {
            uint32_t subject = (uint32_t) std::stoul(matches[1]);
            uint32_t predicate = (uint32_t) std::stoul(matches[2]);
            uint32_t object = (uint32_t) std::stoul(matches[3]);

            addEdge(subject, object, predicate);
        }
    }

    graphFile.close();

    // output first 20 elements of the adjacency list
    std::cout << "adjacency list: ";
    for (int i = 0; i < 20; i++) {
        std::cout << i << ": ";
        for (const auto &pair : adj[i]) {
            std::cout << "(" << pair.first << "," << pair.second << ") ";
        }
        std::cout << std::endl;
    }

    // output sizes of the adjacency list and CSR arrays
    std::cout << "adjacency list size: " << adj.size() << std::endl;
    
    std::cout << "row_position_array size: " << row_position_array.size() << std::endl;
    std::cout << "column_index_array size: " << column_index_array.size() << std::endl;
    std::cout << "cell_value_array size: " << cell_value_array.size() << std::endl;

    std::cout << "noVertices: " << getNoVertices() << std::endl;
    std::cout << "noEdges: " << getNoEdges() << std::endl;
    std::cout << "noDistinctEdges: " << getNoDistinctEdges() << std::endl;

    // output the first elements of each of the arrays
    std::cout << "row_position_array: ";
    for (int i = 0; i < 40; i++) {
        std::cout << row_position_array[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "column_index_array: ";
    for (int i = 0; i < 40; i++) {
        std::cout << column_index_array[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "cell_value_array: ";
    for (int i = 0; i < 40; i++) {
        std::cout << cell_value_array[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "Graph read successfully!" << std::endl;

}