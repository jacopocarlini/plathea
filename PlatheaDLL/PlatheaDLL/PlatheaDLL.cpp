// PlatheaDLL.cpp: definisce le funzioni esportate per l'applicazione DLL.


#include "stdafx.h"
#include "PlatheaDLL.h"
#include <mutex>          // std::mutex

#include "JackSettings.h"

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

#define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s << "\n";                   \
   OutputDebugStringW( os_.str().c_str() );  \
}

/*
TrackedPerson* createPerson(char prefix[2],static int staticID, int X, int Y, int nameID, char identityPrefix[2], char name[256]) {
	return (new TrackedPerson(prefix, staticID, X,Y, nameID, identityPrefix, name));
}
*/

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


std::string setPath(string path) {
	int length = path.length();
	//printf("il path prima: %s\n", path.c_str());
	//printf("length=%d\n", length);
	string delimiter = "\\";
	size_t pos = 0;
	pos = path.find_last_of(delimiter);
	//printf("pos=%d\n", pos);
	//printf("il path dopo: %s\n", path.substr(0, pos).c_str());
	return path.substr(0, pos);	
}


void system_loadconfigurationfile(int roomId, char str[]) {
	char currentDirectory[_MAX_PATH];
	strcpy(currentDirectory, str);
	//PathRemoveFileSpecA(currentDirectory); 
	//setPath è una funzione naif per emulare PathRemoveFileSpecA 
	string s(str);
	//printf("prima della chiamata: %s\n",s.c_str());
	//DBOUT(setPath(s).c_str());
	//const char* path = setPath(s).c_str();
	//printf("dopo la chiamata: %s\n", setPath(s).c_str());
	si->GetVideoOutput()->setRoomID(roomId);
	IDroom = roomId;
	SetCurrentDirectoryA(setPath(s).c_str());
	SettingsPersistence::GetInstance()->LoadFromFile(str);
	//UpdateWindowTitle(hwnd);
	//SetCursor(originalCursor);
}



void system_saveconfigurationfileas(const char dir[]) {
	si = new SystemInfo();
	ApplicationWorkFlow::GetInstance()->UpdateSystemState(APPLICATION_STARTED);

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
	PositionTestStart();
}

void test_positiontest_stop() {
	TerminateThread(positionThread, 0);
}

void test_savetrackstofile() {
	saveTracksToFile = !saveTracksToFile;
}

void test_platheaplayer() {
	if (si->GetStereoRig()) {
		PLaTHEARecorderMode selectedMode = PLaTHEA_RECORDER_MODE;
		if (si->GetStereoRig()->GetAcquisitionProperties().cameraAvailableOptions & AcquisitionCameraFactory::CameraDescription::VIRTUAL_CAMERA) {
			selectedMode = PLaTHEA_PLAYER_MODE;
		}
		ShowRecordPlayerDialog(NULL, selectedMode);
	}
}

void test_platheaplayer_start(const char* dir) {
	//const char* s = "D:\PLaTHEATest\Tests\21-12-2012 - 11-25-10-165";
	printf("start\n");
	StartPlayer(dir);
	printf("started\n");
}

void test_plathearecorder_stop() {
	StopPlayer();
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
		DBOUT(dir);
		PlanViewMap::SetSVMClassifierFileName(dir);
		svmTracker = true;
}


void calibration_saveexternalcalibrationdata(const char dir[]){
	ExternalCalibration::GetInstance()->SaveToFolder(dir);
}

void calibration_loadexternalcalibrationdata(const char str[]) {
	int ret = ExternalCalibration::GetInstance()->LoadFromFolder(str);
	printf("external calibration =%d\n",ret);
}

//enum CalibrationType{LEFT_INTERNAL = 1, RIGHT_INTERNAL = 2, STEREO = 4, ALL_CALIBRATION = 8};
void calibration_internalcalibration_save(const char dir[], int selectedMask) {
	StereoCalibration::GetInstance()->SaveToFolder(dir, selectedMask);
}

//enum CalibrationType{LEFT_INTERNAL = 1, RIGHT_INTERNAL = 2, STEREO = 4, ALL_CALIBRATION = 8};
void calibration_internalcalibration_load(const char dir[], int selectedMask) {
	//InternalState currentState = ApplicationWorkFlow::GetInstance()->GetCurrentState();
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
	printf("start localization engine\n");
	if (withoutTracking)
		ec->SetElaborationCoreMode(ElaborationCore::TRACKING_FREE_ELABORATION_CORE_MODE);
	else if (saveTracksToFile) {
			ec->ActivateSaveToFileMode(dir);
	}
	if (ec->PrerequisitesCheck(errMsg , 1024)) {
	 	printf("ec avviato\n");
		si->SetElaborationCore(ec);
		si->GetElaborationCore()->Start();
	}
	else {
		printf("ec delete\n");

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
		DBOUT(dir);
		PlanViewMap::SetSVMClassifierFileName(dir);		
		svmTracker = true;	
		printf("svm selected\n");
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

void localizationengine_setengineparameters() {

}




std::vector<TrackedObject*> getTrackedPeople() {
	mtx.lock();
	std::vector<TrackedObject*> ret = trackedPeople;
	mtx.unlock();
	return ret;
}

ElaborationCore* getElaborationCore() {
	return (si->GetElaborationCore());
}


void updateTrackedPeople(std::vector<TrackedObject*> trackedPersons) {
	printf("update tracked people\n");
	mtx.lock();
	//trackedPeople = trackedPersons;
	std::vector<TrackedObject*> persons = trackedPersons;

	trackedPeople.clear();
	//trackedPeople = std::vector<TrackedPerson*>();
	static int round = 0;
	int notificationStep = 1; //How many frames should pass between a measurement and another? >=1
	char rowToWrite[1024];
	int texelSide = (int)RoomSettings::GetInstance()->data.texelSide;
	printf("persons.size() %d\n",persons.size());
	for (int i = 0; i < int(persons.size()); i++) {
		if (persons[i]->type == TRACKED || persons[i]->type == LOST) {
			printf("TRACKED OR LOST\n");
			int newX = ((persons[i]->bottomRight.x + persons[i]->upperLeft.x) / 2)*texelSide;
			int newY = ((persons[i]->bottomRight.y + persons[i]->upperLeft.y) / 2)*texelSide;
			char prefix[2];
			if (persons[i]->type == TRACKED)
				sprintf_s(prefix, "t");
			else if (persons[i]->type == LOST)
				sprintf_s(prefix, "l");
			char identityPrefix[2];
			if (persons[i]->nameID != -1) {
				if (persons[1]->justIdentified)
					sprintf_s(identityPrefix, "j");
				else
					sprintf_s(identityPrefix, "i");
			}
			else {
				sprintf_s(identityPrefix, "-");
			}
			sprintf_s(rowToWrite, "%s\t%d\t(%d,%d)\t%d\t%s\t%s\r\n", prefix, persons[i]->ID, newX, newY,
				persons[i]->nameID, identityPrefix, (persons[i]->nameID == -1 ? "-" : persons[i]->name));
			printf("rowToWrite %s\n",rowToWrite);
			persons[i]->X = newX;
			persons[i]->Y = newY;
			trackedPeople.push_back((persons[i]));
			//trackedPeople.push_back(new TrackedPerson(prefix, persons[i]->ID, newX, newY,persons[i]->nameID, identityPrefix, persons[i]->name));
			if (round == 0)
				printf("round 0");
		}
		
	}
	round++; round %= notificationStep;

	mtx.unlock();
}





//