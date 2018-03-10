#ifndef FILE_H
#define FILE_H


#include "Exceptions.h"

#include <string>
#include <fstream>

class File {

public:
    File(std::string filePath);
    ~File();

    void openWrite(std::ios::openmode mode) throw(FileNotWritableException);

    std::shared_ptr<std::ofstream> getFile() const;

    void close();

private:
    std::string filePath;
    std::shared_ptr<std::ofstream> file;
};


#endif //FILE_H
