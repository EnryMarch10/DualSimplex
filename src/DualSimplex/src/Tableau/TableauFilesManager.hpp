#ifndef __TABLEAU_FILES_MANAGER__HPP
#define __TABLEAU_FILES_MANAGER__HPP

#include "Tableau.hpp"
#include <string>

class TableauFilesManager {

public:
    TableauFilesManager(Tableau *const tableau, const char *filePath);
    void readTableau(void);
    const std::string getNextFilename(void);
    void printTableau(const std::string& description, const std::string& title = "", const std::string& filename = "");
    ~TableauFilesManager(void) { }

private:
    const char *inFilePath;
    std::string outPartialFilePath;
    unsigned long counter = 1;
    Tableau *tableau;

    void printTableauShort(const std::string& caption, const std::string& description,
                           const std::string& title, const std::string& filename);

};

#endif // __TABLEAU_FILES_MANAGER__HPP
