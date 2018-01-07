//
// Created by necon on 07.01.2018.
//

#ifndef REPO_EVILMODULEPROVIDER_H
#define REPO_EVILMODULEPROVIDER_H

#include <string>
#include "Exceptions.h"
#include "Service.h"


class EvilModuleProvider {

public:


    /**
     * Constructs a new EvilModuleProvider object and initializes it with a given service.
     */
    EvilModuleProvider(Service service);

    /**
     * Default destructor.
     */
    ~EvilModuleProvider();

    /**
     * Provides the file path to the evil java module.
     * @throws NoSuchFileException - if the evil java module wasn't successfully downloaded before
     * or doesn't exists.
     * @return the file path to the evil java module.
     */
    std::string provideEvilModuleFilePath() throw(NoSuchFileException);

    /**
     * Downloads the evil java module and provides its file path.
     * @param env the current JNI environment.
     * @throws DownloadException - if the evil java module couldn't be downloaded.
     *
     * @return The file path to the evil java module.
     *
     */
    std::string downloadEvilModule(JNIEnv *env) throw(DownloadException);

private:

    Service service;
    std::string filePath;
    bool fileDownloaded;

};


#endif //REPO_EVILMODULEPROVIDER_H
