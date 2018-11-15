/***************************************************************************
	PLaTHEA: People Localization and Tracking for HomE Automation
	Copyright (C) 2014 Francesco Leotta
	
	This file is part of PLaTHEA
	PLaTHEA is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser Public License as published by
	the Free Software Foundation, version 3 of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	
	GNU Lesser Public License for more details.

	You should have received a copy of the GNU Lesser Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/

#ifndef NETWORK_CAMERA
#define NETWORK_CAMERA

#include "CommonHeader.h"
#include <LeoWindowsThread.h>
#include <LeoColorSpaces.h>
#include <LeoWindowsNetwork.h>
#include "VideoRecorder.h"

enum RaisableEvent {NONE_EVENT, IMAGE_READY, STEREO_IMAGE_READY, DISPARITY_MAP_READY, IMAGE_STARTED};

/*TODO: We could define asynchronous and synchronous camera.*/

enum CameraSide {LEFT_SIDE_CAMERA, RIGHT_SIDE_CAMERA};

typedef void (* FrameRateNotificationFunction) (float currentFrameRate);

struct InitializationStruct {
	struct AcquisitionProperties {
		std::string cameraModel;
		int cameraAvailableOptions;
		std::string resolution;
		int fps;
		CvSize GetResolution(int * height, int * width, bool smallImg) const {
			int height2 = 240, width2 = 320;
			if (!smallImg) {
				sscanf_s(resolution.c_str(), "%dx%d", &width2, &height2);
			}
			if (height != NULL && width != NULL) {
				*height = height2;
				*width = width2;
			}
			return cvSize(width2, height2);
		}
	} acquisitionProperties;
	std::string componentName;
	struct Authentication {
		enum AuthenticationType {NONE_AUTH, OPEN_HTTP} type;
		std::string username, password;
	} authentication;
	struct TCDData {
		std::string IPAddress;
		unsigned short portNumber;
	} tcddata;
};

class AcquisitionCamera: public Thread, public EventRaiser {
protected:
	DWORD lastTimestampStart;
	DWORD lastTimestampEnd;
	InitializationStruct is;
	IplImage *image;
	int maximumWait;
	float currentAvgDelay;
public:
	virtual IplImage * GetImage(DWORD* lastTimeStampStart = NULL, DWORD* lastTimeStampEnd = NULL, float *currentAvgDelay = NULL);
	RWLock ImageLock;
	AcquisitionCamera(const InitializationStruct &is);
	virtual ~AcquisitionCamera();
	FrameRateNotificationFunction notifyFunction;

	HANDLE hSemaphore;
	bool IncreamentForcedWaitingSemaphore();
	bool DecrementForcedWaitingSemaphore();
};

class AcquisitionCameraFactory {
public:
	typedef AcquisitionCamera * (*AcquisitionCameraInstantiator)(const InitializationStruct &is);
	struct CameraDescription {
		enum CameraOptions {CAMERA_OPTION_NETWORK = 1, CAMERA_OPTION_AUTHENTICATION = 2,
			CAMERA_OPTION_RESOLUTION = 4, CAMERA_OPTION_FPS = 8, VIRTUAL_CAMERA = 16};
		std::string supportedResolutions;
		AcquisitionCameraInstantiator instantiator;
		int availableOptions;
		CameraDescription(std::string supportedResolutions, AcquisitionCameraInstantiator instantiator, int availableOptions) {
			this->supportedResolutions = supportedResolutions;
			this->instantiator = instantiator;
			this->availableOptions = availableOptions;
		}
		CameraDescription() {}
	};
	typedef std::map<std::string, CameraDescription> AcquisitionCameraInstantiatorMap;
private:
	static AcquisitionCameraInstantiatorMap m_AcquisitionCameraInstantiatorMap;
public:
	template <class T> static AcquisitionCamera * GenericAcquisitionCameraInstantiator (const InitializationStruct &is) {
		return (AcquisitionCamera *) new T(is);
	}
	static AcquisitionCamera * CreateNewInstance(std::string model, const InitializationStruct &is);
	static AcquisitionCameraInstantiatorMap &GetRegisteredCameras() {
		return m_AcquisitionCameraInstantiatorMap;
	}
};

#define ACQUISITIONCAMERA_CLASS_ENTRY(symbolicName, supportedResolutions, classID, options) \
	AcquisitionCameraFactory::AcquisitionCameraInstantiatorMap::value_type(symbolicName, \
	AcquisitionCameraFactory::CameraDescription(supportedResolutions, AcquisitionCameraFactory::GenericAcquisitionCameraInstantiator< classID >, options))

//CvCapture Camera Models Definition

class OpenCvCaptureCamera: public AcquisitionCamera {
protected:
	CvCapture *theCapture;
public:
	OpenCvCaptureCamera(const InitializationStruct &is): AcquisitionCamera(is) {}
	virtual ~OpenCvCaptureCamera() {}
	virtual void Run(void *param = NULL) {}
	virtual bool StopPreprocedure() {return true;}
	virtual bool Start() {return true;}
	virtual bool Stop() {return true;}
	virtual IplImage * GetImage(DWORD* lastTimeStamp = NULL) {
		return cvQueryFrame(theCapture);
	}
	CvCapture **GetCapture() {
		return &theCapture;
	}
};

class NullAcquisitionCamera: public OpenCvCaptureCamera {
public:
	NullAcquisitionCamera(const InitializationStruct &is): OpenCvCaptureCamera(is) {}
	virtual ~NullAcquisitionCamera() {}
};

class FireWireCamera: public OpenCvCaptureCamera {
public:
	FireWireCamera(const InitializationStruct &is);
	virtual ~FireWireCamera();
};

//Network Camera Models Definition

class NetworkCamera: public AcquisitionCamera {
public:
	NetworkCamera(const InitializationStruct &is, std::string cameraURL);
	virtual ~NetworkCamera();
	DWORD initialTimestamp;
private:
	char * HTTPRequestCommand;
	bool continueRunning;
	//This method returns the URL with the authorization property
	char *GetHTTPRequestCommand(std::string cameraURL);
	virtual std::string GetURL(const InitializationStruct &is) = 0;

	float onlineAcquisitionRate;
protected:
	sockaddr_in cameraInfo;
	virtual void Run(void *param = NULL);
	virtual bool StopPreprocedure();
};

class Axis207: public NetworkCamera  {
private:
	std::string GetURL(const InitializationStruct &is);
public:
	Axis207(const InitializationStruct &is) : NetworkCamera(is, GetURL(is)) {}
};

class VivotekIP7160: public NetworkCamera {
private:
	std::string GetURL(const InitializationStruct &is);
public:
	VivotekIP7160(const InitializationStruct &is) : NetworkCamera(is, GetURL(is)) {}
};

enum StereoRigStatus {NOT_STARTED, THREAD_INITIALIZING, THREAD_LOADING,	STOPPED_FOR_ERROR, STOPPED_BY_USER};

class StereoRig: public Thread, public EventRaiser {
public:
	struct StereoTimestampsStruct {
		DWORD cameraTimeStamps[2];
		DWORD stereoTimeStamp;
	};
	static int vDisplacement;
	static int hDisplacement;
private:
	//Modify the current status of the StereoRig, only for internal use
	void SetStatus(StereoRigStatus newStatus);

	InitializationStruct::AcquisitionProperties ap;
	AcquisitionCamera * CameraPtr[2];

	HANDLE videoLoadedEvent, stopPlaybackEvent;

	StereoRigStatus currStatus;
	IplImage *currStereoImage[2], *currStereoBigImage[2]; StereoTimestampsStruct stereoTimestamps;

	bool UseCalibrationData;

	HANDLE closeStereoRigEvent;
	
	HANDLE hOffsetFile;
	bool recorderMode;
	BufferedVideoRecorder *bvr;

	void iApplyCalibrationTransformation(IplImage *leftResized, IplImage *rightResized);

	bool onRequestMode;
	HANDLE onRequestFrameEvent;

	float onlineAcquisitionRate;
public:
	//Create a new Stereo Rig with the specified parameters
	StereoRig(const InitializationStruct::AcquisitionProperties &ap, AcquisitionCamera *leftCamera, AcquisitionCamera *rightCamera, const bool ucd);
	~StereoRig();
	//Return the current status of the StereoRig
	StereoRigStatus GetStatus ();
	RWLock StereoLock;
	StereoTimestampsStruct GetStereoImages(IplImage ** left, IplImage ** right, bool bigImage);
	void SetFrameAsConsumed();
	AcquisitionCamera *GetAcquisitionCamera(CameraSide cs) {
		return CameraPtr[cs];
	}
	const InitializationStruct::AcquisitionProperties &GetAcquisitionProperties() {
		return ap;
	}
	bool StartRecorderMode();
	bool StopRecorderMode();
	bool SaveRecordingToDirectory(const char* baseDirectory);
	bool StartPlaybackMode(const char* baseDirectory, bool onRequestMode);
	bool StopPlaybackMode();
	bool GetRecorderMode() {
		return recorderMode;
	}
	void RequestMoreFrames();
	FrameRateNotificationFunction notifyFunction;
protected:
	virtual void Run(void *param = NULL);
	virtual bool StopPreprocedure();
};

#endif