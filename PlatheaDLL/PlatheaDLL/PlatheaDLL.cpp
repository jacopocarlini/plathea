// PlatheaDLL.cpp: definisce le funzioni esportate per l'applicazione DLL.


#include "stdafx.h"
#include "PlatheaDLL.h"


using namespace std;
using namespace stdext;
using namespace leostorm::logging;
using namespace leostorm::settingspersistence;



SystemInfo * si = new SystemInfo();

HWND hActiveModelessWindow = NULL;
bool hideVideoInput = false;
bool saveTracksToFile = false;
bool svmTracker = false;

// DEMO's functions
void loadConfigurationFile(const char str[]) {
	//char str[] = "C:\\Users\\jack1\\Desktop\\PLaTHEATest\\";
	SettingsPersistence::GetInstance()->LoadFromFile(str);
}


//enum CalibrationType{LEFT_INTERNAL = 1, RIGHT_INTERNAL = 2, STEREO = 4, ALL_CALIBRATION = 8};
void internalCalibration(const char str[], int selectedMask) {
	DBOUT("internalCalibration");
	//char str[] = "C:\\Users\\jack1\\Desktop\\PLaTHEATest\\InternalCalibration";
	StereoCalibration::GetInstance()->LoadFromFolder(str, selectedMask);
}


void externalCalibration(const char str[]) {
	DBOUT("externalCalibration");
	//char str[] = "C:\\Users\\jack1\\Desktop\\PLaTHEATest\\ExternalCalibration";
	ExternalCalibration::GetInstance()->LoadFromFolder(str);
}

void svmClassifier(const char str[]) {
	DBOUT("svmClassifier");
	//char str[] = "C:\\Users\\jack1\\Desktop\\PLaTHEATest\\Tracking\\svmclassifier.xml";
	PlanViewMap::SetSVMClassifierFileName(str);
}

// this is equals then initializeSystem() but with defined values - ONLY for testing purpose
void initializeSystemDemo() {
	DBOUT("initializeSystem");
	char strBuffer[64];

	InitializationStruct leftIs, rightIs;
	InitializationStruct::AcquisitionProperties ap;
	InitializationStruct::Authentication auth;

	auth.username = "root";
	auth.password = "password";
	auth.type = (strcmp("none", "none") == 0 ? InitializationStruct::Authentication::NONE_AUTH : InitializationStruct::Authentication::OPEN_HTTP);

	ap.resolution = "320x240";
	ap.fps = atoi("10");
	ap.cameraModel = ap.cameraModel = "Virtual Acquisition Camera";
	ap.cameraAvailableOptions = AcquisitionCameraFactory::GetRegisteredCameras()[ap.cameraModel].availableOptions;

	leftIs.tcddata.IPAddress = "192.168.8.3";
	leftIs.tcddata.portNumber = atoi("80");
	leftIs.authentication = auth;
	leftIs.acquisitionProperties = ap;
	leftIs.componentName = "LEFT_SIDE_CAMERA";

	rightIs.tcddata.IPAddress = "192.168.8.2";
	rightIs.tcddata.portNumber = atoi("80");
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
// type: none/Open HTTP
// resolution: 320x240/480x360/640/480
// fps: 10/15
// camera model: Virtual Acquisition Camera/Axis 207 Network Camera/FireWire Camera/Vivotek IP161 Network Camera
void initializeSystem(const char* username, const char* password, const char* type, const char* resolution,
	int fps, const char* cameraModel, const char* IPAddress1, int port1, const char* IPAddress2, int port2) {
	DBOUT("initializeSystem");
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

void startLocalEngine(bool withoutTracking, bool saveTracksToFile) {
	DBOUT("startLocalEngine");
	wchar_t errMsg[1024];
	ElaborationCore *ec = new ElaborationCore();
	if (withoutTracking)
		ec->SetElaborationCoreMode(ElaborationCore::TRACKING_FREE_ELABORATION_CORE_MODE);
	else if (saveTracksToFile) {
		//COMDLG_FILTERSPEC filtro[] = { L"Track Log File", L"*.log" };
		//if (UseCommonItemDialog(buf, sizeof(buf), hwnd, CLSID_FileSaveDialog, 0, filtro, 1)) {
			ec->ActivateSaveToFileMode("track.log");
		//}
	}
	if (ec->PrerequisitesCheck(errMsg, 1024)) {
		si->SetElaborationCore(ec);
		si->GetElaborationCore()->Start();
	}
	else {
		delete ec;
		//MessageBox(si->GetMainWindow(), errMsg, L"Error", MB_ICONERROR);
	}

	//ElaborationCore *ec = new ElaborationCore();
	//si->SetElaborationCore(ec);
	//si->GetElaborationCore()->Start();
}

void openPositionTest() {
	DBOUT("startPositionTest")
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

// not implemented
void system_acquisitionstats() {}

// TODO: gestire facebd view
void system_editfacedatabase() {
	wchar_t errMsg[1024];
	if (mainFaceDatabase.PrerequisitesCheck(errMsg, 1024)) {
		//DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_FACEDBASE), NULL, FaceDBaseDialogProc);
		// dovrebbe aprire la finestra per il facedb
	}
	else {
		MessageBox(si->GetMainWindow(), errMsg, L"Error", MB_ICONERROR);
	}
}


void test_savetrackstofile() {
	saveTracksToFile = !saveTracksToFile;
	//CheckMenuItem(hMenu, ID_TEST_SAVETRACKSTOFILE, (saveTracksToFile ? MF_CHECKED : MF_UNCHECKED));
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
		si->GetStereoRig()->SaveRecordingToDirectory(buf);
}






