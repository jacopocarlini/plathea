#include <jni.h>
#include <iostream>
#include <vector>
#include <string>
#include "PlatheaDLL.h"
#include "io_core_InterfaceJNI.h"
using namespace std;


JNIEXPORT jint JNICALL Java_io_core_InterfaceJNI_loadConfigurationFile(JNIEnv *env, jobject thisObj, jstring jdir) {
	const char *dir = env->GetStringUTFChars(jdir, 0);
	printf("dir:%s\n",dir);
	system_loadconfigurationfile((char*)dir);
	return 0;
}


JNIEXPORT jint JNICALL Java_io_core_InterfaceJNI_internalCalibration(JNIEnv *env, jobject thisObj, jstring jdir, jint jmask) {
	const char *dir = env->GetStringUTFChars(jdir, 0);
	int mask = (int)jmask;
	printf("dir:%s\n",dir);
	calibration_internalcalibration_load((char*)dir, mask);
	return 0;
}

JNIEXPORT jint JNICALL Java_io_core_InterfaceJNI_externalCalibration(JNIEnv *env, jobject thisObj, jstring jdir) {
	const char *dir = env->GetStringUTFChars(jdir, 0);
	printf("dir:%s\n", dir);
	calibration_loadexternalcalibrationdata((char*)dir);
	return 0;
}

JNIEXPORT jint JNICALL Java_io_core_InterfaceJNI_selectSVMclassifier(JNIEnv *env, jobject thisObj, jstring jdir) {
	const char *dir = env->GetStringUTFChars(jdir, 0);
	printf("dir:%s\n", dir);
	localizationengine_selectsvmclassifier((char*)dir);
	return 0;
}


JNIEXPORT jint JNICALL Java_io_core_InterfaceJNI_initializeSystem
(JNIEnv *env, jobject thisObj, jstring jusername, jstring jpassword, jstring jtype, jstring jresolution, 
	jint jfps, jstring jcameraModel, jstring jIPAddress1, jint jport1, jstring jIPAddress2, jint jport2){

	const char *username = env->GetStringUTFChars(jusername, 0);
	const char *password = env->GetStringUTFChars(jpassword, 0);
	const char *type = env->GetStringUTFChars(jtype, 0);
	const char *resolution = env->GetStringUTFChars(jresolution, 0);
	const char *cameraModel = env->GetStringUTFChars(jcameraModel, 0);
	const char *IPAddress1 = env->GetStringUTFChars(jIPAddress1, 0);
	const char *IPAddress2 = env->GetStringUTFChars(jIPAddress2, 0);
	system_initializesystem(username, password, type, resolution, jfps, cameraModel,
		IPAddress1, jport1, IPAddress2, jport2);
	return 0;
}


JNIEXPORT jint JNICALL Java_io_core_InterfaceJNI_startLocalizationEngine
(JNIEnv *env, jobject thisObj, jboolean withoutTracking, jboolean saveTracksToFile, jstring jdir) {
	const char *dir = env->GetStringUTFChars(jdir, 0);
	printf("dir:%s\n", dir);
	localizationengine_startlocalizationengine(withoutTracking, saveTracksToFile, dir);
	return 0;
}

JNIEXPORT jint JNICALL Java_io_core_InterfaceJNI_platheaRecorder
(JNIEnv *env, jobject thisObj) {
	test_plathearecorder();
	return 0;
}

JNIEXPORT jint JNICALL Java_io_core_InterfaceJNI_platheaRecorderStart
(JNIEnv *env, jobject thisObj, jstring jdir) {
	const char *dir = env->GetStringUTFChars(jdir, 0);
	printf("dir:%s\n", dir);
	test_plathearecorder_start(dir);
	return 0;
}

JNIEXPORT jstring JNICALL Java_io_core_InterfaceJNI_getRoomInfo(JNIEnv *env, jobject thisObj) {
	char buf[10] = "Hello";
	jstring str = env->NewStringUTF(buf);
	std::vector<TrackedObject*> tracked = getTrackedPeople();
	printf("\ntracked: %d\n", tracked.size());
	return str;
}
