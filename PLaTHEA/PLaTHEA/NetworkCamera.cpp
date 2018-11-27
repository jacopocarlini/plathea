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

#include "NetworkCamera.h"
#include "StereoCalibration.h"
#include "ApplicationWorkFlow.h"
#include "Shared.h"

#include <LeoWindowsOpenCV.h>
#include <LeoWindowsNetwork.h>
#include <LeoLibJPEGOpenCV.h>
#include <LeoLog4CPP.h>
#include <LeoWindowsConsole.h>

using namespace std;
using namespace stdext;
using namespace leostorm::logging;

extern SystemInfo *si;

//****** Functions Supporting the different camera models *******//

AcquisitionCamera::AcquisitionCamera(const InitializationStruct &is): Thread(), EventRaiser() {
	this->is = is;
	int height, width;
	is.acquisitionProperties.GetResolution(&height, &width, false);
	image = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	hSemaphore = INVALID_HANDLE_VALUE; maximumWait = 0;
}

AcquisitionCamera::~AcquisitionCamera() {
	cvReleaseImage(&image);
	CloseHandle(hSemaphore);
}

IplImage * AcquisitionCamera::GetImage(DWORD* lastTimestampStart, DWORD* lastTimestampEnd, float *currentAvgDelay) {
	if (lastTimestampStart)
		*lastTimestampStart = this->lastTimestampStart;
	if (lastTimestampEnd)
		*lastTimestampEnd = this->lastTimestampEnd;
	if (currentAvgDelay)
		*currentAvgDelay = this->currentAvgDelay;
	return image;
}

//These functions has to be modified in order to support runtime operations. Per adesso OK perchè l'unico che le usa è StereoRig
bool AcquisitionCamera::IncreamentForcedWaitingSemaphore() {
	if (hSemaphore != INVALID_HANDLE_VALUE)
		CloseHandle(hSemaphore);
	maximumWait++;
	hSemaphore = CreateSemaphore(NULL, maximumWait, maximumWait, NULL);
	return true;
}

bool AcquisitionCamera::DecrementForcedWaitingSemaphore() {
	if (hSemaphore != INVALID_HANDLE_VALUE)
		CloseHandle(hSemaphore);
	maximumWait--;
	hSemaphore = CreateSemaphore(NULL, maximumWait, maximumWait, NULL);
	return true;
}

static AcquisitionCameraFactory::AcquisitionCameraInstantiatorMap::value_type instantiatorMapEntries[] = {
	ACQUISITIONCAMERA_CLASS_ENTRY("Axis 207 Network Camera", "320x240;480x360;640x480", Axis207, AcquisitionCameraFactory::CameraDescription::CAMERA_OPTION_NETWORK | AcquisitionCameraFactory::CameraDescription::CAMERA_OPTION_AUTHENTICATION | AcquisitionCameraFactory::CameraDescription::CAMERA_OPTION_FPS | AcquisitionCameraFactory::CameraDescription::CAMERA_OPTION_RESOLUTION),
	ACQUISITIONCAMERA_CLASS_ENTRY("Vivotek IP7161 Network Camera", "320x240;640x480;800x600;1280x960;1600x1200", VivotekIP7160, AcquisitionCameraFactory::CameraDescription::CAMERA_OPTION_NETWORK | AcquisitionCameraFactory::CameraDescription::CAMERA_OPTION_AUTHENTICATION | AcquisitionCameraFactory::CameraDescription::CAMERA_OPTION_FPS | AcquisitionCameraFactory::CameraDescription::CAMERA_OPTION_RESOLUTION),
	ACQUISITIONCAMERA_CLASS_ENTRY("Virtual Acquisition Camera", "320x240", NullAcquisitionCamera, AcquisitionCameraFactory::CameraDescription::CAMERA_OPTION_RESOLUTION | AcquisitionCameraFactory::CameraDescription::VIRTUAL_CAMERA),
	ACQUISITIONCAMERA_CLASS_ENTRY("FireWire Camera", "1280x960", FireWireCamera, AcquisitionCameraFactory::CameraDescription::CAMERA_OPTION_RESOLUTION | AcquisitionCameraFactory::CameraDescription::CAMERA_OPTION_FPS)
};
AcquisitionCameraFactory::AcquisitionCameraInstantiatorMap AcquisitionCameraFactory::m_AcquisitionCameraInstantiatorMap(instantiatorMapEntries,
	instantiatorMapEntries + sizeof(instantiatorMapEntries)/sizeof(instantiatorMapEntries[0]));

AcquisitionCamera * AcquisitionCameraFactory::CreateNewInstance(std::string model, const InitializationStruct &is) {
	return m_AcquisitionCameraInstantiatorMap[model].instantiator(is);
}

//****** FireWireCamera Implementation ********//
FireWireCamera::FireWireCamera(const InitializationStruct &is): OpenCvCaptureCamera(is) {
	theCapture = cvCaptureFromCAM(int(is.tcddata.portNumber));
	WindowsConsole::writeToLOG(L"Setting acquisition properties %d, %d\r\n",
		cvSetCaptureProperty(theCapture, CV_CAP_PROP_FRAME_WIDTH, 640),
		cvSetCaptureProperty(theCapture, CV_CAP_PROP_FRAME_HEIGHT, 480));
}

FireWireCamera::~FireWireCamera() {
	cvReleaseCapture(&theCapture);
	WindowsConsole::writeToLOG(L"Closing Camera Capture %d\r\n", (int) is.tcddata.portNumber);
}

//****** NetworkCamera Implementation *******//
NetworkCamera::NetworkCamera(const InitializationStruct& is, std::string cameraURL): AcquisitionCamera(is) {
	
	ZeroMemory(&cameraInfo, sizeof(cameraInfo));
	this->cameraInfo.sin_family=AF_INET;
	this->cameraInfo.sin_port=htons(is.tcddata.portNumber);
	this->cameraInfo.sin_addr.S_un.S_addr = inet_addr(is.tcddata.IPAddress.c_str());

	this->HTTPRequestCommand = GetHTTPRequestCommand(cameraURL);

	this->notifyFunction = NULL;
}

NetworkCamera::~NetworkCamera() {
	free(this->HTTPRequestCommand);
}

bool NetworkCamera::StopPreprocedure() {
	continueRunning = false;
	return true;
}

void NetworkCamera::Run(void *param) {
	JPeg2IplImage decompressor; continueRunning = true;
	
	SOCKET cameraSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (cameraSocket == INVALID_SOCKET) {
		Logger::writeToLOG(L"%S: Unable to create socket\r\n", is.componentName.c_str());
		return;
	}

	int result = connect(cameraSocket, (SOCKADDR *) &cameraInfo, sizeof(cameraInfo));
	if (result == SOCKET_ERROR) {
		Logger::writeToLOG(L"%S: Unable to connect to the camera\r\n", is.componentName.c_str());
		return;
	}

	Logger::writeToLOG(L"%S: Network Camera acquisition correctly started\r\n", is.componentName.c_str());

	result = send(cameraSocket, HTTPRequestCommand, (int) strlen(HTTPRequestCommand), 0);

	unordered_map<string, string> imageHeader;
	
	HttpStreamSplitter ss(cameraSocket);
	ss.ReadHeaders(imageHeader);

	char * recvBuffer = NULL;
	unsigned long bufferLength = 0;

	lastTimestampStart = GetTickCount();
	DWORD expectedDelay = 1000 / is.acquisitionProperties.fps;
	currentAvgDelay = (float) expectedDelay;
	float updateLaw = 1.f/float(is.acquisitionProperties.fps);

	//HANDLE hFile = CreateFileA(is.componentName.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	while (continueRunning) {
		initialTimestamp = ss.ReadHeaders(imageHeader);
		RaiseEvent(IMAGE_STARTED);
		unsigned long imageLength = strtoul(imageHeader["Content-Length"].c_str(), NULL, 10);
		
		if (imageLength > bufferLength) {
			if (recvBuffer)
				delete[] recvBuffer;
			recvBuffer = new char[imageLength*2];
			bufferLength = imageLength*2;
		}

		result = recv(cameraSocket, recvBuffer, imageLength, MSG_WAITALL);
		DWORD finalTimestamp = GetTickCount();
		//writeToConsole(L"%ul %d\r\n", imageLength, result);
		
		//The hSemaphore object ensures that every module which need every single frame consumes it before a new frame is acquired
		for (int i = 0; i < maximumWait; i++)
			WaitForSingleObject(hSemaphore, INFINITE);
		ImageLock.AcquireWriteLock();
			bool decompressionResult = decompressor.Decompress((unsigned char *) recvBuffer, imageLength, image);
			currentAvgDelay = ((1.0f - updateLaw) * currentAvgDelay) + (updateLaw * (initialTimestamp - lastTimestampStart));
			onlineAcquisitionRate = 1000.f / currentAvgDelay;
			lastTimestampStart = initialTimestamp;
			lastTimestampEnd = finalTimestamp;
		ImageLock.ReleaseWriteLock();
		if (notifyFunction)
				notifyFunction(onlineAcquisitionRate);

		/*char lineToWrite[1024]; int written = sprintf_s(lineToWrite, 1024, "%lu;%lu\r\n", lastTimestampStart,
			lastTimestampEnd);
		DWORD writtenBytes;
		WriteFile(hFile, lineToWrite, written, &writtenBytes, NULL);*/

		if (!decompressionResult) {
			Logger::writeToLOG(L"Decompression error:\r\n");
			unordered_map<string, string>::const_iterator it = imageHeader.begin();
			while (it != imageHeader.end()) {
				Logger::writeToLOG(L"\t%S: %S\r\n", it->first.c_str(), it->second.c_str());
				it++;
			}
		}

		RetireEvent(IMAGE_STARTED);
		RaiseEvent(IMAGE_READY);
		
		readLine(cameraSocket);
	}
	closesocket(cameraSocket);
	if (recvBuffer)
		delete[] recvBuffer;
	//CloseHandle(hFile);
}

char* NetworkCamera::GetHTTPRequestCommand(std::string cameraURL) {
	char * res = new char[512];
	
	char *authString = (is.authentication.type == InitializationStruct::Authentication::OPEN_HTTP ? createHttpSimpleAuthentication(is.authentication.username.c_str(), is.authentication.password.c_str()) : NULL);

	sprintf_s(res, 512, "GET %s HTTP/1.0\r\n", cameraURL.c_str());

	if (authString) {
		sprintf_s(res, 512, "%s%s", res, authString);
		delete[] authString;
	}

	sprintf_s(res, 512, "%s\r\n", res);
	
	return res;
}

std::string Axis207::GetURL(const InitializationStruct &is) {
	char requestBuffer[512];
	sprintf_s(requestBuffer, 512, "/axis-cgi/mjpg/video.cgi?fps=%i&resolution=%s&compression=5",
		is.acquisitionProperties.fps, is.acquisitionProperties.resolution.c_str());
	return requestBuffer;
}

std::string VivotekIP7160::GetURL(const InitializationStruct &is) {
	sockaddr_in tempAddr;
	ZeroMemory(&tempAddr, sizeof(tempAddr));
	tempAddr.sin_family=AF_INET;
	tempAddr.sin_port=htons(is.tcddata.portNumber);
	tempAddr.sin_addr.S_un.S_addr = inet_addr(is.tcddata.IPAddress.c_str());

	char replyBuffer[1024], requestBuffer[512];
	sprintf_s(requestBuffer, 512, "GET /cgi-bin/admin/setparam.cgi?videoin_c0_s0_resolution=%s&videoin_c0_s0_mjpeg_maxframe=%d&videoin_c0_s0_mjpeg_quant=5&videoin_c0_s0_mjpeg_qvalue=10&update=1 HTTP/1.0\r\n",
		is.acquisitionProperties.resolution.c_str(), is.acquisitionProperties.fps);
	if (is.authentication.type == InitializationStruct::Authentication::OPEN_HTTP) {
		char *authString = createHttpSimpleAuthentication(is.authentication.username.c_str(), is.authentication.password.c_str());
		sprintf_s(requestBuffer, 512, "%s%s", requestBuffer, authString);
	}
	sprintf_s(requestBuffer, 512, "%s\r\n", requestBuffer);
	TCPRequestAndReply(&tempAddr, requestBuffer, replyBuffer, 1024);
	Logger::writeToLOG(L"%S#############%S\r\n", requestBuffer, replyBuffer);
	sprintf_s(requestBuffer, 512, "/video.mjpg");
	return requestBuffer;
}

//****** StereoRig Implementation *******//

int StereoRig::vDisplacement = 0;
int StereoRig::hDisplacement = 0;

StereoRig::StereoRig(const InitializationStruct::AcquisitionProperties& ap, AcquisitionCamera *leftCamera, AcquisitionCamera *rightCamera, bool ucd): Thread(), EventRaiser() {
	this->ap = ap;

	this->currStatus = NOT_STARTED;

	this->CameraPtr[LEFT_SIDE_CAMERA] = leftCamera;
	this->CameraPtr[RIGHT_SIDE_CAMERA] = rightCamera;

	int height, width;
	ap.GetResolution(&height, &width, true);
	this->currStereoImage[LEFT_SIDE_CAMERA] = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	this->currStereoImage[RIGHT_SIDE_CAMERA] = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);

	ap.GetResolution(&height, &width, false);
	this->currStereoBigImage[LEFT_SIDE_CAMERA] = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	this->currStereoBigImage[RIGHT_SIDE_CAMERA] = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);

	this->UseCalibrationData = ucd && StereoCalibration::GetInstance()->IsComplete();
	
	this->closeStereoRigEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	this->recorderMode = false;

	this->videoLoadedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	this->stopPlaybackEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	this->onRequestMode = false;
	this->notifyFunction = NULL;
}

StereoRig::~StereoRig() {
	cvReleaseImage(&currStereoImage[LEFT_SIDE_CAMERA]);
	cvReleaseImage(&currStereoImage[RIGHT_SIDE_CAMERA]);
	cvReleaseImage(&currStereoBigImage[LEFT_SIDE_CAMERA]);
	cvReleaseImage(&currStereoBigImage[RIGHT_SIDE_CAMERA]);
	wchar_t platheaTempDir[_MAX_PATH]; GetPLaTHEATempPath(platheaTempDir, _MAX_PATH);
	char sourceFileName[_MAX_PATH]; sprintf_s(sourceFileName, "%S\\left.avi", platheaTempDir);
	DeleteFileA(sourceFileName);
	sprintf_s(sourceFileName, "%S\\right.avi", platheaTempDir);
	DeleteFileA(sourceFileName);

	CloseHandle(videoLoadedEvent);
	CloseHandle(stopPlaybackEvent);
}

bool StereoRig::StopPreprocedure() {
	this->currStatus = STOPPED_BY_USER;
	SetEvent(closeStereoRigEvent);
	return true;
}

void StereoRig::iApplyCalibrationTransformation(IplImage *leftResized, IplImage *rightResized) {
	//if (hDisplacement == 0) hDisplacement = -3; //ONLY FOR STRANGE CASA AGEVOLE TEST
	IplImage *rightFinalDestination = (vDisplacement == 0 && hDisplacement == 0 ? currStereoImage[RIGHT_SIDE_CAMERA] : cvCreateImage(cvSize(320, 240), IPL_DEPTH_8U, 3));
	if (this->UseCalibrationData) {
		//We have to do undistortion and rectification
		cvResize(currStereoBigImage[LEFT_SIDE_CAMERA], leftResized, CV_INTER_AREA);
		cvRemap(leftResized, currStereoImage[LEFT_SIDE_CAMERA], StereoCalibration::GetInstance()->mx_LEFT,
			StereoCalibration::GetInstance()->my_LEFT);
		cvResize(currStereoBigImage[RIGHT_SIDE_CAMERA], rightResized, CV_INTER_AREA);
		cvRemap(rightResized, rightFinalDestination, StereoCalibration::GetInstance()->mx_RIGHT,
			StereoCalibration::GetInstance()->my_RIGHT);
	} else {
		cvResize(currStereoBigImage[LEFT_SIDE_CAMERA], currStereoImage[LEFT_SIDE_CAMERA], CV_INTER_AREA);
		cvResize(currStereoBigImage[RIGHT_SIDE_CAMERA], currStereoImage[RIGHT_SIDE_CAMERA], CV_INTER_AREA);
	}
	if (vDisplacement != 0 || hDisplacement != 0) {
		for (int row = 0; row < 240; row++) {
			uchar* clonedRowPtr = (uchar *) (currStereoImage[RIGHT_SIDE_CAMERA]->imageData + row*currStereoImage[RIGHT_SIDE_CAMERA]->widthStep);
			uchar* RowPtr = (uchar *) (rightFinalDestination->imageData + (row - vDisplacement)*rightFinalDestination->widthStep - hDisplacement*3);
			for (int col = 0; col < 320; col++, clonedRowPtr+=3, RowPtr+=3) {
				if (row >= vDisplacement && row < 240 - vDisplacement && col >= hDisplacement && col < 320 - hDisplacement) {
					clonedRowPtr[0] = RowPtr[0];
					clonedRowPtr[1] = RowPtr[1];
					clonedRowPtr[2] = RowPtr[2];
				} else {
					clonedRowPtr[0] = 0;
					clonedRowPtr[1] = 0;
					clonedRowPtr[2] = 0;
				}
			}
		}
		cvReleaseImage(&rightFinalDestination);
	}
}

void StereoRig::RequestMoreFrames() {
	if ((ap.cameraAvailableOptions & AcquisitionCameraFactory::CameraDescription::VIRTUAL_CAMERA) && onRequestMode) {
		SetEvent(onRequestFrameEvent);
	}
}

void StereoRig::Run(void *param) {
	if (this->UseCalibrationData)
		ApplicationWorkFlow::GetInstance()->UpdateSystemState(ACQUISITION_STARTED_CALIB);
	else
		ApplicationWorkFlow::GetInstance()->UpdateSystemState(ACQUISITION_STARTED_NO_CALIB);

	SetStatus(THREAD_INITIALIZING);

	int height, width;
	ap.GetResolution(&height, &width, false);
	int heightS, widthS;
	ap.GetResolution(&heightS, &widthS, true);

	IplImage * currBuffer[2] = {cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3),
		cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3)};

	IplImage *remapTempLeft = cvCreateImage(cvSize(widthS, heightS), IPL_DEPTH_8U, 3);
	IplImage *remapTempRight = cvCreateImage(cvSize(widthS, heightS), IPL_DEPTH_8U, 3);
	
	SetStatus(THREAD_LOADING);

	//HANDLE hFile = CreateFile(L".\\STEREO_CAMERA.csv", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if (ap.cameraAvailableOptions & AcquisitionCameraFactory::CameraDescription::CAMERA_OPTION_NETWORK) {
		IplImage * networkDelayBuffer = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
		bool networkDelayBufferUsageFlag = false; int networkDelayBufferSide = -1;
		DWORD networkDelayBufferTimestamp; float networkDelayBufferAvgDelay;

		NetworkCamera* CastedCameraPtr[2] = {(NetworkCamera *) CameraPtr[0], (NetworkCamera *) CameraPtr[1]};

		HANDLE hReadyEvent[3] = {CreateEvent(NULL, FALSE, FALSE, NULL), CreateEvent(NULL, FALSE, FALSE, NULL), closeStereoRigEvent};
		HANDLE hStartEvent[2] = {CreateEvent(NULL, TRUE, FALSE, NULL), CreateEvent(NULL, TRUE, FALSE, NULL)};
		CameraPtr[LEFT_SIDE_CAMERA]->SubscribeEvent(IMAGE_READY, hReadyEvent[LEFT_SIDE_CAMERA]);
		CameraPtr[LEFT_SIDE_CAMERA]->SubscribeEvent(IMAGE_STARTED, hStartEvent[LEFT_SIDE_CAMERA]);
		CameraPtr[RIGHT_SIDE_CAMERA]->SubscribeEvent(IMAGE_READY, hReadyEvent[RIGHT_SIDE_CAMERA]);
		CameraPtr[RIGHT_SIDE_CAMERA]->SubscribeEvent(IMAGE_STARTED, hStartEvent[RIGHT_SIDE_CAMERA]);
		CameraPtr[LEFT_SIDE_CAMERA]->IncreamentForcedWaitingSemaphore();
		CameraPtr[RIGHT_SIDE_CAMERA]->IncreamentForcedWaitingSemaphore();

		CameraPtr[LEFT_SIDE_CAMERA]->Start();
		CameraPtr[RIGHT_SIDE_CAMERA]->Start();

		DWORD currCamera = 0;

		//Calculation of stereo rate
		DWORD expectedDelay = 1000 / ap.fps;
		float averageStereoFrameToStereoFrameInterval = (float) expectedDelay;
		float updateLaw = 1.f/float(ap.fps);
		float cameraAvgDelays[2];

		bool validBuffer[2] = {false, false};
		DWORD startTimestamps[2];
		DWORD lastStereoFrame = GetTickCount();
		while((currCamera = WaitForMultipleObjects(3, hReadyEvent, FALSE, INFINITE) - WAIT_OBJECT_0) != 2) {
			int otherCamera = (currCamera + 1) % 2;
			DWORD theTimestamp; float theDelay;
			IplImage *imgPtr = CameraPtr[currCamera]->GetImage(&theTimestamp, NULL, &theDelay);
			bool otherCameraBegins = WaitForSingleObject(hStartEvent[otherCamera], 0) == WAIT_OBJECT_0;
			/* A new frame is loaded if.... */
			DWORD member1 = (startTimestamps[currCamera] > CastedCameraPtr[otherCamera]->initialTimestamp ? startTimestamps[currCamera] - CastedCameraPtr[otherCamera]->initialTimestamp : CastedCameraPtr[otherCamera]->initialTimestamp - startTimestamps[currCamera]);
			DWORD member2 = (theTimestamp > CastedCameraPtr[otherCamera]->initialTimestamp ? theTimestamp - CastedCameraPtr[otherCamera]->initialTimestamp : CastedCameraPtr[otherCamera]->initialTimestamp - theTimestamp);
			bool loadNewImage = !validBuffer[currCamera] || //Simple case: the current camera has no available image so we can fill. If the is a frame then by construction there is no image in the other.
				!otherCameraBegins || //Neither a valid image is ongoing on the other camera so we can surely replace
				(member1 > member2); //Looking at the arriving frame the new frame is better in terms of timestamp
			if (loadNewImage) {
				validBuffer[currCamera] = true;
				CameraPtr[currCamera]->ImageLock.AcquireReadLock();
				cvCopy(imgPtr, currBuffer[currCamera]);
				startTimestamps[currCamera] = theTimestamp;
				cameraAvgDelays[currCamera] = theDelay;
				CameraPtr[currCamera]->ImageLock.ReleaseReadLock();
			} else {
				CameraPtr[currCamera]->ImageLock.AcquireReadLock();
				cvCopy(imgPtr, networkDelayBuffer);
				CameraPtr[currCamera]->ImageLock.ReleaseReadLock();
				networkDelayBufferUsageFlag = true;
				networkDelayBufferSide = currCamera;
				networkDelayBufferTimestamp = theTimestamp;
				networkDelayBufferAvgDelay = theDelay;
			}
			ReleaseSemaphore(CameraPtr[currCamera]->hSemaphore, 1, NULL);

			while (validBuffer[LEFT_SIDE_CAMERA] && validBuffer[RIGHT_SIDE_CAMERA]) {
				DWORD minTimeStamp = MIN(startTimestamps[LEFT_SIDE_CAMERA], startTimestamps[RIGHT_SIDE_CAMERA]);
				DWORD maxTimeStamp = MAX(startTimestamps[LEFT_SIDE_CAMERA], startTimestamps[RIGHT_SIDE_CAMERA]);

				/*char lineToWrite[1024]; int written = sprintf_s(lineToWrite, 1024, "%lu;%lu;%lu;%lu;%f\r\n", minTimeStamp,
				maxTimeStamp, maxTimeStamp - minTimeStamp, (DWORD) (cameraAvgDelays[LEFT_SIDE_CAMERA] + cameraAvgDelays[RIGHT_SIDE_CAMERA]) / 4, onlineAcquisitionRate);
				DWORD writtenBytes;
				WriteFile(hFile, lineToWrite, written, &writtenBytes, NULL);*/

				if (maxTimeStamp - minTimeStamp <= (cameraAvgDelays[LEFT_SIDE_CAMERA] + cameraAvgDelays[RIGHT_SIDE_CAMERA]) / 4) {
					StereoLock.AcquireWriteLock();
					cvCopy(currBuffer[LEFT_SIDE_CAMERA], currStereoBigImage[LEFT_SIDE_CAMERA]);
					stereoTimestamps.cameraTimeStamps[LEFT_SIDE_CAMERA] = startTimestamps[LEFT_SIDE_CAMERA];
					cvCopy(currBuffer[RIGHT_SIDE_CAMERA], currStereoBigImage[RIGHT_SIDE_CAMERA]);
					stereoTimestamps.cameraTimeStamps[RIGHT_SIDE_CAMERA] = startTimestamps[RIGHT_SIDE_CAMERA];
					stereoTimestamps.stereoTimeStamp =  minTimeStamp + ((maxTimeStamp - minTimeStamp) / 2);
					iApplyCalibrationTransformation(remapTempLeft, remapTempRight);
					StereoLock.ReleaseWriteLock();
					DWORD decurredTime = stereoTimestamps.stereoTimeStamp - lastStereoFrame;
					averageStereoFrameToStereoFrameInterval = ((1.0f - updateLaw) * averageStereoFrameToStereoFrameInterval) + (updateLaw * decurredTime);
					onlineAcquisitionRate = 1000.f / averageStereoFrameToStereoFrameInterval;
					updateLaw = 1.f/onlineAcquisitionRate;
					if (notifyFunction)
						notifyFunction(onlineAcquisitionRate);

					lastStereoFrame = stereoTimestamps.stereoTimeStamp;
					StereoLock.AcquireReadLock();
					if (recorderMode) {
						bvr->AddStereoFrame((this->UseCalibrationData ? remapTempLeft : currStereoBigImage[LEFT_SIDE_CAMERA]),
							(this->UseCalibrationData ? remapTempRight : currStereoBigImage[RIGHT_SIDE_CAMERA]), decurredTime);
					}
					StereoLock.ReleaseReadLock();
					validBuffer[LEFT_SIDE_CAMERA] = validBuffer[RIGHT_SIDE_CAMERA] = false;
					RaiseEvent(STEREO_IMAGE_READY);
				} else if (startTimestamps[LEFT_SIDE_CAMERA] > startTimestamps[RIGHT_SIDE_CAMERA]) {
					validBuffer[RIGHT_SIDE_CAMERA] = false;
				} else {
					validBuffer[LEFT_SIDE_CAMERA] = false;
				}
				if (networkDelayBufferUsageFlag && !validBuffer[networkDelayBufferSide]) {
					networkDelayBufferUsageFlag = false;
					IplImage *exchangePtr = networkDelayBuffer;
					networkDelayBuffer = currBuffer[networkDelayBufferSide];
					currBuffer[networkDelayBufferSide] = exchangePtr;
					validBuffer[networkDelayBufferSide] = true;
					startTimestamps[networkDelayBufferSide] = networkDelayBufferTimestamp;
					cameraAvgDelays[networkDelayBufferSide] = networkDelayBufferAvgDelay;
					networkDelayBufferSide = -1;
				}
			}
		}
		CameraPtr[LEFT_SIDE_CAMERA]->DecrementForcedWaitingSemaphore();
		CameraPtr[RIGHT_SIDE_CAMERA]->DecrementForcedWaitingSemaphore();
		CameraPtr[LEFT_SIDE_CAMERA]->UnSubscribeEvent(IMAGE_READY, hReadyEvent[LEFT_SIDE_CAMERA]);
		CameraPtr[LEFT_SIDE_CAMERA]->Stop();
		CameraPtr[RIGHT_SIDE_CAMERA]->UnSubscribeEvent(IMAGE_READY, hReadyEvent[RIGHT_SIDE_CAMERA]);
		CameraPtr[RIGHT_SIDE_CAMERA]->Stop();

		CloseHandle(hReadyEvent[0]); CloseHandle(hReadyEvent[1]);
		cvReleaseImage(&networkDelayBuffer);
	} else {
		HANDLE hReadyEvent[2] = {videoLoadedEvent, closeStereoRigEvent};
		if (!(ap.cameraAvailableOptions & AcquisitionCameraFactory::CameraDescription::VIRTUAL_CAMERA))
			SetEvent(videoLoadedEvent);
		OpenCvCaptureCamera* CastedCameraPtr[2] = {(OpenCvCaptureCamera *) CameraPtr[0], (OpenCvCaptureCamera *) CameraPtr[1]};
		while (WaitForMultipleObjects(2, hReadyEvent, FALSE, INFINITE) - WAIT_OBJECT_0 != 1) {
			ResetEvent(stopPlaybackEvent);

			int fps = ap.fps;
			if (ap.cameraAvailableOptions & AcquisitionCameraFactory::CameraDescription::VIRTUAL_CAMERA) {
				fps = (int) cvGetCaptureProperty(*CastedCameraPtr[0]->GetCapture(), CV_CAP_PROP_FPS);
			}
			LONG requestedDelay = int(floor(1000.0/fps + 0.5));

			IplImage *leftFrame = CastedCameraPtr[0]->GetImage();
			IplImage *rightFrame = CastedCameraPtr[1]->GetImage();
			DWORD firstFrameTime = GetTickCount();
			DWORD lastFrameTime = firstFrameTime;
			DWORD waitResult = WAIT_TIMEOUT;
			LARGE_INTEGER initialWaiting; initialWaiting.QuadPart = 0LL;
			HANDLE triggerHandle = INVALID_HANDLE_VALUE;
			stereoTimestamps.cameraTimeStamps[LEFT_SIDE_CAMERA] = stereoTimestamps.cameraTimeStamps[RIGHT_SIDE_CAMERA] = stereoTimestamps.stereoTimeStamp = GetTickCount();
			if (onRequestMode) {
				triggerHandle = onRequestFrameEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
			} else {
				triggerHandle = CreateWaitableTimer(NULL, FALSE, NULL);
			}
			HANDLE innerEventHandleVector[] = {triggerHandle, stopPlaybackEvent, closeStereoRigEvent};
			do {
				if (hOffsetFile != INVALID_HANDLE_VALUE) {
					DWORD readedDelay; DWORD readedDelayBytes; ReadFile(hOffsetFile, &readedDelay, sizeof(DWORD), &readedDelayBytes, NULL);
					requestedDelay = (LONG) readedDelay; //Disable this line to disable variable delay frames
				}
				__int64 qwDueTime = -requestedDelay * 10000;
				initialWaiting.LowPart = (DWORD) (qwDueTime & 0xFFFFFFFF);
				initialWaiting.HighPart = (LONG) (qwDueTime >> 32);
				SetWaitableTimer(triggerHandle, &initialWaiting, 0, NULL, NULL, 0);
				StereoLock.AcquireWriteLock();
				//if (onRequestMode) {
					stereoTimestamps.stereoTimeStamp += requestedDelay;
					stereoTimestamps.cameraTimeStamps[LEFT_SIDE_CAMERA] = stereoTimestamps.cameraTimeStamps[RIGHT_SIDE_CAMERA] = stereoTimestamps.stereoTimeStamp;
				//} else {
				//	stereoTimestamps.cameraTimeStamps[LEFT_SIDE_CAMERA] = stereoTimestamps.cameraTimeStamps[RIGHT_SIDE_CAMERA] = stereoTimestamps.stereoTimeStamp = GetTickCount();
				//}
				cvCopy(leftFrame, currStereoBigImage[LEFT_SIDE_CAMERA]);
				cvCopy(rightFrame, currStereoBigImage[RIGHT_SIDE_CAMERA]);
				iApplyCalibrationTransformation(remapTempLeft, remapTempRight);
				StereoLock.ReleaseWriteLock();
				RaiseEvent(STEREO_IMAGE_READY);
				leftFrame = CastedCameraPtr[0]->GetImage();
				rightFrame = CastedCameraPtr[1]->GetImage();
				lastFrameTime = GetTickCount();
			} while (leftFrame && rightFrame && (waitResult = WaitForMultipleObjects(3, innerEventHandleVector, FALSE, INFINITE)) == WAIT_OBJECT_0);
			CancelWaitableTimer(innerEventHandleVector[0]);
			CloseHandle(innerEventHandleVector[0]);
			Logger::writeToLOG(L"Video Duration: %d\r\n", int(lastFrameTime - firstFrameTime));
			StereoLock.AcquireWriteLock();
			cvReleaseCapture(CastedCameraPtr[0]->GetCapture());
			cvReleaseCapture(CastedCameraPtr[1]->GetCapture());
			CloseHandle(hOffsetFile);
			StereoLock.ReleaseWriteLock();
		}
	}

	//CloseHandle(hFile);

	ResetEvent(closeStereoRigEvent);

	cvReleaseImage(&currBuffer[LEFT_SIDE_CAMERA]);
	cvReleaseImage(&currBuffer[RIGHT_SIDE_CAMERA]);
	cvReleaseImage(&remapTempLeft);
	cvReleaseImage(&remapTempRight);

	ApplicationWorkFlow::GetInstance()->UpdateSystemState(ACQUISITION_STOPPED);
}

StereoRigStatus StereoRig::GetStatus() {
	return this->currStatus;
}

void StereoRig::SetStatus(StereoRigStatus newStatus) {
	currStatus = newStatus;
}

StereoRig::StereoTimestampsStruct StereoRig::GetStereoImages(IplImage **left, IplImage **right, bool bigImage) {
	IplImage **selectedPtr = (bigImage ? currStereoBigImage : currStereoImage);
	if (left)
		*left = selectedPtr[LEFT_SIDE_CAMERA];
	if (right)
		*right = selectedPtr[RIGHT_SIDE_CAMERA];
	return stereoTimestamps;
}

bool StereoRig::StartRecorderMode() {
	if (recorderMode)
		return false;
	wchar_t platheaTempDir[_MAX_PATH]; GetPLaTHEATempPath(platheaTempDir, _MAX_PATH);
	StereoLock.AcquireReadLock();
	int onlineWritingRate = (int) floor(onlineAcquisitionRate + 0.5f);
	StereoLock.ReleaseReadLock();
	CvSize resolution = (this->UseCalibrationData ? cvSize(320, 240) : ap.GetResolution(NULL, NULL, false));
	bvr = new BufferedVideoRecorder(platheaTempDir, resolution, onlineWritingRate);
	bvr->Start();
	StereoLock.AcquireWriteLock();
	recorderMode = true;
	StereoLock.ReleaseWriteLock();
	return true;
}

bool StereoRig::StopRecorderMode() {
	if (!recorderMode)
		return false;
	StereoLock.AcquireWriteLock();
	recorderMode = false;
	StereoLock.ReleaseWriteLock();
	bvr->Stop();
	delete bvr;
	return true;
}

bool StereoRig::SaveRecordingToDirectory(const char *baseDirectory) {
	wchar_t platheaTempDir[_MAX_PATH]; GetPLaTHEATempPath(platheaTempDir, _MAX_PATH);
	char * fileNames[] = {"left.avi", "right.avi"};
	char destinationFileName[_MAX_PATH]; char sourceFileName[_MAX_PATH];
	for (int i = 0; i < 2; i++) {
		sprintf_s(destinationFileName, "%s\\%s", baseDirectory, fileNames[i]);
		sprintf_s(sourceFileName, "%S\\%s", platheaTempDir, fileNames[i]);
		if (!MoveFileExA(sourceFileName, destinationFileName, MOVEFILE_COPY_ALLOWED))
			return false;
	}
	sprintf_s(destinationFileName, "%s\\offsets.bin", baseDirectory);
	sprintf_s(sourceFileName, "%S\\offsets.bin", platheaTempDir);
	if (!MoveFileExA(sourceFileName, destinationFileName, MOVEFILE_COPY_ALLOWED))
		return false;
	return true;
}

bool StereoRig::StartPlaybackMode(const char *baseDirectory, bool onRequestMode) {
	if (!(ap.cameraAvailableOptions & AcquisitionCameraFactory::CameraDescription::VIRTUAL_CAMERA))
		return false;
	this->onRequestMode = onRequestMode;
	StereoLock.AcquireWriteLock();
	OpenCvCaptureCamera* CastedCameraPtr[2] = {(OpenCvCaptureCamera *) CameraPtr[0], (OpenCvCaptureCamera *) CameraPtr[1]};
	char * fileNames[] = {"left.avi", "right.avi"};
	char destinationFileName[_MAX_PATH];
	for (int i = 0; i < 2; i++) {
		sprintf_s(destinationFileName, "%s\\%s", baseDirectory, fileNames[i]);
		*CastedCameraPtr[i]->GetCapture() = cvCreateFileCapture(destinationFileName);
	}
	sprintf_s(destinationFileName, "%s\\offsets.bin", baseDirectory);
	hOffsetFile = CreateFileA(destinationFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	cvReleaseImage(&currStereoBigImage[LEFT_SIDE_CAMERA]);
	cvReleaseImage(&currStereoBigImage[RIGHT_SIDE_CAMERA]);
	int width = (int) cvGetCaptureProperty(*CastedCameraPtr[LEFT_SIDE_CAMERA]->GetCapture(), CV_CAP_PROP_FRAME_WIDTH);
	int height = (int) cvGetCaptureProperty(*CastedCameraPtr[LEFT_SIDE_CAMERA]->GetCapture(), CV_CAP_PROP_FRAME_HEIGHT);
	currStereoBigImage[LEFT_SIDE_CAMERA] = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
	currStereoBigImage[RIGHT_SIDE_CAMERA] = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);

	char resString[64]; sprintf_s(resString, "%dx%d", width, height);
	ap.resolution = resString;

	StereoLock.ReleaseWriteLock();
	SetEvent(videoLoadedEvent);
	return true;
}

bool StereoRig::StopPlaybackMode() {
	if (!(ap.cameraAvailableOptions & AcquisitionCameraFactory::CameraDescription::VIRTUAL_CAMERA))
		return false;
	onRequestMode = false;
	SetEvent(stopPlaybackEvent);
	return true;
}

void StereoRig::SetFrameAsConsumed() {
	//Fill if we want to consume all the frames
}