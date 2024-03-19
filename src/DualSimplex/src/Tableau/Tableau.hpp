#ifndef __TABLEAU__HPP
#define __TABLEAU__HPP

#include <Eigen/Dense>

namespace Eigen {
    typedef Eigen::Array<Eigen::Index, Eigen::Dynamic, 1> ArrayXindex;
    typedef Eigen::Array<bool, Eigen::Dynamic, 1> ArrayXb;
}

typedef struct Tableau {
    // Value of the objective function as a minimization problem (negated result if you want to minimize)
    Eigen::dcomplex objFunc;
    // Column that contains known terms adapted to the minimization problem
    Eigen::ArrayXcd knownTermsCol;
    // Row that contains variables reduced costs
    Eigen::ArrayXd redCostsRow;
    // Matrix of coefficients
    Eigen::MatrixXd coeffMatrix;

    // Column in base for each row
    Eigen::ArrayXindex varInBaseCol;

    // Number of equations
    Eigen::Index getM(void) {
        return knownTermsCol.size();
    }
    // Number of variables
    Eigen::Index getN(void) {
        return redCostsRow.size();
    }

    // Number of rows in tableau
    Eigen::Index getRows(void) {
        return getM() + 1;
    }
    // Number of columns in tableau
    Eigen::Index getCols(void) {
        return getN() + 1;
    }

    bool isValid(void) {
        return coeffMatrix.rows() == getM() // Coefficient matrix number of rows
            && coeffMatrix.cols() == getN() // Coefficient matrix number of columns
            && redCostsRow.size() == getN() // Reduced costs row size
            && knownTermsCol.size() == getM() // Known terms column size
            && varInBaseCol.size() == getM(); // Variables in base column size
    }

    bool isFeasible(void) {
        return getM() <= getN(); // Variables >= equations
    }

    bool isDualFeasible(void) {
        if (!isFeasible()) {
            return false;
        }

        // Reduced costs are positive or zero
        if ((redCostsRow < 0.0).any()) {
            return false;
        }

        // The tableau has a valid base (with different variables each)
        Eigen::ArrayXb isInBase = Eigen::ArrayXb::Constant(getN(), false);
        for (Eigen::Index i = 0; i < getM(); i++) {
            if ((varInBaseCol[i] <= 0) || (varInBaseCol[i] >= getCols())) {
                return false;
            } else {
                isInBase[varInBaseCol[i] - 1] = true;
            }
        }

        return (isInBase == true).count() == getM();
    }

} Tableau;

#endif // __TABLEAU__HPP
