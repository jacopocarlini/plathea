#include <jni.h>
#include <iostream>
#include <vector>
#include <string>
#include <list>
#include "PlatheaDLL.h"
#include "InstancePlathea.h"
using namespace std;


JNIEXPORT void JNICALL Java_InstancePlathea_hello
(JNIEnv *env , jobject thisobj) {
	printf("hello\n");
}

JNIEXPORT jint JNICALL Java_InstancePlathea_loadConfigurationFile(JNIEnv *env, jobject thisObj, jint jroomID, jstring jdir) {
	const char *dir = env->GetStringUTFChars(jdir, 0);
	printf("dir:%s\n", dir);
	system_loadconfigurationfile(jroomID, (char*)dir);
	return 0;
}


JNIEXPORT jint JNICALL Java_InstancePlathea_internalCalibration(JNIEnv *env, jobject thisObj, jstring jdir, jint jmask) {
	const char *dir = env->GetStringUTFChars(jdir, 0);
	int mask = (int)jmask;
	printf("dir:%s\n", dir);
	calibration_internalcalibration_load((char*)dir, mask);
	return 0;
}

JNIEXPORT jint JNICALL Java_InstancePlathea_externalCalibration(JNIEnv *env, jobject thisObj, jstring jdir) {
	const char *dir = env->GetStringUTFChars(jdir, 0);
	printf("dir:%s\n", dir);
	calibration_loadexternalcalibrationdata((char*)dir);
	return 0;
}

JNIEXPORT jint JNICALL Java_InstancePlathea_selectSVMclassifier(JNIEnv *env, jobject thisObj, jstring jdir) {
	const char *dir = env->GetStringUTFChars(jdir, 0);
	printf("dir:%s\n", dir);
	localizationengine_selectsvmclassifier((char*)dir);
	return 0;
}


JNIEXPORT jint JNICALL Java_InstancePlathea_initializeSystem
(JNIEnv *env, jobject thisObj, jstring jusername, jstring jpassword, jstring jtype, jstring jresolution,
	jint jfps, jstring jcameraModel, jstring jIPAddress1, jint jport1, jstring jIPAddress2, jint jport2) {

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


JNIEXPORT jint JNICALL Java_InstancePlathea_startLocalizationEngine
(JNIEnv *env, jobject thisObj, jboolean withoutTracking, jboolean saveTracksToFile, jstring jdir) {
	const char *dir = env->GetStringUTFChars(jdir, 0);
	printf("dir:%s\n", dir);
	localizationengine_startlocalizationengine(withoutTracking, saveTracksToFile, dir);
	return 0;
}

JNIEXPORT jint JNICALL Java_InstancePlathea_platheaPlayer
(JNIEnv *env, jobject thisObj) {
	test_platheaplayer();
	return 0;
}

JNIEXPORT jint JNICALL Java_InstancePlathea_platheaPlayerStart
(JNIEnv *env, jobject thisObj, jstring jdir) {
	const char *dir = env->GetStringUTFChars(jdir, 0);
	printf("dir:%s\n", dir);
	test_platheaplayer_start(dir);
	return 0;
}

JNIEXPORT jint JNICALL Java_InstancePlathea_platheaPlayerStop(JNIEnv *env, jobject thisObj) {
	test_plathearecorder_stop();
	return 0;
}


JNIEXPORT jobjectArray JNICALL Java_InstancePlathea_getTrackedPeople(JNIEnv *env, jobject thisObj) {
	printf("start jni\n");
	std::vector<TrackedObject*> tracked = getTrackedPeople();
	printf("ok dll\n");
	//std::vector<TrackedObject*> tracked;
	jclass javaLocalClass = env->FindClass("InstancePlathea$TrackedPerson");

	if (javaLocalClass == NULL) {
		printf("Find Class Failed.\n");
		return NULL;
	}
	else {
		printf("Found class.\n");
	}

	jclass javaGlobalClass = reinterpret_cast<jclass>(env->NewGlobalRef(javaLocalClass));

	// info: last argument is Java method signature
	//jmethodID javaConstructor = env->GetMethodID(javaGlobalClass, "<init>", "(Lio/core/InterfaceJNI;Ljava/lang/String;IIIILjava/lang/String;)V");
	jmethodID javaConstructor = env->GetMethodID(javaGlobalClass, "<init>", "(LInstancePlathea;)V");
	jmethodID setname = env->GetMethodID(javaGlobalClass, "setName", "(Ljava/lang/String;)V");
	jmethodID settype = env->GetMethodID(javaGlobalClass, "setType", "(Ljava/lang/String;)V");
	jmethodID setints = env->GetMethodID(javaGlobalClass, "setIntegers", "(IIII)V");


	if (javaConstructor == NULL) {
		printf("Find method Failed.\n");
		return NULL;
	}
	else {
		printf("Found method.\n");
	}
	if (setname == NULL) {
		printf("Find method Failed.\n");
		return NULL;
	}
	if (settype == NULL) {
		printf("Find method Failed.\n");
		return NULL;
	}

	jobjectArray ret = env->NewObjectArray(tracked.size(), javaLocalClass, NULL);
	//jobjectArray ret = env->NewObjectArray(1, javaGlobalClass, NULL);

	//jobject TrackedPersonObject;
	//printf(".\n");
	//TrackedPersonObject = env->NewObject(javaLocalClass, javaConstructor, 0,0,0,0);
	//env->CallObjectMethod(TrackedPersonObject, setname, jstr);
	//env->CallObjectMethod(TrackedPersonObject, settype, jstr);



	for (int i = 0; i < tracked.size(); i++) {
		printf("%s\n", (tracked[i]->name));
		printf("creo object\n");

		printf("%s\n", tracked[i]->name);
		printf("%d\n", tracked[i]->nameID);
		printf("%d\n", tracked[i]->X);
		printf("%d\n", tracked[i]->Y);
		printf("%d\n", tracked[i]->ID);

		int nameID = tracked[i]->nameID;
		int X = tracked[i]->X;
		int Y = tracked[i]->Y;
		int ID = tracked[i]->ID;
		jobject TrackedPersonObject;
		printf("newobject\n");
		// non so perchè ma se chiamo il costruttore con -1 non funziona
		// per il momento tutti tutti hanno nameID==-1 
		TrackedPersonObject = env->NewObject(javaLocalClass, javaConstructor);
		//, (tracked[i]->type== TRACKED)? env->NewStringUTF("Tracked") : env->NewStringUTF("Lost"));
		printf("creo string\n");
		env->CallObjectMethod(TrackedPersonObject, setints, nameID, X, Y, ID);

		const char* name;
		if (tracked[i]->nameID == -1) {
			printf("%d\n", tracked[i]->nameID);
			name = "-";
		}
		else
			name = ((tracked[i])->name);
		printf("prima lettera %c\n", name[0]);
		jstring jname = env->NewStringUTF(name);
		const char* type = (tracked[i]->type == TRACKED) ? "Tracked" : "Lost";
		jstring jtype = env->NewStringUTF(type);

		printf("set string\n");
		env->CallObjectMethod(TrackedPersonObject, setname, jname);
		env->CallObjectMethod(TrackedPersonObject, settype, jtype);
		printf("aggiungo all'array\n");
		env->SetObjectArrayElement(ret, i, TrackedPersonObject);
	}

	//env->SetObjectArrayElement(ret, 0, TrackedPersonObject);
	printf("ok jni\n");
	return ret;




}


JNIEXPORT jbyteArray JNICALL Java_InstancePlathea_getFrame(JNIEnv *env, jobject thisObj, jint id) {
	StreamsVideo* streams = getStreamsVideo();

	//list to array
	std::vector<unsigned char> bytes;
	//printf("id=%d\n",id);

	if (id == 0)
		bytes = streams->getFrame();
	if (id == 1) {
		bytes = streams->getFrameBackground();
		printf("getFrameBackground bytes %d\n", bytes.size());
	}
	if (id == 2)
		bytes = streams->getFrameRawforeground();
	if (id == 3)
		bytes = streams->getFrameForeground();
	if (id == 4)
		bytes = streams->getFrameDisparity();
	if (id == 5)
		bytes = streams->getFrameEdge();
	if (id == 6)
		bytes = streams->getFrameOccupancy();
	if (id == 7)
		bytes = streams->getFrameHeight();

	//printf("size della lista %d\n",bytes.size());
	//bytes = streams->getFrame();
	if (bytes.size() == 0) return env->NewByteArray(0);
	//unsigned char *arr = new unsigned char[bytes.size()];
	//printf("copy\n");
	//copy(bytes.begin(), bytes.end(), arr);
	unsigned char* arr = &bytes[0];
	//array to jbyteArray
	//printf("new jarray\n");
	jbyteArray jarr = env->NewByteArray(bytes.size());
	//printf("set array\n");
	env->SetByteArrayRegion(jarr, 0, bytes.size(), reinterpret_cast<jbyte*>(arr));
	//printf("done jni\n");
	return jarr;
}