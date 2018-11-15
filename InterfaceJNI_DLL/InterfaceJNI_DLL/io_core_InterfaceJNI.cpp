#include <jni.h>
#include <iostream>
#include <vector>
#include <string>
#include "PlatheaDLL.h"
#include "io_core_InterfaceJNI.h"
using namespace std;


JNIEXPORT jint JNICALL Java_io_core_InterfaceJNI_loadConfigurationFile(JNIEnv *env, jobject thisObj, jstring jdir) {
	const char *dir = env->GetStringUTFChars(jdir, 0);
	system_loadconfigurationfile((char*)dir);
	return 0;
}


JNIEXPORT jint JNICALL Java_io_core_InterfaceJNI_internalCalibration(JNIEnv *env, jobject thisObj, jstring jdir, jint jmask) {
	const char *dir = env->GetStringUTFChars(jdir, 0);
	return 0;
}

JNIEXPORT jstring JNICALL Java_io_core_InterfaceJNI_getRoomInfo(JNIEnv *env, jobject thisObj) {
	char buf[10] = "Hello";
	jstring str = env->NewStringUTF(buf);

	return str;
}
