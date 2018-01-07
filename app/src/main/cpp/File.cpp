//
// Created by Neconspictor on 02.01.2018.
//

#include "File.h"
#include "Util.h"
#include <fstream>

File::File(std::string&& filePath) {
    this->filePath = std::move(filePath);
    file = std::shared_ptr<std::ofstream>();
}

File::~File() {
    LOGE("File::~File called!");
    close();
}

void File::close() {
    LOGE("File::close() called!");
    std::ofstream* filePtr = file.get();
    if (filePtr) {
        filePtr->close();
    }
}

void File::openWrite(std::ios::openmode mode) throw(FileNotWritableException){
    close();
    file.reset(new std::ofstream(filePath.c_str(), mode));
    std::ofstream* fs = file.get();
    if (!fs->is_open()) {
        throw FileNotWritableException("Couldn't open file for writing: " + filePath);
    }
}

std::shared_ptr<std::ofstream> File::getFile() const {
    return file;
}