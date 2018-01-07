//
// Created by Neconspictor on 02.01.2018.
//

#ifndef BACHELOR_SEMINAR_FILE_H
#define BACHELOR_SEMINAR_FILE_H


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


#endif //BACHELOR_SEMINAR_FILE_H
