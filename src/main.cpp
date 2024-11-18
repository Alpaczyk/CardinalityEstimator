#include <Bench.h>
#include <iostream>
#include <rss.h>

int main(int argc, char *argv[]) {

    if(argc < 3) {
        std::cout << "Usage: quicksilver <graphFile> <queriesFile>" << std::endl;
        return 0;
    }

    // args
    std::string graphFile {argv[1]};
    std::string queriesFile {argv[2]};

    // uncomment for part 1
    estimatorBench(graphFile, queriesFile);
    
    // uncomment for part 2
    // evaluatorBench(graphFile, queriesFile);


    return 0;
}



