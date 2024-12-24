#include "SimpleGraph.h"

SimpleGraph::SimpleGraph(uint32_t n)   {
    setNoVertices(n);
}

SimpleGraph::SimpleGraph(std::shared_ptr<SimpleGraph> &other)   {
    cell_value_array = other->cell_value_array;
    column_index_array = other->column_index_array;
    row_position_array = other->row_position_array;

    V = other->getNoVertices();
    L = other->getNoLabels();
}

uint32_t SimpleGraph::getNoVertices() const {
    return V;
}

void SimpleGraph::setNoVertices(uint32_t n) {
    V = n;
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

uint32_t SimpleGraph::getNoDistinctEdges() const {
    uint32_t sum = 0;

    for (uint32_t i = 0; i < V; ++i) { // Iterate over all rows
        std::vector<std::pair<uint32_t, uint32_t>> row; 

        // Extract the row from CSR representation
        for (uint32_t j = row_position_array[i]; j < row_position_array[i + 1]; ++j) {
            row.emplace_back(cell_value_array[j], column_index_array[j]); 
        }

        uint32_t prevTarget = 0;
        uint32_t prevLabel = 0;
        bool first = true;

        // Count distinct edges in the row
        for (const auto &labelTgtPair : row) {
            if (first || !(prevTarget == labelTgtPair.second && prevLabel == labelTgtPair.first)) {
                first = false;
                sum++;
                prevTarget = labelTgtPair.second;
                prevLabel = labelTgtPair.first;
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
    if(from >= V || to >= V || edgeLabel >= L)
        throw std::runtime_error(std::string("Edge data out of bounds: ") +
                                         "(" + std::to_string(from) + "," + std::to_string(to) + "," +
                                         std::to_string(edgeLabel) + ")");

    // store the edge data in the CSR format
    cell_value_array.push_back(edgeLabel);
    column_index_array.push_back(to);

    // for the first edge, set the row_position_array to 0
    if (cell_value_array.size() == 1) {
        row_position_array.push_back(0);
        lastSource = from;
    }

    if (from > lastSource) {
        for (uint32_t i = lastSource + 1; i <= from; ++i) {
            row_position_array.push_back(cell_value_array.size() - 1); 
        }
        lastSource = from;
    }
}

bool SimpleGraph::edgeExists(uint32_t from, uint32_t to, uint32_t edgeLabel) {
    for (uint32_t i = row_position_array[from]; i < row_position_array[from + 1]; ++i) {
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

    // output the first 10 elements of each of the arrays
    std::cout << "cell_value_array: ";
    for (int i = 0; i < 20; i++) {
        std::cout << cell_value_array[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "column_index_array: ";
    for (int i = 0; i < 20; i++) {
        std::cout << column_index_array[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "row_position_array: ";
    for (int i = 0; i < 20; i++) {
        std::cout << row_position_array[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "Graph read successfully!" << std::endl;

}