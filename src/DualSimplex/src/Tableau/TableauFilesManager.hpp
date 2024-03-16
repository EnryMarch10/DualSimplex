#ifndef __TABLEAU_FILES_MANAGER__HPP
#define __TABLEAU_FILES_MANAGER__HPP

#include "Tableau.hpp"
#include <string>

class TableauFilesManager {

public:
    TableauFilesManager(Tableau *const tableau, const char *filePath);
    void readTableau(void);
    const std::string getNextFilename(void);
    void printTableau(const std::string& title, const std::string& description, const std::string& filename = "");
    void printTableauWithCheck(const std::string& title, const std::string& description, const std::string& filename = "");
    void printTableauFeasible(const std::string& description);
    void printTableauInfeasible(const std::string& description);
    ~TableauFilesManager(void) { }

private:
    const char *inFilePath;
    std::string outPartialFilePath;
    unsigned long counter = 1;
    Tableau *tableau;

    void printTableau(const std::string& title, const std::string& description,
                      const std::string& filename, const bool doCheck,
                      const bool isDualFeasible);

};

#endif // __TABLEAU_FILES_MANAGER__HPP
