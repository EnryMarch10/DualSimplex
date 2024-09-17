#include "utils.hpp"
#include <sstream>
#include <iomanip>
#include <regex>

/*
 * Be very careful, you can not place template functions here or this function in .hpp header
 * because of known C++ compilation problems in that case.
*/

std::string getFileNameWithoutExtension(const std::string& input) {
    std::regex re(".*/(.*)\\.[^.]*$|.*/(.*)$");
    std::smatch match;
    if (std::regex_search(input, match, re)) {
        return match[1];
    } else {
        return "";
    }
}

std::string getFormattedComplex(const Eigen::dcomplex& num) {
    std::string real;
    if (num.real() == 0.0) {
        real = "0";
    } else {
        std::stringstream ss1;
        ss1 << std::fixed << std::setprecision(PRECISION) << num.real();
        real = ss1.str();
    }

    if (num.imag() == 0.0) {
        if (real.length() < COMPLEX_LENGTH) {
            real = real + std::string(COMPLEX_LENGTH - real.length(), ' ');
        }
        return real;
    } else if (num.real() == 0.0) {
        std::stringstream ss2;
        ss2 << std::fixed << std::setprecision(PRECISION) << num.imag() << "M";
        std::string imag = ss2.str();

        if (imag.length() < COMPLEX_LENGTH) {
            imag = imag + std::string(COMPLEX_LENGTH - imag.length(), ' ');
        }
        return imag;
    }

    std::stringstream ss2;
    ss2 << std::fixed << std::setprecision(PRECISION) << abs(num.imag());
    std::string imag = ss2.str();

    imag = imag + "M";

    std::string result;
    if (num.imag() > 0.0) {
        result = real + " + " + imag;
    } else {
        result = real + " - " + imag;
    }
    
    if (result.length() < COMPLEX_LENGTH) {
        result = result + std::string(COMPLEX_LENGTH - result.length(), ' ');
    }
    return result;
}

std::string getFormattedDouble(const double& num) {
    std::string str;
    if (num == 0.0) {
        str = "0";
    } else {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(PRECISION) << num;
        str = ss.str();
    }

    if (str.length() < REAL_LENGTH) {
        str = str + std::string(REAL_LENGTH - str.length(), ' ');
    }
    
    return str;
}

// Uses Gauss-Jordan Elimination algorithm to determine linearly dependent rows
std::vector<Eigen::Index> getDependentRows(const Eigen::MatrixXd& matrix) {
    Eigen::MatrixXd mat = matrix; // Make a copy to avoid modifying the original matrix
    Eigen::Index numRows = mat.rows();
    Eigen::Index numCols = mat.cols();
    std::vector<Eigen::Index> dependentIndices;

    Eigen::Index i = 0, j = 0;
    while (i < numRows && j < numCols) {
        // Find pivot for this column
        Eigen::Index pivotRow = i;
        while (pivotRow < numRows && mat(pivotRow, j) == 0.0) {
            pivotRow++;
        }

        if (pivotRow == numRows) {
            j++;
            continue;
        }

        // Swap rows to get nonzero pivot
        if (pivotRow != i) {
            mat.row(pivotRow).swap(mat.row(i));
        }

        // Normalize pivot row
        double pivotValue = mat(i, j);
        mat.row(i) /= pivotValue;

        // Eliminate nonzero elements below the pivot
        for (Eigen::Index k = 0; k < numRows; k++) {
            if (k != i && mat(k, j) != 0.0) {
                double factor = mat(k, j);
                mat.row(k) -= factor * mat.row(i);
            }
        }

        i++;
        j++;
    }

    // Any row with all zero elements is dependent
    for (Eigen::Index i = 0; i < numRows; i++) {
        if (mat.row(i).isZero()) {
            dependentIndices.push_back(i);
        }
    }

    return dependentIndices;
}

bool complexIsZero(const Eigen::dcomplex& value) {
    return (value.real() == 0.0) && (value.imag() == 0.0);
}

bool complexIsNotZero(const Eigen::dcomplex& value) {
    return (value.real() != 0.0) || (value.imag() != 0.0);
}

bool isZero(const double& value, const double& tolerance) {
    return std::fabs(value) < tolerance;
}

bool isNotZero(const double& value, const double& tolerance) {
    return !isZero(value, tolerance);
}

bool isLessThanZero(const double& value, const double& tolerance) {
    return value < -tolerance;
}

bool isMoreThanZero(const double& value, const double& tolerance) {
    return value > tolerance;
}
