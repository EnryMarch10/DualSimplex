#include "TableauFilesManager.hpp"
#include "utils.hpp"
#include <assert.h>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <filesystem>
#include <vector>

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
    return outPartialFilePath + to_string(counter++) + FILES_OUT_EXTENSION;
}

void TableauFilesManager::readTableau(void) {
    FILE *fin = fopen(inFilePath, "r");
    assert(fin != NULL);

    // READ number of rows and columns
    unsigned long m, n;
    assert(fscanf(fin, "%lu %lu", &n, &m) > 0);
    // It's not important to check here m <= n because there could be linear dependence between equations
    // This check will be done later on

    // Slack or surplus terms for each row
    vector<double> slackOrSurplus(m);

    tableau->varInBaseCol.resize(m);
    tableau->varInBaseCol.fill(0);

    tableau->knownTermsCol.resize(m);
    // READ known terms column
    for (unsigned long i = 0; i < m; i++) {
        double di;
        assert(fscanf(fin, "%lf", &(di)) > 0);
        tableau->knownTermsCol[i] = di;
    }

    // Counter of slack and surplus
    Eigen::Index artificialCount = 0;

    // READ slack or surplus vector
    for (unsigned long i = 0; i < m; i++) {
        assert(fscanf(fin, "%lf", &(slackOrSurplus[i])) > 0);
        assert(slackOrSurplus[i] == 0.0
            || slackOrSurplus[i] == 1.0
            || slackOrSurplus[i] == -1.0);

        if (slackOrSurplus[i] != 0.0) {
            artificialCount++;
        }
    }

    unsigned long na = n + artificialCount;
    tableau->coeffMatrix = Eigen::MatrixXd::Zero(m, na);

    tableau->redCostsRow = Eigen::ArrayXd::Zero(na);
    // READ the coefficients matrix A
    for (unsigned long j = 0; j < n; j++) {
        bool inBase = true;

        // READ column reduced cost
        double cc;
        assert(fscanf(fin, "%lf", &cc) > 0);
        if (cc != 0.0) {
            inBase = false;
        }
        tableau->redCostsRow[j] = cc;

        // READ number of non zeros coefficients
        unsigned long not0;
        assert(fscanf(fin, "%lu", &not0) > 0);
        if (inBase && not0 != 1) {
            inBase = false;
        }

        // READ non zeros coefficients
        unsigned long rr;
        for (unsigned long k = 0; k < not0; k++) {
            assert(fscanf(fin, "%lu", &rr) > 0);
            assert(fscanf(fin, "%lf", &(tableau->coeffMatrix(rr - 1, j))) > 0);
        }
        if (inBase && (tableau->coeffMatrix(rr - 1, j) == 1.0)) {
            tableau->varInBaseCol[rr - 1] = j + 1;
        }
    }

    // Fill with slacks and surplus (artificial variables)
    Eigen::Index k = 0;
    for (unsigned long j = n; j < na; j++) {
        while (slackOrSurplus[k] == 0.0) {
            k++;
        }
        tableau->coeffMatrix(k, j) = slackOrSurplus[k];
        if (slackOrSurplus[k] == -1.0) {
            tableau->coeffMatrix.row(k) *= -1;
            tableau->knownTermsCol[k] *= -1;
        }
        tableau->varInBaseCol[k] = j + 1;
        k++;
    }

    assert(fclose(fin) == 0);

    printTableau("Tableau represented as read.\n"
    "The first row contains the objective function negate and reduced costs.\n"
    "The first column contains the known terms.\n"
    "x1..n are the decision variables.", "AS READ");
}

void TableauFilesManager::printTableauShort(const string& caption, const string& description,
                                            const string& title, const string& filename) {

    FILE *fout;
    if (filename == "") {
        fout = fopen(getNextFilename().c_str(), "w");
    } else {
        fout = fopen(filename.c_str(), "w");
    }
    assert(fout != NULL);

    assert(fprintf(fout, "%s\n\n\n", caption.c_str()) > 0);

    if (title != "") {
        assert(fprintf(fout, "%s\n\n", title.c_str()) > 0);
    }

    assert(description != "");
    assert(fprintf(fout, "%s", description.c_str()) > 0);
    assert(fprintf(fout, "\n\n") > 0);

    assert(fprintf(fout, "Tableau rows x cols: %lu x %lu\n\n", tableau->getRows(), tableau->getCols()) > 0);

    assert(fprintf(fout, "OBJECTIVE FUNCTION VALUE:\n") > 0);
    assert(fprintf(fout, "%s\n\n", getFormattedComplex(-tableau->objFunc).c_str()) > 0);

    assert(fprintf(fout, "OBJECTIVE FUNCTION VALUE (negated):\n") > 0);
    assert(fprintf(fout, "%s\n\n", getFormattedComplex(tableau->objFunc).c_str()) > 0);

    assert(fprintf(fout, "KNOWN TERMS:\n") > 0);
    for (Eigen::Index i = 0; i < tableau->getM(); i++) {
        assert(fprintf(fout, "%s (x%-5lu -> x%-5lu)  ",
            getFormattedComplex(tableau->knownTermsCol[i]).c_str(),
            i + 1,
            tableau->varInBaseCol[i]) > 0);
        if ((i + 1) % 3 == 0) {
            assert(fprintf(fout, "\n") > 0);
        }
    }
    assert(fprintf(fout, "\n\n") > 0);

    assert(fprintf(fout, "COSTS:\n") > 0);
    for (Eigen::Index j = 0; j < tableau->getN(); j++) {
        assert(fprintf(fout, "%s (x%-5lu)  ", getFormattedDouble(tableau->redCostsRow[j]).c_str(), j + 1) > 0);
        if ((j + 1) % 3 == 0) {
            assert(fprintf(fout, "\n") > 0);
        }
    }
    assert(fprintf(fout, "\n") > 0);

    assert(fclose(fout) == 0);
}

void TableauFilesManager::printTableau(const string& description, const string& title, const string& filename) {
    string caption;
#ifdef DEBUG
    assert(tableau->isValid());
#endif

    if (tableau->isDualFeasible()) {
        caption = "TABLEAU dual feasible (variables >= equations, base and costs >= 0)";
    } else if (tableau->isFeasible()) {
        caption = "TABLEAU feasible (variables >= equations but not dual feasible)";
    } else {
        caption = "TABLEAU infeasible (no variables >= equations rows linearly dependents)";
    }

    if (tableau->getRows() > 300 || tableau->getCols() > 300) {
        printTableauShort(caption, description, title, filename);
        return;
    }

    FILE *fout;
    if (filename == "") {
        fout = fopen(getNextFilename().c_str(), "w");
    } else {
        fout = fopen(filename.c_str(), "w");
    }
    assert(fout != NULL);

    assert(fprintf(fout, "%s\n\n\n", caption.c_str()) > 0);

    if (title != "") {
        assert(fprintf(fout, "%s\n\n", title.c_str()) > 0);
    }

    assert(description != "");
    assert(fprintf(fout, "%s", description.c_str()) > 0);
    assert(fprintf(fout, "\n\n") > 0);

    assert(fprintf(fout, "Tableau rows x cols: %lu x %lu\n\n", tableau->getRows(), tableau->getCols()) > 0);

    assert(fprintf(fout, "OBJECTIVE FUNCTION VALUE:\n") > 0);
    assert(fprintf(fout, "%s\n\n", getFormattedComplex(-tableau->objFunc).c_str()) > 0);

    if (tableau->getCols() > 0) {
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
    assert(fprintf(fout, "    %s", getFormattedComplex(tableau->objFunc).c_str()) > 0);
    for (Eigen::Index j = 0; j < tableau->getN(); j++) {
        assert(fprintf(fout, "    %s", getFormattedDouble(tableau->redCostsRow[j]).c_str()) > 0);
    }

    for (Eigen::Index i = 0; i < tableau->getM(); i++) {
        if (tableau->varInBaseCol[i] == 0) {
            assert(fprintf(fout, "\n?     ") > 0);
        } else {
            assert(fprintf(fout, "\nx%-5lu", tableau->varInBaseCol[i]) > 0);
        }
        assert(fprintf(fout, "    %s", getFormattedComplex(tableau->knownTermsCol[i]).c_str()) > 0);
        for (Eigen::Index j = 0; j < tableau->getN(); j++) {
            assert(fprintf(fout, "    %s", getFormattedDouble(tableau->coeffMatrix(i, j)).c_str()) > 0);
        }
    }
    assert(fprintf(fout, "\n") > 0);

    assert(fclose(fout) == 0);
}
