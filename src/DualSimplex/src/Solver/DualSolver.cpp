#include "DualSolver.hpp"
#include "utils.hpp"
#include <assert.h>
#include <stdlib.h>
#include <limits>

using namespace std;

DualSolver::DualSolver(const char *filePath) {
    tableauInOut = new TableauFilesManager(&tableau, filePath);
    tableauInOut->readTableau();

    Eigen::VectorXd knownTermsRealPart = tableau.knownTermsCol.real();
    Eigen::MatrixXd equationsMatrix(tableau.coeffMatrix.rows(), tableau.coeffMatrix.cols() + 1);
    equationsMatrix.col(0) = knownTermsRealPart;
    equationsMatrix.block(0, 1, tableau.coeffMatrix.rows(), tableau.coeffMatrix.cols()) = tableau.coeffMatrix;

    vector<Eigen::Index> indicesRows = getDependentRows(equationsMatrix);

    if (indicesRows.size() > 0) {
        Eigen::MatrixXd newCoeffMatrix(tableau.coeffMatrix.rows() - indicesRows.size(), tableau.coeffMatrix.cols());
        Eigen::ArrayXcd newKnownTermsCol(tableau.knownTermsCol.size() - indicesRows.size());
        Eigen::ArrayXindex newVarInBaseCol(tableau.varInBaseCol.size() - indicesRows.size());
        Eigen::Index resRowIdx = 0;

        for (Eigen::Index i = 0; i < tableau.coeffMatrix.rows(); i++) {
            // Check if the current row index should be deleted
            if (find(indicesRows.begin(), indicesRows.end(), i) != indicesRows.end()) {
                continue; // Skip this row
            }

            // Copy the current row to the newCoeffMatrix matrix
            newCoeffMatrix.row(resRowIdx) = tableau.coeffMatrix.row(i);
            newKnownTermsCol[resRowIdx] = tableau.knownTermsCol[i];
            newVarInBaseCol[i] = tableau.varInBaseCol[i];
            resRowIdx++;
        }

        tableau.coeffMatrix = newCoeffMatrix;
        tableau.knownTermsCol = newKnownTermsCol;
        tableau.varInBaseCol = newVarInBaseCol;

        assert(tableau.isFeasible());
        tableauInOut->printTableau("Deleted linearly dependent rows.", "MAX RANK");
    }
    assert(tableau.isValid());
}

void DualSolver::startDualSimplex(void) {
    bool notBasic = false;

    for (Eigen::Index i = 0; i < tableau.getM(); i++) {
        if (tableau.varInBaseCol[i] == 0) {
            for (Eigen::Index j = 0; j < tableau.getN(); j++) {
                if (isNotZero(tableau.coeffMatrix(i, j))) {
                    doDualPivot(i, j);
                    break;
                }
            }
            notBasic = true;
        }
    }

    if (notBasic) {
        tableauInOut->printTableau("Created basic solution.", "BASE CREATED");
    }

    bool invalidCosts = !doDualCostsValidation();
    if (invalidCosts) {
        tableauInOut->printTableau("Added artificial variable so as to make base dual feasible now (positive costs).", "COSTS POSITIVE");
    }

    bool found;
    do {
        Eigen::dcomplex mostNegativeKnownTerm = Eigen::dcomplex();
        Eigen::Index row;
        Eigen::Index varIndex;
        found = false;

        for (Eigen::Index i = 0; i < tableau.getM(); i++) {
            // Here exact <>= 0.0 checks wouldn't be correct because knownTerms column could have this very small values
            if (isLessThanZero(tableau.knownTermsCol[i].imag())
                    || (isZero(tableau.knownTermsCol[i].imag())
                        && isLessThanZero(tableau.knownTermsCol[i].real()))) {
                if (!found) {
                    mostNegativeKnownTerm = tableau.knownTermsCol[i];
                    row = i;
                    varIndex = tableau.varInBaseCol[i];
                    found = true;
                // This is the application of the Bland Dual rule
                } else if (tableau.varInBaseCol[i] < varIndex) {
                    mostNegativeKnownTerm = tableau.knownTermsCol[i];
                    row = i;
                    varIndex = tableau.varInBaseCol[i];
                }
            }
        }

        if (found) {
            double min = numeric_limits<double>::infinity();
            Eigen::Index col;

            for (Eigen::Index j = 0; j < tableau.getN(); j++) {
                if (isLessThanZero(tableau.coeffMatrix(row, j))) {
                    const double tmp = tableau.redCostsRow[j] / abs(tableau.coeffMatrix(row, j));
                    if (tmp < min) {
                        min = tmp;
                        col = j;
                    }
                }
            }

            if (min != numeric_limits<double>::infinity()) {
#ifdef PRINT_ITERATIONS
                stringstream ss;
                ss << "Pivot in (x" << tableau.varInBaseCol[row] << ", x" << col + 1 << ")";
                doDualPivot(row, col);
                ss << " => (x" << tableau.varInBaseCol[row] << ", x" << col + 1 << ").";
                const string summary = ss.str();
                tableauInOut->printTableau(summary, "ITERATION");
#else
                doDualPivot(row, col);
#endif
            } else {
                tableauInOut->printTableau("Primal infeasible (Dual unbounded).", "SOLUTION");
                return;
            }
        }
    } while (found);

    if (invalidCosts && complexIsZero(tableau.knownTermsCol[tableau.knownTermsCol.size() - 1])) {
        // Here I check costs with exactly 0.0, because I always set costs values to 0.0 in case of pivoting, so it's correct
        if (tableau.redCostsRow[tableau.getN() - 1] != 0.0) { // This means > 0, but here >= 0 is assumed for reduced costs 
            stringstream ss;
            ss << "Primal unbounded (in Dual solution artificial variable x" << tableau.getN() << " is not in base).";
            const string summary = ss.str();
            tableauInOut->printTableau(summary, "SOLUTION");
            return;
        }
        // Otherwise it cannot be `tableau.redCostsRow[tableau.getN() - 1] < 0.0`, so it
        // is not necessary to verify this case because of dual conditions
        // so in this case: `tableau.redCostsRow[tableau.getN() - 1] == 0.0`
        // that is considered optimal though not basic to the original problem.
    }

    // Here 0.0 check is with tolerance because of possible calculation problems
    if (isZero(tableau.objFunc.imag())) {
        tableauInOut->printTableau("Optimal solution reached (Primal solution = Dual solution).", "SOLUTION");
    } else {
        tableauInOut->printTableau("Primal infeasible (Dual unbounded).", "SOLUTION");
    }
}

bool DualSolver::doDualCostsValidation(void) {
    Eigen::Index index;

    bool found = false;
    double min = 0.0;
    for (Eigen::Index i = 0; i < tableau.redCostsRow.size(); i++) {
        if (tableau.redCostsRow[i] < min) {
            min = tableau.redCostsRow[i];
            index = i;
            found = true;
        }
    }

    if (found) {
        Eigen::Index size1 = tableau.coeffMatrix.rows();
        Eigen::Index size2 = tableau.coeffMatrix.cols();

        tableau.coeffMatrix.conservativeResize(size1 + 1, size2 + 1);
        tableau.coeffMatrix.row(size1).setZero();
        tableau.coeffMatrix.col(size2).setZero();

        size1 = tableau.redCostsRow.size();
        tableau.redCostsRow.conservativeResize(size1 + 1);
        tableau.redCostsRow[size1] = 0.0;

        size1 = tableau.knownTermsCol.size();
        tableau.knownTermsCol.conservativeResize(size1 + 1);
        tableau.knownTermsCol[size1] = Eigen::dcomplex(0.0, 1.0);

        size1 = tableau.coeffMatrix.rows() - 1;
        size2 = tableau.coeffMatrix.cols() - 1;
        for (Eigen::Index j = 0; j < size2; j++) {
            // Here exact 0.0 check is valid because every cost must be positive
            if (tableau.redCostsRow[j] != 0.0) {
                tableau.coeffMatrix(size1, j) = 1.0;
            }
        }
        tableau.coeffMatrix(size1, size2) = 1.0;

        size1 = tableau.varInBaseCol.size();
        tableau.varInBaseCol.conservativeResize(size1 + 1);
        tableau.varInBaseCol[size1] = tableau.getN() + 1;

        doDualPivot(tableau.coeffMatrix.rows() - 1, index);
        return false;
    }

    return true;
}

void DualSolver::doDualPivot(const Eigen::Index r, const Eigen::Index c) {
    // Here 0.0 check is correct because this assertion is more theoretical than practical
    assert(tableau.coeffMatrix(r, c) != 0.0);

    // Adapts target row
    if (tableau.coeffMatrix(r, c) != 1.0) {
        tableau.knownTermsCol[r] /= tableau.coeffMatrix(r, c);
        tableau.coeffMatrix.row(r) /= tableau.coeffMatrix(r, c);
        tableau.coeffMatrix(r, c) = 1.0;
    }

    // Updates reduced costs row
    if (tableau.redCostsRow[c] != 0.0) {
        tableau.objFunc += -tableau.redCostsRow[c] * tableau.knownTermsCol[r];
        tableau.redCostsRow += -tableau.redCostsRow[c] * tableau.coeffMatrix.row(r).array();
        tableau.redCostsRow[c] = 0.0;
    }

    // Updates every other row
    for (Eigen::Index i = 0; i < tableau.coeffMatrix.rows(); i++) {
        // Here approximate != 0.0 check may cause problems
        // Probably, not sure about this, because certain rows aren't updated correctly
        // Verified in practice
        if ((i != r) && (tableau.coeffMatrix(i, c) != 0.0)) {
            tableau.knownTermsCol[i] += -tableau.coeffMatrix(i, c) * tableau.knownTermsCol[r];
            tableau.coeffMatrix.row(i) += -tableau.coeffMatrix(i, c) * tableau.coeffMatrix.row(r);
            tableau.coeffMatrix(i, c) = 0.0;
        }
    }

    tableau.varInBaseCol[r] = c + 1;
}

DualSolver::~DualSolver(void) {
    delete tableauInOut;
}
