#include <jni.h>       // JNI header provided by JDK
#include <iostream>    // C++ standard IO header
#include "io_core_InterfaceJNI.h"  // Generated
using namespace std;

// Implementation of the native method sayHello()
JNIEXPORT jint JNICALL Java_io_core_InterfaceJNI_loadConfigurationFile(JNIEnv *env, jobject thisObj, jstring jdir) {
    jint i = 0;
    return i;
}