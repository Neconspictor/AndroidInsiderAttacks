#include "File.h"
#include "Util.h"
#include <fstream>

using namespace util;
using namespace std;

#define LOG_TAG "EVIL_LIB::FILE"

File::File(std::string filePath) {
    this->filePath = move(filePath);
    file = shared_ptr<ofstream>();
}

File::~File() {
    logE(LOG_TAG, "File::~File called!");
    close();
}

void File::close() {
    logE(LOG_TAG, "File::close() called!");
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