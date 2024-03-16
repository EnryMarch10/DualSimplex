#include "TableauFilesManager.hpp"
#include "utils.hpp"
#include <assert.h>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

TableauFilesManager::TableauFilesManager(Tableau *const tableau, const char *filePath) {
    assert(tableau != NULL && filePath != NULL);

    const string outFolderName = getFileNameWithoutExtension(filePath);
    stringstream ss;
    ss << FILES_OUT_PATH << outFolderName;
    string folderPath = ss.str();
    ss << "/" << FILES_OUT_PREFIX;
    outPartialFilePath = ss.str();
    fs::path outDirectory = fs::path(outPartialFilePath);
    if (!fs::exists(folderPath.c_str())) {
        if (!fs::create_directory(folderPath.c_str())) {
            std::cerr << "Failed to create " << folderPath << " directory." << std::endl;
            outDirectory = FILES_OUT_PATH;
            assert(fs::exists(outDirectory));
        }
    }

    this->inFilePath = filePath;
    this->tableau = tableau;
}

const string TableauFilesManager::getNextFilename(void) {
    return outPartialFilePath + to_string(counter++);
}

void TableauFilesManager::readTableau(void) {
    FILE *fin = fopen(inFilePath, "r");
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

    tableau->varInBaseCol.resize(m);
    tableau->varInBaseCol.fill(0);

    tableau->knownTermsCol.resize(m);
    // READ known terms column
    for (Eigen::Index i = 0; i < m; i++) {
        double di;
        assert(fscanf(fin, "%lf", &(di)) > 0);
        tableau->knownTermsCol[i] = di;
    }

    // Counter of slack and surplus
    Eigen::Index artificialCount = 0;

    // READ slack or surplus vector
    for (Eigen::Index i = 0; i < m; i++) {
        assert(fscanf(fin, "%lf", &(slackOrSurplus[i])) > 0);
        assert(slackOrSurplus[i] == 0.0
            || slackOrSurplus[i] == 1.0
            || slackOrSurplus[i] == -1.0);

        if (slackOrSurplus[i] != 0.0) {
            artificialCount++;
        }
    }

    unsigned long na = n + artificialCount;
    tableau->coefficientsMatrix = Eigen::MatrixXd::Zero(m, na);

    // Fill with slacks and surplus (artificial variables)
    Eigen::Index k = 0;
    for (Eigen::Index j = n; j < na; j++) {
        while (slackOrSurplus[k] == 0.0) {
            k++;
        }
        tableau->coefficientsMatrix(k, j) = slackOrSurplus[k];
        if (slackOrSurplus[k] == -1.0) {
            tableau->coefficientsMatrix.row(k) = -tableau->coefficientsMatrix.row(k);
        }
        tableau->varInBaseCol[k] = j + 1;
        k++;
    }

    tableau->costsRow = Eigen::ArrayXd::Zero(na);
    // READ the coefficients matrix A
    for (Eigen::Index j = 0; j < n; j++) {
        bool inBase = true;

        // READ column reduced cost
        double cc;
        assert(fscanf(fin, "%lf", &cc) > 0);
        if (cc != 0.0) {
            inBase = false;
        }
        tableau->costsRow[j] = cc;

        // READ number of non zeros coefficients
        unsigned long not0; // TODO: vedi come fare per il tipo
        assert(fscanf(fin, "%lu", &not0) > 0);
        if (inBase && not0 != 1) {
            inBase = false;
        }

        // READ non zeros coefficients
        unsigned long rr; // TODO: vedi come fare per il tipo
        for (unsigned long k = 0; k < not0; k++) {
            assert(fscanf(fin, "%lu", &rr) > 0);
            assert(fscanf(fin, "%lf", &(tableau->coefficientsMatrix(rr - 1, j))) > 0);
        }
        if (inBase && (tableau->coefficientsMatrix(rr - 1, j) == 1.0)) {
            tableau->varInBaseCol[rr - 1] = j + 1;
        }
    }

    assert(fclose(fin) == 0);

    printTableauInfeasible("Tableau represented as read.\n"
    "The first row contains the objective function negate and reduced costs.\n"
    "The first column contains the known terms.\n"
    "x1..n are the decision variables");
}

void TableauFilesManager::printTableau(const string& title, const string& description,
                                 const string& filename, const bool doCheck,
                                 const bool isDualFeasible = true) {
    string caption;
    if (doCheck) {
        if (isDualFeasible) {
            assert(tableau->isDualFeasible());
            caption = "FEASIBLE TABLEAU";
        } else {
            assert(tableau->isValid());
            caption = "INFEASIBLE TABLEAU";
        }
    } else {
        caption = title;
    }

    FILE *fout;
    if (filename == "") {
        fout = fopen(getNextFilename().c_str(), "w");
    } else {
        fout = fopen(filename.c_str(), "w");
    }
    assert(fout != NULL);

    assert(fprintf(fout, "%s\n\n", caption.c_str()) > 0);

    assert(description != "");
    assert(fprintf(fout, "%s", description.c_str()) > 0);
    assert(fprintf(fout, ".\n\n") > 0);

    assert(fprintf(fout, "SIZE: %lu x %lu\n\n", tableau->getRows(), tableau->getCols()) > 0);

    if (tableau->getCols() > 0LU) {
        assert(fprintf(fout, "          d") > 0);
        for (Eigen::Index i = 0U; i < COMPLEX_LENGTH - 1; i++) {
            assert(fprintf(fout, " ") > 0);
        }
    }
    for (Eigen::Index j = 1; j < tableau->getCols(); j++) {
        assert(fprintf(fout, "    x%-5lu", j) > 0);
        for (Eigen::Index i = 6U; i < REAL_LENGTH; i++) {
            assert(fprintf(fout, " ") > 0);
        }
    }

    assert(fprintf(fout, "\nc     ") > 0);
    assert(fprintf(fout, "    %s", getFormattedComplex(tableau->z).c_str()) > 0);
    for (Eigen::Index j = 0; j < tableau->getN(); j++) {
        assert(fprintf(fout, "    %s", getFormattedDouble(tableau->costsRow[j]).c_str()) > 0);
    }

    for (Eigen::Index i = 0UL; i < tableau->getM(); i++) {
        if (tableau->varInBaseCol[i] == 0) {
            assert(fprintf(fout, "\n?     ") > 0);
        } else {
            assert(fprintf(fout, "\nx%-5lu", tableau->varInBaseCol[i]) > 0);
        }
        assert(fprintf(fout, "    %s", getFormattedComplex(tableau->knownTermsCol[i]).c_str()) > 0);
        for (Eigen::Index j = 0; j < tableau->getN(); j++) {
            assert(fprintf(fout, "    %s", getFormattedDouble(tableau->coefficientsMatrix(i, j)).c_str()) > 0);
        }
    }
    assert(fprintf(fout, "\n") > 0);

    assert(fclose(fout) == 0);
}

void TableauFilesManager::printTableau(const string& title, const string& description, const string& filename) {
    printTableau(title, description, filename, false);
}

void TableauFilesManager::printTableauWithCheck(const string& title, const string& description, const string& filename) {
    printTableau(title, description, filename, true, true);
}

void TableauFilesManager::printTableauFeasible(const string& description) {
    printTableau("", description, "", true);
}

void TableauFilesManager::printTableauInfeasible(const string& description) {
    printTableau("", description, "", false);
}
