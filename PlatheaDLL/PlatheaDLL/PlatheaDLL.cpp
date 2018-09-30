// PlatheaDLL.cpp: definisce le funzioni esportate per l'applicazione DLL.


#include "stdafx.h"
#include "PlatheaDLL.h"
#include <mutex>          // std::mutex

std::mutex mtx;           // mutex for critical section


using namespace std;
using namespace stdext;
using namespace leostorm::logging;
using namespace leostorm::settingspersistence;


// variables

SystemInfo * si = new SystemInfo();
vector<TrackedObject*> trackedPeople;

HWND hActiveModelessWindow = NULL;
bool hideVideoInput = false;
bool saveTracksToFile = false;
bool svmTracker = false;


HANDLE* positionThread;



// Da cancellare
Point* createPoint() {
	return new Point();
}

void internalCalibrationDemo(const char str[]) {
	calibration_internalcalibration_load(str, 7);
}
void initializeSystemDemo() {
	system_initializesystem("root", "password", "none", "320x240", 10, "Virtual Acquisition Camera", 
		"192.168.8.3", 80, "192.168.8.2", 80);
}

void openPositionTest() {
	if (si->GetElaborationCore() == NULL) {
		//MessageBox(hwnd, L"Elaboration core has not been started. No software measurements will be available.",
		//	L"Warning", MB_OK | MB_ICONWARNING);
		return;
	}
	else if (si->GetElaborationCore()->DuringInitializationPhase()) {
		//int exitInitPhase = MessageBox(hwnd, L"Elaboration core is still into initialization phase. Tracking will not be performed. Do you want to exit from initialization phase?",
		//	L"Warning", MB_YESNO | MB_ICONQUESTION);
		//if (exitInitPhase == IDYES)
			si->GetElaborationCore()->EndInitializationPhase();
	}
	PositionTestThread();
	cout << "PositionTestThread" << endl;


}
void startPositionTest() {
	PositionTestProc(0);
}



// type: none/Open HTTP
// resolution: 320x240/480x360/640/480
// fps: 10/15
// camera model: Virtual Acquisition Camera/Axis 207 Network Camera/FireWire Camera/Vivotek IP161 Network Camera
void system_initializesystem(const char* username, const char* password, const char* type, const char* resolution,
	int fps, const char* cameraModel, const char* IPAddress1, int port1, const char* IPAddress2, int port2) {
	char strBuffer[64];

	InitializationStruct leftIs, rightIs;
	InitializationStruct::AcquisitionProperties ap;
	InitializationStruct::Authentication auth;

	auth.username = username;
	auth.password = password;
	auth.type = (strcmp(type, "none") == 0 ? InitializationStruct::Authentication::NONE_AUTH : InitializationStruct::Authentication::OPEN_HTTP);

	ap.resolution = resolution;
	ap.fps = fps;
	ap.cameraModel = ap.cameraModel = cameraModel;
	ap.cameraAvailableOptions = AcquisitionCameraFactory::GetRegisteredCameras()[ap.cameraModel].availableOptions;

	leftIs.tcddata.IPAddress = IPAddress1;
	leftIs.tcddata.portNumber = port1;
	leftIs.authentication = auth;
	leftIs.acquisitionProperties = ap;
	leftIs.componentName = "LEFT_SIDE_CAMERA";

	rightIs.tcddata.IPAddress = IPAddress1;
	rightIs.tcddata.portNumber = port2;
	rightIs.authentication = auth;
	rightIs.acquisitionProperties = ap;
	rightIs.componentName = "RIGHT_SIDE_CAMERA";

	AcquisitionCamera *leftCamera = AcquisitionCameraFactory::CreateNewInstance(ap.cameraModel, leftIs);
	AcquisitionCamera *rightCamera = AcquisitionCameraFactory::CreateNewInstance(ap.cameraModel, rightIs);

	si->SetStereoRig(new StereoRig(ap, leftCamera, rightCamera, true));
	si->GetStereoRig()->Start();

	bool hideVideoInput = false;
	if (!hideVideoInput) {
		VideoEvent ve1;
		ve1.st = NETWORK_STEREO_RIG;
		ve1.eventSource = si->GetStereoRig();
		ve1.ev = STEREO_IMAGE_READY;
		si->GetVideoOutput()->ChangeSource(&ve1);
	}
}

void system_hidevideoinput() {
	hideVideoInput = !hideVideoInput;
	//CheckMenuItem(hMenu, ID_SYSTEM_HIDEVIDEOINPUT, (hideVideoInput ? MF_CHECKED : MF_UNCHECKED));
	if (si->GetStereoRig()->IsRunning()) {
		if (hideVideoInput) {
			si->GetVideoOutput()->ChangeSource(NULL, NULL);
		}
		else {
			VideoEvent ve1;
			ve1.st = NETWORK_STEREO_RIG;
			ve1.eventSource = si->GetStereoRig();
			ve1.ev = STEREO_IMAGE_READY;
			si->GetVideoOutput()->ChangeSource(&ve1);
		}
	}

}

void system_alignrightframe(int hDisplacement, int vDisplacement) {
	wchar_t auxBuf[256];
	//GetDlgItemText(hwndDlg, IDC_EDIT1, auxBuf, 256);
	StereoRig::hDisplacement = hDisplacement;
	//GetDlgItemText(hwndDlg, IDC_EDIT2, auxBuf, 256);
	StereoRig::vDisplacement = vDisplacement;
	//EndDialog(hwndDlg, 0);
}

void system_stopacquisition() {
	si->GetVideoOutput()->ChangeSource(NULL, NULL);
	si->GetStereoRig()->Stop();
	delete si->GetStereoRig()->GetAcquisitionCamera(LEFT_SIDE_CAMERA);
	delete si->GetStereoRig()->GetAcquisitionCamera(RIGHT_SIDE_CAMERA);
	delete si->GetStereoRig();
	si->SetStereoRig(NULL);
}

void system_selecthaarcascadexml(const char dir[]) {
	mainFaceDatabase.SetHaarClassifierFileName(dir);
}

void system_loadconfigurationfile(const char str[]) {
	//char str[] = "C:\\Users\\jack1\\Desktop\\PLaTHEATest\\";
	SettingsPersistence::GetInstance()->LoadFromFile(str);
}

void system_saveconfigurationfileas(const char dir[]) {
	SettingsPersistence::GetInstance()->SaveToFile("PLaTHEAConfiguration", dir);
}

void system_acquisitionstats_start() {
	StartAcquisitionStatisticProc();
}

void system_acquisitionstats_stop() {
	StartAcquisitionStatisticProc();
}

float* system_acquisitionstats() {
	return GetAcquisitionStatisticProc();
}

void test_positiontest(bool exitInitPhase) {
	if (si->GetElaborationCore() == NULL) {
	}
	else if (si->GetElaborationCore()->DuringInitializationPhase()) {
		if (exitInitPhase)
			si->GetElaborationCore()->EndInitializationPhase();
	}
	positionThread = ShowPositionTestWindow();
}

void test_positiontest_start() {
	positionThread = PositionTestProc(0);
}

void test_positiontest_stop() {
	TerminateThread(positionThread, 0);
}

void test_savetrackstofile() {
	saveTracksToFile = !saveTracksToFile;
}

void test_plathearecorder() {
	if (si->GetStereoRig()) {
		PLaTHEARecorderMode selectedMode = PLaTHEA_RECORDER_MODE;
		if (si->GetStereoRig()->GetAcquisitionProperties().cameraAvailableOptions & AcquisitionCameraFactory::CameraDescription::VIRTUAL_CAMERA) {
			selectedMode = PLaTHEA_PLAYER_MODE;
		}
		//ShowRecordPlayerDialog(hwnd, selectedMode);
	}
}

void test_startvideorecording() {
	si->GetStereoRig()->StartRecorderMode();
}

// if <save> is true then record is saved in directory with path <dir>
void test_stopvideorecordingandsave(bool save, const char* dir) {
	si->GetStereoRig()->StopRecorderMode();
	if (save)
		si->GetStereoRig()->SaveRecordingToDirectory(dir);
}

void test_svmlearning(const char dir[]) {
		PlanViewMap::SetSVMClassifierFileName(dir);
		svmTracker = true;
}


void calibration_saveexternalcalibrationdata(const char dir[]){
	ExternalCalibration::GetInstance()->SaveToFolder(dir);
}

void calibration_loadexternalcalibrationdata(const char str[]) {
	ExternalCalibration::GetInstance()->LoadFromFolder(str);
}

//enum CalibrationType{LEFT_INTERNAL = 1, RIGHT_INTERNAL = 2, STEREO = 4, ALL_CALIBRATION = 8};
void calibration_internalcalibration_save(const char dir[], int selectedMask) {
	StereoCalibration::GetInstance()->SaveToFolder(dir, selectedMask);
}

//enum CalibrationType{LEFT_INTERNAL = 1, RIGHT_INTERNAL = 2, STEREO = 4, ALL_CALIBRATION = 8};
void calibration_internalcalibration_load(const char dir[], int selectedMask) {
	StereoCalibration::GetInstance()->LoadFromFolder(dir, selectedMask);
}

void calibration_internalcalibration_rectificationbyparam() {
	StereoCalibration *csc = StereoCalibration::GetInstance();
	if (csc->D_LEFT == NULL || csc->M_LEFT == NULL || csc->D_RIGHT == NULL || csc->M_RIGHT == NULL || csc->R == NULL || csc->T == NULL) {
		return;
	}

	csc->Q = cvCreateMat(4, 4, CV_64FC1);

	csc->mx_LEFT = cvCreateMat(240, 320, CV_32F);
	csc->my_LEFT = cvCreateMat(240, 320, CV_32F);
	csc->mx_RIGHT = cvCreateMat(240, 320, CV_32F);
	csc->my_RIGHT = cvCreateMat(240, 320, CV_32F);

	double R1[3][3], R2[3][3];
	CvMat _R1 = cvMat(3, 3, CV_64F, R1);
	CvMat _R2 = cvMat(3, 3, CV_64F, R2);

	/* Bouguet Method */
	double P1[3][4], P2[3][4];
	CvMat _P1 = cvMat(3, 4, CV_64F, P1);
	CvMat _P2 = cvMat(3, 4, CV_64F, P2);
	cvStereoRectify(csc->M_LEFT, csc->M_RIGHT, csc->D_LEFT, csc->D_RIGHT, cvSize(320, 240),
		csc->R, csc->T, &_R1, &_R2, &_P1, &_P2, csc->Q, CV_CALIB_ZERO_DISPARITY);
	cvInitUndistortRectifyMap(csc->M_LEFT, csc->D_LEFT, &_R1, &_P1, csc->mx_LEFT, csc->my_LEFT);
	cvInitUndistortRectifyMap(csc->M_RIGHT, csc->D_RIGHT, &_R2, &_P2, csc->mx_RIGHT, csc->my_RIGHT);
}

void calibration_internalcalibration_rectification_bouguet(bool singleCalibration) {
	StereoCalibration::GetInstance()->RectificationBouguet(singleCalibration);	
}

void calibration_internalcalibration_rectification_hartley(bool singleCalibration) {
	StereoCalibration::GetInstance()->RectificationHartley(singleCalibration);
}

void calibration_internalcalibration_startleft() {
	StereoCalibration::GetInstance()->StartLeft();
}

void calibration_internalcalibration_startright() {
	StereoCalibration::GetInstance()->StartRight();
}

void calibration_internalcalibration_stop() {
	StereoCalibration::GetInstance()->StopCalibration();
}

void calibration_editroomsettings(float WXmin, float WXmax, float WYmin, float WYmax, float WZmin, float WZmax,
	float texelSide, float personMaximumHeight, float personAverageHeight, float personAverageWidth, float personMimimumHeight) {
	RoomSettings::GetInstance()->EditSettings( WXmin,  WXmax,  WYmin,  WYmax,  WZmin,  WZmax,
		 texelSide,  personMaximumHeight,  personAverageHeight,  personAverageWidth,  personMimimumHeight);
}


void localizationengine_startlocalizationengine(bool withoutTracking, bool saveTracksToFile, const char dir []) {
	ElaborationCore *ec = new ElaborationCore();
	wchar_t errMsg[1024];
	if (withoutTracking)
		ec->SetElaborationCoreMode(ElaborationCore::TRACKING_FREE_ELABORATION_CORE_MODE);
	else if (saveTracksToFile) {
			ec->ActivateSaveToFileMode(dir);
	}
	if (ec->PrerequisitesCheck(errMsg, 1024)) {
		si->SetElaborationCore(ec);
		si->GetElaborationCore()->Start();
	}
	else {
		delete ec;
	}
}

void localizationengine_endinitializationphase() {
	si->GetElaborationCore()->EndInitializationPhase();
}

void localizationengine_stoplocalizationengine() {
	si->SetElaborationCore(NULL);
}

void localizationengine_selectsvmclassifier(const char dir []) {
		PlanViewMap::SetSVMClassifierFileName(dir);		
		svmTracker = true;	
}

void localizationengine_opticalflow() {
	if (svmTracker) {
		svmTracker = false;
	}
}

void localizationengine_svm() {
	if (!svmTracker) {
		svmTracker = true;
	}
}




std::vector<TrackedObject*> getTrackedPeople() {
	mtx.lock();
	std::vector<TrackedObject*> ret = trackedPeople;
	mtx.unlock();
	return ret;
}



void updateTrackedPeople(std::vector<TrackedObject*> trackedPersons) {
	mtx.lock();
	trackedPeople = trackedPersons;
	mtx.unlock();
}





//