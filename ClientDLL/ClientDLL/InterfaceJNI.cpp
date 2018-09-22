#include <jni.h>
#include <iostream>
#include "InterfaceJNI.h"
using namespace std;

JNIEXPORT jstring JNICALL Java_io_library_InterfaceJNI_getRoomInfo(JNIEnv *env, jobject thisObj) {
	char buf[10] = "Hello ";
	jstring str = env->NewStringUTF(buf);
	return str;
}
