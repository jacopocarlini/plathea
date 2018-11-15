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

#include "CommonHeader.h"
#include "FaceDatabase.h"
#include "NetworkCamera.h"
#include "video_output.h"
#include "ElaborationCore.h"
#include "ApplicationWorkFlow.h"
#include "StereoCalibration.h"
#include "SyncTest.h"
#include "RoomSettings.h"
#include "TestDesigner.h"
#include "PositionTestDialog.h"
#include "TestAnalyzerDialog.h"
#include "RecordPlayerDialog.h"
#include "Shared.h"
#include "SVMDialog.h"
#include "AcquisitionStatistics.h"
#include "TCPServer.h"

#include <LeoLog4CPP.h>
#include <LeoWindowsGUI.h>
#include <LeoWindowsConsole.h>
#include <LeoSettingsPersistence.h>




using namespace std;
using namespace stdext;
using namespace leostorm::logging;
using namespace leostorm::settingspersistence;

TCHAR strAppName[] = TEXT("Localization System");
TCHAR mainWindowBaseTitle[] = TEXT("PLaTHEA - Administrator Window");
SystemInfo * si;
HICON hIcon; HWND hwndLeft, hwndRight, statusHwnd; HMENU hMenu;

HWND hActiveModelessWindow = NULL;
bool hideVideoInput = false;
bool saveTracksToFile = false;
bool svmTracker = false;

TCPServer *serv = NULL;

void UpdateWindowTitle(HWND window) {
	DBOUT("UpdateWindowTitle");
	wchar_t windowTitle[1024];
	char *configurationFileName = SettingsPersistence::GetInstance()->GetCurrentlyLoadedFile();
	if (configurationFileName)
		swprintf_s(windowTitle, 1024, L"[%S] %s", configurationFileName, mainWindowBaseTitle);
	else
		swprintf_s(windowTitle, 1024, mainWindowBaseTitle);
	SetWindowText(window, windowTitle);
}

INT_PTR CALLBACK AlignRightFrameProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	DBOUT("AlignRightFrameProc");
	switch (uMsg) {
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDCANCEL:
			EndDialog(hwndDlg,0);
			return TRUE;
		case IDOK:
			{
				wchar_t auxBuf[256];
				GetDlgItemText(hwndDlg, IDC_EDIT1, auxBuf, 256);
				StereoRig::hDisplacement = _wtoi(auxBuf);
				GetDlgItemText(hwndDlg, IDC_EDIT2, auxBuf, 256);
				StereoRig::vDisplacement = _wtoi(auxBuf);
				EndDialog(hwndDlg,0);
			}
			return TRUE;
		}
		return TRUE;
	case WM_INITDIALOG:
		{
			char auxBuf[256];
			sprintf_s(auxBuf, 256, "%d", StereoRig::hDisplacement);
			SetDlgItemTextA(hwndDlg, IDC_EDIT1, auxBuf);
			sprintf_s(auxBuf, 256, "%d", StereoRig::vDisplacement);
			SetDlgItemTextA(hwndDlg, IDC_EDIT2, auxBuf);
		}
		return TRUE;
	}
	return FALSE;
}

INT_PTR CALLBACK InitializeSystemProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	DBOUT("InitializeSystemProc");
	switch (uMsg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					{
						char strBuffer[64];

						InitializationStruct leftIs, rightIs;
						InitializationStruct::AcquisitionProperties ap;
						InitializationStruct::Authentication auth;

						GetDlgItemTextA(hwndDlg, IDC_EDIT1, strBuffer, 64);
						auth.username = strBuffer;
						GetDlgItemTextA(hwndDlg, IDC_EDIT2, strBuffer, 64);
						auth.password = strBuffer;
						GetDlgItemTextA(hwndDlg, IDC_COMBO4, strBuffer, 64);
						auth.type = (strcmp(strBuffer, "none") == 0 ? InitializationStruct::Authentication::NONE_AUTH : InitializationStruct::Authentication::OPEN_HTTP);

						GetDlgItemTextA(hwndDlg, IDC_COMBO1, strBuffer, 64);
						ap.resolution = strBuffer;
						GetDlgItemTextA(hwndDlg, IDC_COMBO2, strBuffer, 64);
						ap.fps = atoi(strBuffer);
						GetDlgItemTextA(hwndDlg, IDC_COMBO3, strBuffer, 64);
						ap.cameraModel = ap.cameraModel = strBuffer;
						ap.cameraAvailableOptions = AcquisitionCameraFactory::GetRegisteredCameras()[ap.cameraModel].availableOptions;

						GetDlgItemTextA(hwndDlg, IDC_IPADDRESS1, strBuffer, 64);
						leftIs.tcddata.IPAddress = strBuffer;
						GetDlgItemTextA(hwndDlg, IDC_EDIT3, strBuffer, 10);
						leftIs.tcddata.portNumber = atoi(strBuffer);
						leftIs.authentication = auth;
						leftIs.acquisitionProperties = ap;
						leftIs.componentName = "LEFT_SIDE_CAMERA";

						GetDlgItemTextA(hwndDlg, IDC_IPADDRESS2, strBuffer, 64);
						rightIs.tcddata.IPAddress = strBuffer;
						GetDlgItemTextA(hwndDlg, IDC_EDIT4, strBuffer, 10);
						rightIs.tcddata.portNumber = atoi(strBuffer);
						rightIs.authentication = auth;
						rightIs.acquisitionProperties = ap;
						rightIs.componentName = "RIGHT_SIDE_CAMERA";

						AcquisitionCamera *leftCamera = AcquisitionCameraFactory::CreateNewInstance(ap.cameraModel, leftIs);
						AcquisitionCamera *rightCamera = AcquisitionCameraFactory::CreateNewInstance(ap.cameraModel, rightIs);
						
						si->SetStereoRig(new StereoRig(ap, leftCamera, rightCamera, IsDlgButtonChecked(hwndDlg, IDC_CHECK1) == BST_CHECKED));

						si->GetStereoRig()->Start();

						if (!hideVideoInput) {
							VideoEvent ve1;
							ve1.st=NETWORK_STEREO_RIG;
							ve1.eventSource=si->GetStereoRig();
							ve1.ev = STEREO_IMAGE_READY;
							si->GetVideoOutput()->ChangeSource(&ve1);
						}
					}
				case IDCANCEL:
					EndDialog(hwndDlg,0);
					return TRUE;
				case IDC_COMBO3:
					{
						if (HIWORD(wParam) == CBN_SELCHANGE) {
							HWND resCombo = GetDlgItem(hwndDlg, IDC_COMBO1);
							ComboBox_ResetContent(resCombo);
							wchar_t resString[100]; char camDescr[50];
							GetDlgItemTextA(hwndDlg, IDC_COMBO3, camDescr, 50);
							swprintf_s(resString, 100, L"%S", AcquisitionCameraFactory::GetRegisteredCameras()[camDescr].supportedResolutions.c_str());
							wchar_t *resStringPtr = resString;
							while (resStringPtr) {
								wchar_t *nextResPtr = wcsstr(resStringPtr, L";");
								if (nextResPtr)
									nextResPtr[0] = L'\0';
								ComboBox_AddString(resCombo, resStringPtr);
								resStringPtr = (nextResPtr ? nextResPtr+1 : nextResPtr);
							}
							ComboBox_SetCurSel(resCombo, 0);
							int cameraCapabilities = AcquisitionCameraFactory::GetRegisteredCameras()[camDescr].availableOptions;
							BOOL enablingCondition = (cameraCapabilities & AcquisitionCameraFactory::CameraDescription::CAMERA_OPTION_NETWORK);
							EnableWindow(GetDlgItem(hwndDlg, IDC_IPADDRESS1), enablingCondition);
							EnableWindow(GetDlgItem(hwndDlg, IDC_IPADDRESS2), enablingCondition);

							enablingCondition = enablingCondition || !(cameraCapabilities & AcquisitionCameraFactory::CameraDescription::VIRTUAL_CAMERA);
							EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT3), enablingCondition);
							EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT4), enablingCondition);

							enablingCondition = (cameraCapabilities & AcquisitionCameraFactory::CameraDescription::CAMERA_OPTION_AUTHENTICATION);
							EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT1), enablingCondition);
							EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT2), enablingCondition);
							EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO4), enablingCondition);
							
							enablingCondition = (cameraCapabilities & AcquisitionCameraFactory::CameraDescription::CAMERA_OPTION_FPS);
							EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO2), enablingCondition);
							
							enablingCondition = (cameraCapabilities & AcquisitionCameraFactory::CameraDescription::CAMERA_OPTION_RESOLUTION);
							EnableWindow(GetDlgItem(hwndDlg, IDC_COMBO3), enablingCondition);
						}
					}
			}
			break;
		case WM_INITDIALOG:
			{
				SetDlgItemText(hwndDlg, IDC_IPADDRESS1, L"192.168.8.3");
				SetDlgItemText(hwndDlg, IDC_IPADDRESS2, L"192.168.8.2");
				SetDlgItemText(hwndDlg, IDC_EDIT3, L"80");
				SetDlgItemText(hwndDlg, IDC_EDIT4, L"80");
				SetDlgItemText(hwndDlg, IDC_EDIT1, L"root");
				SetDlgItemText(hwndDlg, IDC_EDIT2, L"password");
				HWND combo = GetDlgItem(hwndDlg, IDC_COMBO2);
				ComboBox_AddString(combo, L"10");
				ComboBox_AddString(combo, L"15");
				ComboBox_SetCurSel(combo, 0);
				combo = GetDlgItem(hwndDlg, IDC_COMBO3);
				for (AcquisitionCameraFactory::AcquisitionCameraInstantiatorMap::const_iterator it = AcquisitionCameraFactory::GetRegisteredCameras().begin(); it != AcquisitionCameraFactory::GetRegisteredCameras().end(); it++) {
					SendMessageA(combo, CB_ADDSTRING, 0, (LPARAM) it->first.c_str());
				}
				ComboBox_SetCurSel(combo, 0);
				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_COMBO3, CBN_SELCHANGE), (LPARAM) combo);
				combo = GetDlgItem(hwndDlg, IDC_COMBO4);
				ComboBox_AddString(combo, L"none");
				ComboBox_AddString(combo, L"Open HTTP");
				ComboBox_SetCurSel(combo, 0);
				if (!StereoCalibration::GetInstance()->IsComplete())
					EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK1), FALSE);
				return TRUE;
			}
	}
	return FALSE;
}

void OnCommand(HWND hwnd, int id, HWND hWndCtl, UINT codeNotify) {
	DBOUT("OnCommand");
	wchar_t errMsg[1024];
	char buf[_MAX_PATH];
	switch (id) {
		case ID_SYSTEM_HIDEVIDEOINPUT:
			hideVideoInput = !hideVideoInput;
			CheckMenuItem(hMenu, ID_SYSTEM_HIDEVIDEOINPUT, (hideVideoInput ? MF_CHECKED : MF_UNCHECKED));
			if (si->GetStereoRig()->IsRunning()) {
				if (hideVideoInput) {
					si->GetVideoOutput()->ChangeSource(NULL, NULL);
				} else {
					VideoEvent ve1;
					ve1.st=NETWORK_STEREO_RIG;
					ve1.eventSource=si->GetStereoRig();
					ve1.ev = STEREO_IMAGE_READY;
					si->GetVideoOutput()->ChangeSource(&ve1);
				}
			}
			break;
		case ID_TEST_SAVETRACKSTOFILE:
			{
				saveTracksToFile = !saveTracksToFile;
				CheckMenuItem(hMenu, ID_TEST_SAVETRACKSTOFILE, (saveTracksToFile ? MF_CHECKED : MF_UNCHECKED));
			}
			break;
		case ID_SISTEMA_INITIALIZESYSTEM:
			DialogBox (GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_INITIALIZE), hwnd, InitializeSystemProc);
			break;
		case ID_SYSTEM_ALIGNRIGHTFRAME:
			DialogBox (GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ALIGNIDIALOG), hwnd, AlignRightFrameProc);
			break;
		case ID_TEST_PLATHEARECORDER:
			if (si->GetStereoRig()) {
				PLaTHEARecorderMode selectedMode = PLaTHEA_RECORDER_MODE;
				if (si->GetStereoRig()->GetAcquisitionProperties().cameraAvailableOptions & AcquisitionCameraFactory::CameraDescription::VIRTUAL_CAMERA) {
					selectedMode = PLaTHEA_PLAYER_MODE;
				}
				ShowRecordPlayerDialog(hwnd, selectedMode);
			}
			break;
		case ID_TEST_STARTVIDEORECORDING:
			si->GetStereoRig()->StartRecorderMode();
			break;
		case ID_TEST_STOPVIDEORECORDINGANDSAVE:
			si->GetStereoRig()->StopRecorderMode();
			if (UseCommonItemDialog(buf, sizeof(buf), hwnd, CLSID_FileOpenDialog, FOS_PICKFOLDERS))
				si->GetStereoRig()->SaveRecordingToDirectory(buf);
			break;
		case ID_SYSTEM_STOPACQUISITION:
			{
				si->GetVideoOutput()->ChangeSource(NULL, NULL);
				si->GetStereoRig()->Stop();
				delete si->GetStereoRig()->GetAcquisitionCamera(LEFT_SIDE_CAMERA);
				delete si->GetStereoRig()->GetAcquisitionCamera(RIGHT_SIDE_CAMERA);
				delete si->GetStereoRig();
				si->SetStereoRig(NULL);
			}
			break;
		case ID_SYSTEM_ACQUISITIONSTATS:
			OpenAcquisitionStatisticWindow(hwnd);
			break;
		case ID_SYSTEM_EDITFACEDATABASE:
			if (mainFaceDatabase.PrerequisitesCheck(errMsg, 1024)) {
				DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_FACEDBASE), NULL, FaceDBaseDialogProc);
			} else {
				MessageBox(si->GetMainWindow(), errMsg,	L"Error", MB_ICONERROR);
			}
			break;
		case ID_SYSTEM_ESCI:
			PostQuitMessage(0);
			break;
		case ID_CALIBRATION_INTERNALCALIBRATIONPROCEDURE:
			StereoCalibration::GetInstance()->OpenDialogWindow(hwnd);
			break;
		case ID_CALIBRATION_STARTEXTERNALCALIBRATION:
			ExternalCalibration::GetInstance()->OpenDialogWindow(hwnd);
			break;
		case ID_CALIBRATION_LOADEXTERNALCALIBRATIONDATA:
			if (UseCommonItemDialog(buf, sizeof(buf), hwnd, CLSID_FileOpenDialog, FOS_PICKFOLDERS))
				ExternalCalibration::GetInstance()->LoadFromFolder(buf);
			break;
		case ID_CALIBRATION_SAVEEXTERNALCALIBRATIONDATA:
			if (UseCommonItemDialog(buf, sizeof(buf), hwnd, CLSID_FileOpenDialog, FOS_PICKFOLDERS))
				ExternalCalibration::GetInstance()->SaveToFolder(buf);
			break;
		case ID_LOCALIZATIONENGINE_STARTLOCALIZATIONENGINE:
			{
				int withoutTracking = MessageBox(hwnd, L"Start complete system? Choosing no you will disable tracking functionalities",
					L"Full-feature system?", MB_YESNO | MB_ICONQUESTION);
				ElaborationCore *ec = new ElaborationCore();
				if (withoutTracking == IDNO)
					ec->SetElaborationCoreMode(ElaborationCore::TRACKING_FREE_ELABORATION_CORE_MODE);
				else if (saveTracksToFile) {
					COMDLG_FILTERSPEC filtro[] = {L"Track Log File", L"*.log"};
					if (UseCommonItemDialog(buf, sizeof(buf), hwnd, CLSID_FileSaveDialog, 0, filtro, 1)) {
						ec->ActivateSaveToFileMode(buf);
					}
				}
				if (ec->PrerequisitesCheck(errMsg, 1024)) {
					si->SetElaborationCore(ec);
					si->GetElaborationCore()->Start();
				} else {
					delete ec;
					MessageBox(si->GetMainWindow(), errMsg,	L"Error", MB_ICONERROR);
				}
			}
			break;
		case ID_LOCALIZATIONENGINE_ENDINITIALIZATIONPHASE:
			si->GetElaborationCore()->EndInitializationPhase();
			break;
		case ID_LOCALIZATIONENGINE_STOPLOCALIZATIONENGINE:
			si->SetElaborationCore(NULL);
			break;
		case ID_LOCALIZATIONENGINE_SETENGINEPARAMETERS:
			si->GetElaborationCore()->ShowSettingsWindow();
			break;
		case ID_LOCALIZATIONENGINE_SELECTSVMCLASSIFIER:
			{
				COMDLG_FILTERSPEC filtro[] = {L"SVM Classifier", L"*.xml"};
				if (UseCommonItemDialog(buf, sizeof(buf), hwnd, CLSID_FileOpenDialog, 0, filtro, 1)) {
					DBOUT(buf);
					PlanViewMap::SetSVMClassifierFileName(buf);
					CheckMenuItem(hMenu, ID_LOCALIZATIONENGINE_OPTICALFLOW, MF_UNCHECKED);
					CheckMenuItem(hMenu, ID_LOCALIZATIONENGINE_SVM, MF_CHECKED);
					svmTracker = true;
				}
			}
			break;
		case ID_LOCALIZATIONENGINE_OPTICALFLOW:
			{
				if (svmTracker) {
					CheckMenuItem(hMenu, ID_LOCALIZATIONENGINE_OPTICALFLOW, MF_CHECKED);
					CheckMenuItem(hMenu, ID_LOCALIZATIONENGINE_SVM, MF_UNCHECKED);
					svmTracker = false;
				}
			}
			break;
		case ID_LOCALIZATIONENGINE_SVM:
			{
				if (!svmTracker) {
					CheckMenuItem(hMenu, ID_LOCALIZATIONENGINE_OPTICALFLOW, MF_UNCHECKED);
					CheckMenuItem(hMenu, ID_LOCALIZATIONENGINE_SVM, MF_CHECKED);
					svmTracker = true;
				}
			}
			break;
		case ID_SHOWWINDOW_CURRENTBACKGROUND:
			cvNamedWindow("Background", CV_WINDOW_AUTOSIZE);
			break;
		case ID_SHOWWINDOW_FOREGROUND:
			cvNamedWindow("Foreground Model", CV_WINDOW_AUTOSIZE);
			break;
		case ID_SHOWWINDOW_EDGEACTIVITY:
			cvNamedWindow("Edge Activity", CV_WINDOW_AUTOSIZE);
			break;
		case ID_SHOWWINDOW_RAWDISPARITYMAP:
			cvNamedWindow("Disparity", CV_WINDOW_AUTOSIZE);
			break;
		case ID_SHOWWINDOW_FILTEREDFOREGROUND:
			cvNamedWindow("Connected Components", CV_WINDOW_AUTOSIZE);
			break;
		case ID_SHOWWINDOW_PLANVIEWOCCUPANCYMAP:
			cvNamedWindow("Plan View Occupancy Map", CV_WINDOW_AUTOSIZE);
			break;
		case ID_SHOWWINDOW_PLANVIEWHEIGHTSMAP:
			cvNamedWindow("Plan View Heights Map", CV_WINDOW_AUTOSIZE);
			break;
		case ID_CALIBRATION_EDITROOMSETTINGS:
			RoomSettings::GetInstance()->OpenDialogWindow(hwnd);
			break;
		case ID_SYSTEM_LOADCONFIGURATIONFILE:
			{
				DBOUT("load");
				COMDLG_FILTERSPEC filtro[] = {L"XML Configuration File", L"*.xml"};
				if (UseCommonItemDialog(buf, sizeof(buf), hwnd, CLSID_FileOpenDialog, 0, filtro, 1)) {
					DBOUT(buf);
					HCURSOR originalCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
					char currentDirectory[_MAX_PATH]; strcpy(currentDirectory, buf);
					PathRemoveFileSpecA(currentDirectory); SetCurrentDirectoryA(currentDirectory);
					SettingsPersistence::GetInstance()->LoadFromFile(buf);
					UpdateWindowTitle(hwnd);
					SetCursor(originalCursor);
				}
			}
			break;
		case ID_SYSTEM_SAVECONFIGURATIONFILEAS:
			{
				COMDLG_FILTERSPEC filtro[] = {L"XML Configuration File", L"*.xml"};
				if (UseCommonItemDialog(buf, sizeof(buf), hwnd, CLSID_FileSaveDialog, 0, filtro, 1)) {
					HCURSOR originalCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
					SettingsPersistence::GetInstance()->SaveToFile("PLaTHEAConfiguration", buf);
					UpdateWindowTitle(hwnd);
					SetCursor(originalCursor);
				}
			}
			break;
		case ID_SYSTEM_SAVECONFIGURATIONFILE:
			{
				if (SettingsPersistence::GetInstance()->GetCurrentlyLoadedFile()) {
					HCURSOR originalCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
					SettingsPersistence::GetInstance()->SaveToFile("PLaTHEAConfiguration");
					SetCursor(originalCursor);
				} else {
					COMDLG_FILTERSPEC filtro[] = {L"XML Configuration File", L"*.xml"};
					if (UseCommonItemDialog(buf, sizeof(buf), hwnd, CLSID_FileSaveDialog, 0, filtro, 1)) {
						HCURSOR originalCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
						SettingsPersistence::GetInstance()->SaveToFile(buf);
						UpdateWindowTitle(hwnd);
						SetCursor(originalCursor);
					}
				}
			}
			break;
		case ID_SYSTEM_SELECTHAARCASCADEXML:
			{
				COMDLG_FILTERSPEC filtro[] = {L"XML Haar Cascade descriptor File", L"*.xml"};
				if (UseCommonItemDialog(buf, sizeof(buf), hwnd, CLSID_FileOpenDialog, 0, filtro, 1)) {
					mainFaceDatabase.SetHaarClassifierFileName(buf);
				}
			}
			break;
		case ID_TEST_SYNCHRONIZATIONTEST:
			{
				HWND staticHandle[2] = {hwndLeft, hwndRight};
				OpenSynchronizationTest(si->GetStereoRig(), staticHandle);
			}
			break;
		case ID_TEST_SHOWDEBUGCONSOLE:
			{
				if (Logger::GetCurrentLogger()) {
					Logger::SetCurrentLogger(NULL);
					CheckMenuItem(hMenu, ID_TEST_SHOWDEBUGCONSOLE, MF_UNCHECKED);
				} else {
					Logger::SetCurrentLogger(WindowsConsole::GetWindowsConsole(L"Localization System by Francesco Leotta 2009-2013", hIcon));
					CheckMenuItem(hMenu, ID_TEST_SHOWDEBUGCONSOLE, MF_CHECKED);
					Logger::writeToLOG(L"Localization System by Francesco Leotta 2009-2013\r\n");
				}
			}
			break;
		case ID_TEST_TESTDESIGNER:
			{
				ShowTestDesignerWindow(hwnd);
			}
			break;
		case ID_TEST_POSITIONTEST:
			{
				if (si->GetElaborationCore() == NULL) {
					MessageBox(hwnd, L"Elaboration core has not been started. No software measurements will be available.",
						L"Warning", MB_OK | MB_ICONWARNING);
				} else if (si->GetElaborationCore()->DuringInitializationPhase()) {
					int exitInitPhase = MessageBox(hwnd, L"Elaboration core is still into initialization phase. Tracking will not be performed. Do you want to exit from initialization phase?",
						L"Warning", MB_YESNO | MB_ICONQUESTION);
					if (exitInitPhase == IDYES)
						si->GetElaborationCore()->EndInitializationPhase();
				}
				ShowPositionTestWindow(hwnd);
			}
			break;
		case ID_TEST_STATISTICANALYZER:
			{
				ShowTestAnalyzerWindow(hwnd);
			}
			break;
		case ID_TEST_SVMLEARNING:
			{
				ShowSVMDialog(hwnd);
			}
			break;
	}
}

void OnSize(HWND hwnd, UINT state, int cx, int cy) {
	SendMessage(statusHwnd, WM_SIZE, 0, 0);	
	RECT clientRect, statusBarRect;
	GetClientRect(hwnd, &clientRect);
	GetWindowRect(statusHwnd, &statusBarRect);
	int freeHeight = ((clientRect.bottom-clientRect.top)-(statusBarRect.bottom-statusBarRect.top));
	int freeWidth = (clientRect.right-clientRect.left);
	MoveWindow(hwndLeft, 0, 0, int(freeWidth*0.5f), int(freeHeight),TRUE);
	MoveWindow(hwndRight, int(freeWidth*0.5f), 0, int(freeWidth*0.5f), int(freeHeight),TRUE);
}

void OnClose(HWND hwnd) {
	HCURSOR originalCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
	si->SetElaborationCore(NULL);
	si->GetVideoOutput()->ChangeSource(NULL, NULL);
	if (si->GetStereoRig())
		si->GetStereoRig()->Stop();
	SetCursor(originalCursor);
	PostQuitMessage(0);
}

LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	//DBOUT("MainWindowsProc");
	switch (uMsg) {
		HANDLE_MSG(hwnd, WM_CLOSE, OnClose);
		HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
		HANDLE_MSG(hwnd, WM_SIZE, OnSize);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
	DBOUT("wWinMain");
	hIcon = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON,
					GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);

	//Initialization of Common Control Library
	INITCOMMONCONTROLSEX controlli;
	controlli.dwSize	= sizeof(INITCOMMONCONTROLSEX);
	controlli.dwICC		= ICC_BAR_CLASSES | ICC_STANDARD_CLASSES;
	InitCommonControlsEx (&controlli);

	//WinSOCK2 Initialization
	WSADATA wsadata;
	int iResult = WSAStartup(MAKEWORD(2,2),&wsadata);
	if (iResult) {
		MessageBox(NULL, L"Unable to load WIN32 WinSock2 Dll.", L"Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	serv = new TCPServer(9999);
	serv->Start();

	//Window creation
	WNDCLASS wndclass;
	wndclass.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wndclass.hIcon = hIcon;
	wndclass.hInstance = hInstance;
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszMenuName = NULL;
	wndclass.lpfnWndProc = MainWindowProc;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpszClassName = strAppName;
	if (!RegisterClass(&wndclass)) {
		MessageBox(NULL, L"This program requires Windows NT", L"Error", MB_ICONERROR);
		return 0;
	}

	WIN32_FIND_DATA wfd;
	wchar_t platheaDIR[_MAX_PATH]; GetPLaTHEATempPath(platheaDIR, _MAX_PATH);
	HANDLE hTempDirectory = FindFirstFile(platheaDIR, &wfd);
	if (hTempDirectory == INVALID_HANDLE_VALUE) {
		CreateDirectory(platheaDIR, NULL);
	} else {
		FindClose(hTempDirectory);
	}

	HWND hwnd = CreateWindowEx(0L, strAppName, mainWindowBaseTitle,
		WS_OVERLAPPEDWINDOW /*& ~WS_THICKFRAME & ~WS_MAXIMIZEBOX*/, CW_USEDEFAULT, CW_USEDEFAULT, 700,
		350, NULL, hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU)), hInstance, NULL);

	hwndLeft = CreateWindowEx(WS_EX_CLIENTEDGE, L"STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_NOTIFY,
		0, 0, 320, 240, hwnd, NULL, hInstance, NULL);
	hwndRight = CreateWindowEx(WS_EX_CLIENTEDGE, L"STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_NOTIFY,
		320, 0, 320, 240, hwnd, NULL, hInstance, NULL);

	statusHwnd = CreateWindowEx (0L, STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hwnd,
		(HMENU) ID_STATUS_BAR, hInstance, NULL);
	int statusCoord[] = {100, -1};
	SendMessage(statusHwnd, (UINT) SB_SETPARTS, (WPARAM) 2, (LPARAM) statusCoord);
	SendMessage(statusHwnd, (UINT) SB_SETTEXT, (WPARAM) 1, (LPARAM) L"Localization and Tracking Application");

	si = new SystemInfo(statusHwnd, hwnd, hwndLeft, hwndRight);
	ApplicationWorkFlow::GetInstance()->UpdateSystemState(APPLICATION_STARTED);

	ShowWindow(hwnd, SW_SHOW);

	//Accelerators allow us to use keys combination to call menu item
	HACCEL hAccel = LoadAccelerators (hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR1));

	//Message Loop
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (hActiveModelessWindow == NULL || !IsDialogMessage(hActiveModelessWindow, &msg)) {
			if (!TranslateAccelerator (hwnd, hAccel, &msg)) {
				TranslateMessage (&msg);
				DispatchMessage (&msg);
			}
		}
	}

	serv->Stop();
	delete serv;

	//WinSOCK2 Resource release
	WSACleanup();

	FreeConsole();
	DestroyIcon(hIcon);

	return (int) msg.wParam;
}