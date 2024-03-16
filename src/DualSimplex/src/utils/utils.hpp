#ifndef __UTILS__H
#define __UTILS__H

#include <string>
#include <complex>
#include <Eigen/Dense>

// Files locations
#define FILES_IN_PATH "../data/in/"
#define FILES_OUT_PATH "../data/out/"
#define FILES_OUT_PREFIX "tableau_"

// Output tableau format
#define PRECISION 2
#define COMPLEX_LENGTH 17
#define REAL_LENGTH 7

#define XOR_SWAP(X, Y)  {\
                            (X) ^= (Y);\
                            (Y) ^= (X);\
                            (X) ^= (Y);\
                        }

#define TMP_SWAP(tmp, X, Y) {\
                                (tmp) = (X);\
                                (X) = (Y);\
                                (Y) = (tmp);\
                            }

#define ABS_DIFF(x, y) ((x) > (y) ? (x) - (y) : (y) - (x))

#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define MIN(x, y) ((x) < (y) ? (x) : (y))

template<class T>
void swap(T& a, T& b)
{
    T tmp = a;
    a = b;
    b = tmp;
}

bool complexIsZero(const Eigen::dcomplex& value);

bool complexIsNotZero(const Eigen::dcomplex& value);

std::string getFileNameWithoutExtension(const std::string& input);

std::string getFormattedComplex(const Eigen::dcomplex& num);

std::string getFormattedDouble(const double& num);

#endif // __UTILS__H
