#ifndef __TABLEAU__HPP
#define __TABLEAU__HPP

#include "ArrayList.hpp"
#include <complex>

typedef struct Tableau {

    private:
    // Number of equations
    unsigned long m;
    // Number of variables
    unsigned long n;

    // Number of rows in tableau
    unsigned long rows;
    // Number of columns in tableau
    unsigned long cols;

    public:
    // Value of the objective function
    std::complex<double> z;
    // Column that contains known terms
    ArrayList<std::complex<double>> *d;
    // Row that contains variables reduced costs
    ArrayList<double> *c;
    // Matrix of coefficients
    ArrayList<ArrayList<double> *> *A;

    // Column in base for each row
    ArrayList<unsigned long> *variablesInBase;

    const unsigned long getM(void) {
        return d->getLength();
    }
    const unsigned long getN(void) {
        return c->getLength();
    }

    const unsigned long getRows(void) {
        return getM() + 1;
    }
    const unsigned long getCols(void) {
        return getN() + 1;
    }

    void setM(const unsigned long m) {
        this->m = m;
        rows = m + 1;
    }
    void setN(const unsigned long n) {
        this->n = n;
        cols = n + 1;
    }

    void increaseM(void) {
        m++;
        rows++;
    }
    void increaseN(void) {
        n++;
        cols++;
    }
    void decreaseM(void) {
        m--;
        rows--;
    }
    void decreaseN(void) {
        n--;
        cols--;
    }

    bool isValid(void) {
        // Tableau number of ROWS and COLUMNS check
        if (getRows() != getM() + 1UL) {
            return false;
        }
        if (getCols() != getN() + 1UL) {
            return false;
        }

        // Known terms column size
        if (d->getLength() != getM()) {
            return false;
        }
        // Reduced costs row size
        if (c->getLength() != getN()) {
            return false;
        }

        // Coefficient matrix number of rows
        if (A->getLength() != getM()) {
            return false;
        }
        // Coefficient matrix number of columns
        for (unsigned long i = A->getFirstIndex(); i <= A->getLastIndex(); i++) {
            if (A->get(i)->getLength() != getN()) {
                return false;
            }
        }

        // Variables in base list size
        if (variablesInBase->getLength() != getM()) {
            return false;
        }

        return true;
    }
    bool isFeasible(void) {
        if (!isValid()) {
            return false;
        }

        // Reduced costs are positive or zero
        for (unsigned long i = 0; i < c->getLength(); i++) {
            if (c->get(i) < 0) {
                return false;
            }
        }

        // The tableau has a valid base (with different variables each)
        bool base[getN()];
        for (unsigned long i = 0UL; i < getN(); i++) {
            base[i] = false;
        }
        for (unsigned long i = variablesInBase->getFirstIndex(); i <= variablesInBase->getLastIndex(); i++) {
            if ((variablesInBase->get(i) <= 0) || (variablesInBase->get(i) >= getCols())) {
                return false;
            } else {
                base[variablesInBase->get(i) - 1] = true;
            }
        }
        unsigned long count = 0UL;
        for (unsigned long i = 0UL; i < getN(); i++) {
            if (base[i] == true) {
                count++;
            }
        }
        if (count != getM()) {
            return false;
        }

        return true;
    }

} Tableau;

#endif // __TABLEAU__HPP
