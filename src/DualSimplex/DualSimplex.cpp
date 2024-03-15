#include "DualSimplex.hpp"
#include "utils.hpp"
#include <stdlib.h>
#include <cstdio>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <limits>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

DualSimplex::DualSimplex(const char *filePath) {
    const string outFolderName = getFileNameWithoutExtension(filePath);
    stringstream ss;
    ss << FILES_OUT_PATH << outFolderName;
    string folderPath = ss.str();
    ss << "/" << FILES_OUT_PREFIX;
    outFolderPath = ss.str();
    fs::path outDirectory = fs::path(outFolderPath);
    if (!fs::exists(folderPath.c_str())) {
        if (!fs::create_directory(folderPath.c_str())) {
            std::cerr << "Failed to create " << folderPath << " directory." << std::endl;
            outDirectory = FILES_OUT_PATH;
            assert(fs::exists(outDirectory));
        }
    }
    readTableau(filePath);
}

const string DualSimplex::getNextFilename(void) {
    return outFolderPath + to_string(counter++);
}

void DualSimplex::readTableau(const char *filename) {
    FILE *fin = fopen(filename, "r");
    assert(fin != NULL);

    // READ number of rows and columns
    unsigned long m, n;
    assert(fscanf(fin, "%lu %lu", &n, &m) > 0);
    assert(m <= n);
    // TODO: After the matrix is read check max rank condition or remove dependent rows and check again m <=n

    // Slack or surplus terms for each row
    double slackOrSurplus[m];
    // Known terms for each row
    double knownTerms[m];

    tableau.variablesInBase = new ArrayList<unsigned long>(m, 1UL);
    for (unsigned long i = 0UL; i < m; i++) {
        // I assume x0 invalid variable (a good way to initialize)
        tableau.variablesInBase->add(0UL);
    }

    tableau.d = new ArrayList<complex<double>>(m, 1UL);
    // READ known terms column
    for (unsigned long i = 0UL; i < m; i++) {
        double di;
        assert(fscanf(fin, "%lf", &(di)) > 0);
        tableau.d->add(di);
    }

    // Counter of slack and surplus
    unsigned long artificialCount = 0UL;

    // READ slack or surplus vector
    for (unsigned long i = 0UL; i < m; i++) {
        assert(fscanf(fin, "%lf", &(slackOrSurplus[i])) > 0);
        assert(slackOrSurplus[i] == 0.0
            || slackOrSurplus[i] == 1.0
            || slackOrSurplus[i] == -1.0);

        if (slackOrSurplus[i] != 0.0) {
            artificialCount++;
        }
    }

    unsigned long na = n + artificialCount;
    tableau.A = new ArrayList<ArrayList<double> *>(m, 1UL);

    // Init coefficient matrix
    for (unsigned long i = 0UL; i < m; i++) {
        tableau.A->add(new ArrayList<double>(na, 1UL));
        for (unsigned long j = 0UL; j < na; j++) {
            tableau.A->get(i)->add(0UL);
        }
    }

    // Fill with slacks and surplus (artificial variables)
    unsigned long k = 0UL;
    for (unsigned long j = n; j < na; j++) {
        while (slackOrSurplus[k] == 0.0) {
            k++;
        }
        tableau.A->get(k)->get(j) = slackOrSurplus[k];
        tableau.variablesInBase->get(k) = j + 1;
        k++;
    }

    tableau.c = new ArrayList<double>(na, 1UL);
    // READ the coefficients matrix A
    for (unsigned long j = 0UL; j < n; j++) {
        bool inBase = true;

        // READ column reduced cost
        double cc;
        assert(fscanf(fin, "%lf", &cc) > 0);
        if (cc != 0.0) {
            inBase = false;
        }
        tableau.c->add(cc);

        // READ number of non zeros coefficients
        unsigned long not0;
        assert(fscanf(fin, "%lu", &not0) > 0);
        if (inBase && not0 != 1UL) {
            inBase = false;
        }

        // READ non zeros coefficients
        unsigned long rr;
        for (unsigned long k = 0UL; k < not0; k++) {
            assert(fscanf(fin, "%lu", &rr) > 0);
            assert(fscanf(fin, "%lf", &(tableau.A->get(rr - 1)->get(j))) > 0);
        }
        if (inBase && tableau.A->get(rr - 1UL)->get(j) == 1.0) {
            tableau.variablesInBase->get(rr - 1UL) = j + 1;
        }
    }
    for (unsigned long j = n; j < na; j++) {
        tableau.c->add(0.0);
    }

    assert(fclose(fin) == 0);

    printTableauInfeasible("Tableau represented as read.\n"
    "The first row contains the objective function negate and reduced costs.\n"
    "The first column contains the known terms.\n"
    "x1..n are the decision variables");
}

void DualSimplex::printTableau(const bool isFeasible, const string& description = "", const string& filename = "") {
    // VERIFY what declared
    if (isFeasible) {
        assert(tableau.isFeasible());
    } else {
        assert(tableau.isValid());
    }

    FILE *fout;
    if (filename == "") {
        fout = fopen(getNextFilename().c_str(), "w");
    } else {
        fout = fopen(filename.c_str(), "w");
    }
    assert(fout != NULL);

    if (isFeasible) {
        assert(fprintf(fout, "FEASIBLE TABLEAU\n\n") > 0);
    } else {
        assert(fprintf(fout, "INFEASIBLE TABLEAU\n\n") > 0);
    }

    if (description != "") {
        assert(fprintf(fout, "%s", description.c_str()) > 0);
        assert(fprintf(fout, ".\n\n") > 0);
    }
    assert(fprintf(fout, "SIZE: %lu x %lu\n\n", tableau.getRows(), tableau.getCols()) > 0);

    if (tableau.getCols() > 0LU) {
        assert(fprintf(fout, "          d") > 0);
        for (unsigned i = 0U; i < COMPLEX_LENGTH - 1; i++) {
            assert(fprintf(fout, " ") > 0);
        }
    }
    for (unsigned long j = 1UL; j < tableau.getCols(); j++) {
        assert(fprintf(fout, "    x%-5lu", j) > 0);
        for (unsigned i = 6U; i < REAL_LENGTH; i++) {
            assert(fprintf(fout, " ") > 0);
        }
    }

    assert(fprintf(fout, "\nc     ") > 0);
    assert(fprintf(fout, "    %s", getFormattedComplex(tableau.z).c_str()) > 0);
    for (unsigned long j = 0UL; j < tableau.getN(); j++) {
        assert(fprintf(fout, "    %s", getFormattedDouble(tableau.c->get(j)).c_str()) > 0);
    }

    for (unsigned long i = 0UL; i < tableau.getM(); i++) {
        if (tableau.variablesInBase->get(i) == 0) {
            assert(fprintf(fout, "\n?     ") > 0);
        } else {
            assert(fprintf(fout, "\nx%-5lu", tableau.variablesInBase->get(i)) > 0);
        }
        assert(fprintf(fout, "    %s", getFormattedComplex(tableau.d->get(i)).c_str()) > 0);
        for (unsigned long j = 0UL; j < tableau.getN(); j++) {
            assert(fprintf(fout, "    %s", getFormattedDouble(tableau.A->get(i)->get(j)).c_str()) > 0);
        }
    }
    assert(fprintf(fout, "\n") > 0);

    assert(fclose(fout) == 0);
}

void DualSimplex::printTableauFeasible(const string& description) {
    printTableau(true, description);
}

void DualSimplex::printTableauInfeasible(const string& description) {
    printTableau(false, description);
}

void DualSimplex::startDualSimplex(void) {
    bool notBasic = false;
    for (unsigned long i = 0UL; i < tableau.getM(); i++) {
        if (tableau.variablesInBase->get(i) == 0UL) {
            for (unsigned long j = 0; i < tableau.getN(); j++) {
                if (tableau.A->get(i)->get(j) != 0.0) {
                    putInBase(i, j);
                    break;
                }
            }
            notBasic = true;
        }
    }

    if (notBasic) {
        printTableauInfeasible("Create basic solution");
    }

    if (!validateCosts()) {
        printTableauFeasible("Add artificial variable so as to make the costs positive");
    }

    complex<double> mostNegativeKnownTerm;
    do {
        mostNegativeKnownTerm = complex<double>();
        unsigned long row;

        for (unsigned long i = 0UL; i < tableau.getM(); i++) {
            if ((tableau.d->get(i).imag() < mostNegativeKnownTerm.imag())
                || ((tableau.d->get(i).imag() == mostNegativeKnownTerm.imag())
                    && (tableau.d->get(i).real() < mostNegativeKnownTerm.real()))) {
                mostNegativeKnownTerm = tableau.d->get(i);
                row = i;
            }
        }

        if (complexIsNotZero<double>(mostNegativeKnownTerm)) {
            double min = numeric_limits<double>::infinity();
            unsigned long col;

            for (unsigned long j = 0UL; j < tableau.getN(); j++) {
                if (tableau.A->get(row)->get(j) < 0.0) {
                    const double tmp = tableau.c->get(j) / abs(tableau.A->get(row)->get(j));
                    if (tmp < min) {
                        min = tmp;
                        col = j;
                    }
                }
            }

            if (min != numeric_limits<double>::infinity()) {
                putInBase(row, col);
                // printTableauFeasible("Dual Simplex iteration");
            } else {
                printTableauFeasible("SOLUTION unbounded, primal infeasible");
                return;
            }
        }
    } while (complexIsNotZero<double>(mostNegativeKnownTerm));

    // if (complexIsZero<double>(tableau.d->getLast()) && (tableau.c->getLast() == 0.0)) {
    //     // .... ?? in base or not in base ??
    // }
    printTableauFeasible("SOLUTION feasible");
}

bool DualSimplex::validateCosts(void) {
    unsigned long k;
    double mostNegativeCost = 0.0;

    for (unsigned long j = 0UL; j < tableau.getN(); j++) {
        if ((tableau.c->get(j) < 0.0) && (tableau.c->get(j) < mostNegativeCost)) {
            mostNegativeCost = tableau.c->get(j);
            k = j;
        }
    }

    if (mostNegativeCost != 0.0) {
        for (unsigned long i = 0UL; i < tableau.getM(); i++) {
            tableau.A->get(i)->add(0.0);
        }
        tableau.c->add(0.0);
        tableau.increaseN();
        tableau.A->add(new ArrayList<double>(tableau.getN(), 1UL));
        tableau.d->add(complex<double>(0.0, 1.0)); // TODO: fare con i numeri complessi!!
        for (unsigned long j = 0UL; j < tableau.getN(); j++) {
            if (tableau.c->get(j) == 0.0) {
                tableau.A->getLast()->add(0.0);
            } else {
                tableau.A->getLast()->add(1.0);
            }
        }
        tableau.A->getLast()->getLast() = 1.0;
        tableau.increaseM();
        tableau.variablesInBase->add(tableau.getN());
        putInBase(tableau.A->getLastIndex(), k);

        return false;
    }

    return true;
}

void DualSimplex::putInBase(const unsigned long r, const unsigned long c) {
    assert(tableau.A->get(r)->get(c) != 0.0);
    double multiplier;

    // Adapts target row
    if (tableau.A->get(r)->get(c) != 1.0) {
        multiplier = 1.0 / tableau.A->get(r)->get(c);
        tableau.A->get(r)->get(c) = 1.0;
        tableau.d->get(r) *= multiplier;
        for (unsigned long j = 0UL; j < tableau.getN(); j++) {
            if (j != c) {
                tableau.A->get(r)->get(j) *= multiplier;
            }
        }
    }

    // Updates reduced costs row
    if (tableau.c->get(c) != 0.0) {
        multiplier = -tableau.c->get(c);
        tableau.c->get(c) = 0.0;
        tableau.z += multiplier * tableau.d->get(r);
        for (unsigned long j = 0UL; j < tableau.getN(); j++) {
            if (j != c) {
                tableau.c->get(j) += multiplier * tableau.A->get(r)->get(j);
            }
        }
    }

    // Updates every other row
    for (unsigned long i = 0; i < tableau.getM(); i++) {
        if ((i != r) && (tableau.A->get(i)->get(c) != 0.0)) {
            multiplier = -tableau.A->get(i)->get(c);
            tableau.A->get(i)->get(c) = 0.0;
            tableau.d->get(i) += multiplier * tableau.d->get(r);
            for (unsigned long j = 0UL; j < tableau.getN(); j++) {
                if (j != c) {
                    tableau.A->get(i)->get(j) += multiplier * tableau.A->get(r)->get(j);
                }
            }
        }
    }

    tableau.variablesInBase->get(r) = c + 1;
}

DualSimplex::~DualSimplex(void) {
    for (unsigned long i = 0UL; i < tableau.A->getLength(); i++) {
        delete tableau.A->get(i);
    }
    delete tableau.A;
    delete tableau.d;
    delete tableau.c;
    delete tableau.variablesInBase;
}
