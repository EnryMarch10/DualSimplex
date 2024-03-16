#ifndef __TABLEAU__HPP
#define __TABLEAU__HPP

#include <Eigen/Dense>

namespace Eigen {
    typedef Eigen::Array<Eigen::Index, Eigen::Dynamic, 1> ArrayXindex;
    typedef Eigen::Array<bool, Eigen::Dynamic, 1> ArrayXb;
}

typedef struct Tableau {
    // Value of the objective function
    Eigen::dcomplex z;
    // Column that contains known terms
    Eigen::ArrayXcd knownTermsCol;
    // Row that contains variables reduced costs
    Eigen::RowVectorXd costsRow;
    // Matrix of coefficients
    Eigen::MatrixXd coefficientsMatrix;

    // Column in base for each row
    Eigen::ArrayXindex varInBaseCol;

    // Number of equations
    const Eigen::Index getM(void) {
        return knownTermsCol.size();
    }
    // Number of variables
    const Eigen::Index getN(void) {
        return costsRow.size();
    }

    // Number of rows in tableau
    const Eigen::Index getRows(void) {
        return getM() + 1;
    }
    // Number of columns in tableau
    const Eigen::Index getCols(void) {
        return getN() + 1;
    }

    bool isValid(void) {
        return coefficientsMatrix.rows() == getM() // Coefficient matrix number of rows
            && coefficientsMatrix.cols() == getN() // Coefficient matrix number of columns
            && costsRow.size() == getN() // Reduced costs row size
            && knownTermsCol.size() == getM() // Known terms column size
            && varInBaseCol.size() == getM(); // Variables in base column size
    }

    bool isDualFeasible(void) {
        if (!isValid()) {
            return false;
        }

        // Reduced costs are positive or zero
        if ((costsRow.array() < 0.0).any()) {
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
