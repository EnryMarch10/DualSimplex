#ifndef __DUAL_SIMPLEX__HPP
#define __DUAL_SIMPLEX__HPP

#include "ArrayList.hpp"
#include "Tableau.hpp"
#include <complex>

class DualSimplex {

public:
    DualSimplex(const char *filePath);
    void startDualSimplex(void);
    ~DualSimplex(void);

private:
    std::string outFolderPath;
    unsigned long counter = 1;
    Tableau tableau;

    void readTableau(const char *filename);
    void printTableau(const bool isFeasible, const std::string& description, const std::string& filename);
    void printTableauFeasible(const std::string& description);
    void printTableauInfeasible(const std::string& description);
    bool validateCosts(void);
    void putInBase(const unsigned long r, const unsigned long c);
    const std::string getNextFilename(void);
};

#endif // __DUAL_SIMPLEX__HPP
