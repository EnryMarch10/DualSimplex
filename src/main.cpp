#include "utils.hpp"
#include "DualSimplex.hpp"
#include <stdlib.h>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

inline void executeDualSimplex(const char *filePath) {
    DualSimplex tableau(filePath);
    tableau.startDualSimplex();
}

inline void filesExecuteDualSimplex(const char *path) {
    if (fs::is_directory(path)) {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (fs::is_regular_file(entry.path())) {
                executeDualSimplex(entry.path().c_str());
            }
        }
    } else if (fs::is_regular_file(path)) {
        executeDualSimplex(path);
    } else {
        std::cerr << "Unexistent path " << path << " specified" << std::endl;
    }
}

int main(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        if (fs::exists(argv[i])) {
            filesExecuteDualSimplex(argv[i]);
        } else {
            filesExecuteDualSimplex((std::string(FILES_IN_PATH) + argv[i]).c_str());
        }
    }

    return EXIT_SUCCESS;
}
