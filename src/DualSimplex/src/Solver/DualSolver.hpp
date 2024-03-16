#ifndef __DUAL_SOLVER__HPP
#define __DUAL_SOLVER__HPP

#include "Tableau.hpp"
#include "TableauFilesManager.hpp"
#include <string>

class DualSolver {

public:
    DualSolver(const char *filePath);
    void startDualSimplex(void);
    ~DualSolver(void);

private:
    TableauFilesManager *tableauInOut;
    Tableau tableau;

    bool doDualCostsValidation(void);
    void doDualPivot(const Eigen::Index r, const Eigen::Index c);
};

#endif // __DUAL_SOLVER__HPP
