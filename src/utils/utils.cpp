#include "utils.hpp"
#include <sstream>
#include <iomanip>
#include <regex>

/*
 * Be very careful, you can not place template functions here or this function in .hpp header
 * becouse of known C++ compilation problems in that case.
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

std::string getFormattedComplex(const std::complex<double>& num) {
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