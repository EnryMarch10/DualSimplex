#include "DualSolver.hpp"
#include "utils.hpp"
#include <assert.h>
#include <stdlib.h>
#include <limits>

using namespace std;

DualSolver::DualSolver(const char *filePath) {
    tableauInOut = new TableauFilesManager(&tableau, filePath);
    tableauInOut->readTableau();
}

void DualSolver::startDualSimplex(void) {
    bool notBasic = false;

    for (Eigen::Index i = 0; i < tableau.getM(); i++) {
        if (tableau.varInBaseCol[i] == 0) {
            for (Eigen::Index j = 0; i < tableau.getN(); j++) {
                if (tableau.coefficientsMatrix(i, j) != 0.0) {
                    doDualPivot(i, j);
                    break;
                }
            }
            notBasic = true;
        }
    }

    if (notBasic) {
        tableauInOut->printTableauInfeasible("Create basic solution");
    }

    bool invalidCosts = !doDualCostsValidation();
    if (invalidCosts) {
        tableauInOut->printTableauFeasible("Add artificial variable so as to make the costs positive");
    }

    // TODO: this should be done with BLAND RULE (think about what can be done with the array of the variables in base)
    Eigen::dcomplex mostNegativeKnownTerm;
    do {
        mostNegativeKnownTerm = Eigen::dcomplex();
        unsigned long row;

        for (Eigen::Index i = 0; i < tableau.getM(); i++) {
            if ((tableau.knownTermsCol[i].imag() < mostNegativeKnownTerm.imag())
                || ((tableau.knownTermsCol[i].imag() == mostNegativeKnownTerm.imag())
                    && (tableau.knownTermsCol[i].real() < mostNegativeKnownTerm.real()))) {
                mostNegativeKnownTerm = tableau.knownTermsCol[i];
                row = i;
            }
        }

        if (complexIsNotZero(mostNegativeKnownTerm)) {
            double min = numeric_limits<double>::infinity();
            Eigen::Index col;

            for (Eigen::Index j = 0; j < tableau.getN(); j++) {
                if (tableau.coefficientsMatrix(row, j) < 0.0) {
                    const double tmp = tableau.costsRow[j] / abs(tableau.coefficientsMatrix(row, j));
                    if (tmp < min) {
                        min = tmp;
                        col = j;
                    }
                }
            }

            if (min != numeric_limits<double>::infinity()) {
                doDualPivot(row, col);
                // printTableauFeasible("Dual Simplex iteration");
            } else {
                tableauInOut->printTableauFeasible("SOLUTION unbounded, primal infeasible");
                return;
            }
        }
    } while (complexIsNotZero(mostNegativeKnownTerm));

    if (invalidCosts && complexIsZero(tableau.knownTermsCol[tableau.knownTermsCol.size() - 1])) {
        if (tableau.costsRow[tableau.getN() - 1] > 0.0) {
            stringstream ss;
            ss << "SOLUTION: Primal unbounded, artifical variable x" << tableau.getN() << " not in base";
            const string summary = ss.str();
            tableauInOut->printTableauFeasible(summary);
        }
        // Otherwise it cannot be `tableau.costsRow[tableau.getN() - 1] < 0.0`, so it
        // is not necessary to verify this case because of dual conditions
        // so in this case: `tableau.costsRow[tableau.getN() - 1] == 0.0`
        // that is considered optimal though not basic to the original problem.
    }
    tableauInOut->printTableauFeasible("SOLUTION: feasible optimal solution reached");
}

bool DualSolver::doDualCostsValidation(void) {
    Eigen::Index index;
    assert(tableau.costsRow.minCoeff(&index));

    if (tableau.costsRow[index] < 0.0) {
        Eigen::Index size1 = tableau.coefficientsMatrix.rows();
        Eigen::Index size2 = tableau.coefficientsMatrix.cols();

        tableau.coefficientsMatrix.conservativeResize(size1 + 1, size2 + 1);
        tableau.coefficientsMatrix.row(size1).setZero();
        tableau.coefficientsMatrix.col(size2).setZero();

        size1 = tableau.costsRow.size();
        tableau.costsRow.conservativeResize(size1 + 1);
        tableau.costsRow[size1] = 0.0;

        size1 = tableau.knownTermsCol.size();
        tableau.knownTermsCol.conservativeResize(size1 + 1);
        tableau.knownTermsCol[size1] = Eigen::dcomplex(0.0, 1.0);

        size1 = tableau.coefficientsMatrix.rows() - 1;
        size2 = tableau.coefficientsMatrix.cols() - 1;
        for (Eigen::Index j = 0; j < size2; j++) {
            if (tableau.costsRow[j] != 0.0) {
                tableau.coefficientsMatrix(size1, j) = 1.0;
            }
        }
        tableau.coefficientsMatrix(size1, size2) = 1.0;

        size1 = tableau.varInBaseCol.size();
        tableau.varInBaseCol.conservativeResize(size1 + 1);
        tableau.varInBaseCol[size1] = tableau.getN() + 1;

        doDualPivot(tableau.coefficientsMatrix.rows() - 1, index);
        return false;
    }

    return true;
}

void DualSolver::doDualPivot(const Eigen::Index r, const Eigen::Index c) {
    assert(tableau.coefficientsMatrix(r, c) != 0.0);
    double multiplier;

    // Adapts target row
    if (tableau.coefficientsMatrix(r, c) != 1.0) {
        multiplier = 1.0 / tableau.coefficientsMatrix(r, c);
        tableau.coefficientsMatrix(r, c) = 1.0;
        tableau.knownTermsCol[r] *= multiplier;
        for (Eigen::Index j = 0; j < tableau.coefficientsMatrix.cols(); j++) {
            if (j != c) {
                tableau.coefficientsMatrix(r, j) *= multiplier;
            }
        }
    }

    // Updates reduced costs row
    if (tableau.costsRow[c] != 0.0) {
        multiplier = -tableau.costsRow[c];
        tableau.costsRow[c] = 0.0;
        tableau.z += multiplier * tableau.knownTermsCol[r];
        for (Eigen::Index j = 0; j < tableau.getN(); j++) {
            if (j != c) {
                tableau.costsRow[j] += multiplier * tableau.coefficientsMatrix(r, j);
            }
        }
    }

    // Updates every other row
    for (Eigen::Index i = 0; i < tableau.coefficientsMatrix.rows(); i++) {
        if ((i != r) && (tableau.coefficientsMatrix(i, c) != 0.0)) {
            multiplier = -tableau.coefficientsMatrix(i, c);
            tableau.coefficientsMatrix(i, c) = 0.0;
            tableau.knownTermsCol[i] += multiplier * tableau.knownTermsCol[r];
            for (Eigen::Index j = 0; j < tableau.coefficientsMatrix.cols(); j++) {
                if (j != c) {
                    tableau.coefficientsMatrix(i, j) += multiplier * tableau.coefficientsMatrix(r, j);
                }
            }
        }
    }

    tableau.varInBaseCol[r] = c + 1;
}

DualSolver::~DualSolver(void) {
    delete tableauInOut;
}
