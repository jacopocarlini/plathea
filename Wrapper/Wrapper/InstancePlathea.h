/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class InstancePlathea */

#ifndef _Included_InstancePlathea
#define _Included_InstancePlathea
#ifdef __cplusplus
extern "C" {
#endif
	/*
	* Class:     InstancePlathea
	* Method:    loadConfigurationFile
	* Signature: (ILjava/lang/String;)I
	*/
	JNIEXPORT jint JNICALL Java_InstancePlathea_loadConfigurationFile
	(JNIEnv *, jclass, jint, jstring);

	/*
	* Class:     InstancePlathea
	* Method:    internalCalibration
	* Signature: (Ljava/lang/String;I)I
	*/
	JNIEXPORT jint JNICALL Java_InstancePlathea_internalCalibration
	(JNIEnv *, jclass, jstring, jint);

	/*
	* Class:     InstancePlathea
	* Method:    externalCalibration
	* Signature: (Ljava/lang/String;)I
	*/
	JNIEXPORT jint JNICALL Java_InstancePlathea_externalCalibration
	(JNIEnv *, jclass, jstring);

	/*
	* Class:     InstancePlathea
	* Method:    selectSVMclassifier
	* Signature: (Ljava/lang/String;)I
	*/
	JNIEXPORT jint JNICALL Java_InstancePlathea_selectSVMclassifier
	(JNIEnv *, jclass, jstring);

	/*
	* Class:     InstancePlathea
	* Method:    initializeSystem
	* Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;ILjava/lang/String;I)I
	*/
	JNIEXPORT jint JNICALL Java_InstancePlathea_initializeSystem
	(JNIEnv *, jclass, jstring, jstring, jstring, jstring, jint, jstring, jstring, jint, jstring, jint);

	/*
	* Class:     InstancePlathea
	* Method:    startLocalizationEngine
	* Signature: (ZZLjava/lang/String;)I
	*/
	JNIEXPORT jint JNICALL Java_InstancePlathea_startLocalizationEngine
	(JNIEnv *, jclass, jboolean, jboolean, jstring);

	/*
	* Class:     InstancePlathea
	* Method:    platheaPlayer
	* Signature: ()I
	*/
	JNIEXPORT jint JNICALL Java_InstancePlathea_platheaPlayer
	(JNIEnv *, jclass);

	/*
	* Class:     InstancePlathea
	* Method:    platheaPlayerStart
	* Signature: (Ljava/lang/String;)I
	*/
	JNIEXPORT jint JNICALL Java_InstancePlathea_platheaPlayerStart
	(JNIEnv *, jclass, jstring);

	/*
	* Class:     InstancePlathea
	* Method:    platheaPlayerStop
	* Signature: ()I
	*/
	JNIEXPORT jint JNICALL Java_InstancePlathea_platheaPlayerStop
	(JNIEnv *, jclass);

	/*
	* Class:     InstancePlathea
	* Method:    getTrackedPeople
	* Signature: ()[LInstancePlathea/TrackedPerson;
	*/
	JNIEXPORT jobjectArray JNICALL Java_InstancePlathea_getTrackedPeople
	(JNIEnv *, jclass);

	/*
	* Class:     InstancePlathea
	* Method:    getFrame
	* Signature: (I)[B
	*/
	JNIEXPORT jbyteArray JNICALL Java_InstancePlathea_getFrame
	(JNIEnv *, jclass, jint);

#ifdef __cplusplus
}
#endif
#endif
