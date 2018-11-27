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

#include "PositionTestDialog.h"
#include "TestAnalyzerDialog.h"
#include <LeoWindowsRawMouse.h>
#include <Uxtheme.h>
#include <LeoWindowsGDI.h>
#include <LeoLog4CPP.h>
#include <LeoWindowsGUI.h>
#include "video_output.h"
#include "TestAnalyzer.h"

using namespace leostorm::logging;

extern SystemInfo *si;

//eventToWait[0] is raised when new software measurements are available
static HANDLE eventToWait[] = {CreateEvent(NULL, FALSE, FALSE, NULL), CreateEvent(NULL, FALSE, FALSE, NULL)};

RWLock notYetAnalyzedDataLock;
SoftwareMeasurementsVector notYetAnalyzedData;

void RegisterNewSoftwareMeasurements(std::vector<TrackedObject *> &data, DWORD measurementTimestamp) {
	DWORD currentTimeStamp = GetTickCount();
	PositionTestMeasuredEntry ptme;
	ptme.timestamp = measurementTimestamp;
	notYetAnalyzedDataLock.AcquireWriteLock();
	for (std::vector<TrackedObject *>::const_iterator it = data.begin(); it != data.end(); it++) {
		ptme.max_height = (*it)->kalmanFilter->state_post->data.fl[4];
		ptme.x_position = ((*it)->kalmanFilter->state_post->data.fl[0]);
		ptme.y_position = ((*it)->kalmanFilter->state_post->data.fl[1]);
		ptme.x_velocity = ((*it)->kalmanFilter->state_post->data.fl[2]);
		ptme.y_velocity = ((*it)->kalmanFilter->state_post->data.fl[3]);
		ptme.x_base = float(((*it)->bottomRight.x) - ((*it)->upperLeft.x));
		ptme.y_base = float(((*it)->bottomRight.y) - ((*it)->upperLeft.y));
		ptme.type = (*it)->type;
		ptme.nameID = (*it)->nameID;
		ptme.trackingID = ((*it)->ID);
		
		notYetAnalyzedData.push_back(ptme);
	}
	notYetAnalyzedDataLock.ReleaseWriteLock();
	SetEvent(eventToWait[0]);
}

static HWND testPositionHwnd = NULL;
static HIMAGELIST hSmall = NULL;
static HIMAGELIST hSmallMouseActivity = NULL;

static HWND currentlyTabbedWindow = NULL;

static HWND destFrame = NULL;
static HWND backgroundStaticHwnd = NULL;
static HWND leftLittleStaticHwnd = NULL;
static HWND rightLittleStaticHwnd = NULL;

static HWND numberOfSubjectEdit = NULL;
static HWND numberOfSubjectSpin = NULL;
static HWND testList = NULL;
static HWND pathList = NULL;
static HWND userMouseList = NULL;

static HWND offlineTestList = NULL;
static HWND offlineTestDetailList = NULL;

static std::unordered_map<int, int> selectedPathsImage;

static PositionTest currentlySelectedTest;
PositionTest *GetCurrentlySelectedTest() {
	return &currentlySelectedTest;
}

std::vector<std::vector<int>> pathDispositions;

static IplImage *currentImage = 0; static HBITMAP hBitmap = NULL; static HDC memDC = NULL;

void FillPossibleTests(int numberOfSubjects, HWND selectedTestPaths, HWND testList, std::unordered_map<int, int> &selectedPathsImage) {
	ListView_DeleteAllItems(testList);
	while (ListView_DeleteColumn(testList, 0)) {}

	wchar_t wideBuffer[256];
	char normalBuffer[256];

	LVCOLUMN lc;
	lc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lc.fmt = LVCFMT_LEFT;

	lc.cx = 60;
	lc.pszText = L"ID";
	lc.iSubItem = 0;
	ListView_InsertColumn(testList, 0, &lc);

	for (int i = 1; i <= numberOfSubjects; i++) {
		swprintf_s(wideBuffer, 32, L"User %d", i);
		lc.pszText = wideBuffer;
		lc.iSubItem = i;

		ListView_InsertColumn(testList, i, &lc);
	}

	int numberOfSelectedPaths = ListBox_GetSelCount(selectedTestPaths);
	if (numberOfSelectedPaths > 0) {
		std::vector<int> selectedPaths;
		int *selectedPathsIndexes = new int[numberOfSelectedPaths];
		ListBox_GetSelItems(selectedTestPaths, numberOfSelectedPaths, selectedPathsIndexes);
		for (int i = 0; i < numberOfSelectedPaths; i++) {
			SendMessageA(selectedTestPaths, LB_GETTEXT, (WPARAM) selectedPathsIndexes[i], (LPARAM) normalBuffer);
			selectedPaths.push_back(mainTestDesigner.FindPathIndexByName(normalBuffer));
		}

		pathDispositions.clear();
		Dispositions(selectedPaths, numberOfSubjects, pathDispositions);

		int j = 0;
		for (std::vector<std::vector<int>>::const_iterator it = pathDispositions.begin(); it != pathDispositions.end(); it++, j++) {
			LVITEM li;
			li.mask = LVIF_TEXT | LVIF_IMAGE;
			li.state = 0; li.stateMask = 0;
			li.pszText = wideBuffer;
			li.cchTextMax = 20;
			li.iSubItem = 0;
			li.iImage = -1;

			li.iItem = j;

			swprintf_s(wideBuffer, 20, L"%d", j);
			ListView_InsertItem(testList, &li);

			for (int i = 0; i < numberOfSubjects; i++) {
				swprintf_s(wideBuffer, 20, L"%S", mainTestDesigner[(*it)[i]].pathName);
				li.iSubItem = i + 1;
				li.iImage = selectedPathsImage[(*it)[i]];
				ListView_SetItem(testList, &li);
			}
		}
	}
}

void FillUserMouseAssociations (HWND userMouseList, bool createColumns) {
	ListView_DeleteAllItems(userMouseList);

	if (createColumns) {
		LVCOLUMN lc;
		lc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lc.fmt = LVCFMT_LEFT;

		lc.cx = 60;
		lc.pszText = L"MouseID";
		lc.iSubItem = 0;
		ListView_InsertColumn(userMouseList, 0, &lc);

		lc.cx = 180;
		lc.pszText = L"Mouse Name";
		lc.iSubItem = 1;
		ListView_InsertColumn(userMouseList, 1, &lc);

		lc.cx = 60;
		lc.pszText = L"User";
		lc.iSubItem = 2;
		ListView_InsertColumn(userMouseList, 2, &lc);

		lc.cx = 180;
		lc.pszText = L"Registered Subject";
		lc.iSubItem = 3;
		ListView_InsertColumn(userMouseList, 3, &lc);
	}

	wchar_t wideBuffer[64];
	LVITEM li;
	li.mask = LVIF_TEXT | LVIF_IMAGE;
	li.state = 0; li.stateMask = 0;
	li.pszText = wideBuffer;
	li.cchTextMax = 64;

	std::vector<int> suppressedMice;
	
	int numberOfMice = int(standardRawMouseMessageDispatcher.GetInstalledMice().size());
	for (int i = 0; i < numberOfMice; i++) {
		if (standardRawMouseMessageDispatcher.GetInstalledMice()[i].suppressSystemEvents) {
			suppressedMice.push_back(i);
		}
		swprintf_s(wideBuffer, 64, L"%d", i);
		li.iItem = i;
		li.iSubItem = 0;
		li.iImage = -1;
		ListView_InsertItem(userMouseList, &li);

		li.iSubItem = 1;
		li.iImage = 0;
		swprintf_s(wideBuffer, 64, L"%s", wcsstr(standardRawMouseMessageDispatcher.GetInstalledMice()[i].mouseName, L";") + 1);
		ListView_SetItem(userMouseList, &li);

		li.iSubItem = 2;
		li.iImage = -1;
		int associatedUser = standardRawMouseMessageDispatcher.GetInstalledMice()[i].associatedUser;
		if (associatedUser == 0)
			swprintf_s(wideBuffer, 64, L"<none>");
		else
			swprintf_s(wideBuffer, 64, L"User %d", associatedUser);
		ListView_SetItem(userMouseList, &li);

		li.iSubItem = 3;
		li.iImage = -1;
		int associatedSubject = standardRawMouseMessageDispatcher.GetInstalledMice()[i].associatedSubject;
		if (associatedSubject == -1)
			swprintf_s(wideBuffer, 64, L"<none>");
		else
			swprintf_s(wideBuffer, 64, L"%S", mainFaceDatabase[associatedSubject].name);
		ListView_SetItem(userMouseList, &li);
	}
	numberOfMice = (int) suppressedMice.size();
	for (int i = 0; i < numberOfMice; i++) {
		ListView_SetCheckState(userMouseList, suppressedMice[i], TRUE);
	}
}

void RawMouseCallback(DWORD eventTime, int mouseIndex, UINT message) {
	static int lastClickedMouse = 0;
	static bool lastLeftButtonClicked = true;
	if (message == NEW_MOUSE_CONNECTED || message == MOUSE_REMOVED)
		FillUserMouseAssociations(userMouseList, false);
	else if (message == RAW_MOUSE_LEFT_BUTTON_DOWN || message == RAW_MOUSE_LEFT_BUTTON_UP || message == RAW_MOUSE_RIGHT_BUTTON_DOWN || message == RAW_MOUSE_RIGHT_BUTTON_UP) {
		LVITEM li;
		li.iItem = (mouseIndex >= 0 ? mouseIndex : lastClickedMouse);
		li.iSubItem = 1;
		li.mask = LVIF_IMAGE;
		li.state = 0; li.stateMask = 0;
		ListView_GetItem(userMouseList, &li);
		if (message == RAW_MOUSE_LEFT_BUTTON_DOWN) {
			li.iImage = 1;
			if (mouseIndex >= 0) lastLeftButtonClicked = true;
		} else if (message == RAW_MOUSE_RIGHT_BUTTON_DOWN) {
			li.iImage = 2;
			if (mouseIndex >= 0) lastLeftButtonClicked = false;
		} else
			li.iImage = 0;
		ListView_SetItem(userMouseList, &li);
		if (mouseIndex == -1) {
			if (lastLeftButtonClicked)
				standardRawMouseMessageDispatcher.GetInstalledMice()[lastClickedMouse].leftButtonState = false;
			else
				standardRawMouseMessageDispatcher.GetInstalledMice()[lastClickedMouse].rightButtonState = false;
		}
		if (mouseIndex >= 0) lastClickedMouse = mouseIndex;
	} else if (message == RAW_INPUT_MANAGEMENT_STARTED)
		Logger::writeToLOG(L"Raw Input Management Started\r\n");
	else if (message == RAW_INPUT_MANAGEMENT_ENDED)
		Logger::writeToLOG(L"Raw Input Management Ended\r\n");
	else if (message == RAW_MOUSE_LEFT_CLICK) {
		if (currentlySelectedTest.IsTestStarted()) {
			int correspondentUser = standardRawMouseMessageDispatcher.GetInstalledMice()[mouseIndex].associatedUser;
			if (correspondentUser > 0 && currentlySelectedTest.GetGroundTruthMeasurements(correspondentUser).size() < mainTestDesigner[currentlySelectedTest[correspondentUser - 1]].size()) {
				currentlySelectedTest.GetGroundTruthMeasurements(correspondentUser).push_back(GetTickCount() - currentlySelectedTest.GetStartTime());
				currentlySelectedTest.UpdateImage(currentImage, mainTestDesigner, PositionTest::SHOW_PATH | PositionTest::SHOW_CURRENT_POINT);
				HWND staticControl = GetDlgItem(testPositionHwnd, IDC_POSITIONTESTSTATIC);
				hBitmap = IplImage2HBITMAP(staticControl, hBitmap, currentImage);
				RedrawStaticControl(&staticControl, 1);
			}
		}
	}
}

INT_PTR CALLBACK NewPositionTestProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_ENABLE:
		{
			BOOL bEnable = (BOOL) wParam;
			EnableWindow(testList, bEnable);
			EnableWindow(pathList, bEnable);
			EnableWindow(userMouseList, bEnable);
			EnableWindow(numberOfSubjectSpin, bEnable);
			EnableWindow(GetDlgItem(hwndDlg, IDC_ONLINETEST), bEnable);
			EnableWindow(GetDlgItem(hwndDlg, IDC_OFFLINETEST), bEnable && si->GetElaborationCore() && si->GetElaborationCore()->GetElaborationCoreMode() == ElaborationCore::TRACKING_FREE_ELABORATION_CORE_MODE);
		}
		return TRUE;
	case WM_INITDIALOG:
		{
			numberOfSubjectEdit = GetDlgItem(hwndDlg, IDC_NUMBEROFSUBJECTSEDIT);
			numberOfSubjectSpin = GetDlgItem(hwndDlg, IDC_NUMBEROFSUBJECTSSPIN);
			testList = GetDlgItem(hwndDlg, IDC_TESTLIST);
			pathList = GetDlgItem(hwndDlg, IDC_PATHLIST);
			userMouseList = GetDlgItem(hwndDlg, IDC_USERMOUSELIST);

			SendMessage(numberOfSubjectSpin, UDM_SETBUDDY, (WPARAM) numberOfSubjectEdit, 0);
			SendMessage(numberOfSubjectSpin, UDM_SETRANGE, 0, MAKELPARAM(1, 1));
			SendMessage(numberOfSubjectSpin, UDM_SETPOS, 0, 1);

			DWORD dwStyle = (DWORD) SendMessage(testList, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
			dwStyle |= (LVS_EX_SUBITEMIMAGES | LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);
			SendMessage(testList,LVM_SETEXTENDEDLISTVIEWSTYLE,0,dwStyle);
			SetWindowTheme(testList, L"Explorer", NULL);

			dwStyle = (DWORD) SendMessage(userMouseList, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
			dwStyle |= (LVS_EX_CHECKBOXES | LVS_EX_SUBITEMIMAGES | LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);
			SendMessage(userMouseList,LVM_SETEXTENDEDLISTVIEWSTYLE,0,dwStyle);
			SetWindowTheme(userMouseList, L"Explorer", NULL);

			FillPossibleTests(LOWORD(SendMessage(numberOfSubjectSpin, UDM_GETPOS, 0, 0)), pathList, testList, selectedPathsImage);
			FillUserMouseAssociations(userMouseList, true);

			ListView_SetImageList(testList, hSmall, LVSIL_SMALL);
			ListView_SetImageList(userMouseList, hSmallMouseActivity, LVSIL_SMALL);

			for (std::unordered_map<int, TestPath>::const_iterator it = mainTestDesigner.begin(); it != mainTestDesigner.end(); it++) {
				SendMessageA(pathList, LB_ADDSTRING, 0, (LPARAM) it->second.pathName);
			}

			Button_SetCheck(GetDlgItem(hwndDlg, IDC_ONLINETEST), BST_CHECKED);

			if (si->GetElaborationCore() == NULL || si->GetElaborationCore()->GetElaborationCoreMode() == ElaborationCore::FULL_FEATURE_ELABORATION_CORE_MODE)
				EnableWindow(GetDlgItem(hwndDlg, IDC_OFFLINETEST), FALSE);

			standardRawMouseMessageDispatcher.rawMouseEventListener = RawMouseCallback;
			standardRawMouseMessageDispatcher.StartRawMessageAnalysis(testPositionHwnd);
		}
		return TRUE;
	case WM_DESTROY:
		{
			standardRawMouseMessageDispatcher.StopRawMessageAnalysis();
			standardRawMouseMessageDispatcher.rawMouseEventListener = NULL;
		}
		return TRUE;
	case WM_NOTIFY:
		{
			NMHDR *notificationCode = (NMHDR *) lParam;
			if (notificationCode->code == UDN_DELTAPOS) {
				NMUPDOWN *updownstate = (NMUPDOWN *) lParam;
				if (notificationCode->idFrom == IDC_NUMBEROFSUBJECTSSPIN) {
					int selectedNumberOfUsers = MAX(1, MIN(LOWORD(SendMessage(numberOfSubjectSpin, UDM_GETRANGE, 0, 0)),
						updownstate->iPos + updownstate->iDelta));
					FillPossibleTests(selectedNumberOfUsers, pathList, testList, selectedPathsImage);
					FillUserMouseAssociations(userMouseList, false);
				}
				return TRUE;
			} else if (notificationCode->code == NM_CLICK) {
				NMITEMACTIVATE *activatePtr = (NMITEMACTIVATE *) lParam;
				if (notificationCode->idFrom == IDC_TESTLIST) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_SAVETESTTOREPOSITORY), FALSE);
					if (activatePtr->iItem != -1) {
						currentlySelectedTest = pathDispositions[activatePtr->iItem];
						if (IsDlgButtonChecked(hwndDlg, IDC_OFFLINETEST) == BST_CHECKED)
							currentlySelectedTest.SetPositionTestType(PositionTest::OFFLINE_POSITION_TEST);
						else if (IsDlgButtonChecked(hwndDlg, IDC_ONLINETEST) == BST_CHECKED)
							currentlySelectedTest.SetPositionTestType(PositionTest::ONLINE_POSITION_TEST);
					} else {
						currentlySelectedTest.clear();
					}
					currentlySelectedTest.UpdateImage(currentImage, mainTestDesigner);
					hBitmap = IplImage2HBITMAP(destFrame, hBitmap, currentImage);
					RedrawStaticControl(&destFrame, 1);
				}
				if (activatePtr->iItem != -1)
					RawMouseCallback(GetTickCount(), -1, RAW_MOUSE_LEFT_BUTTON_UP); //Workaround...ListView does not produce WM_LBUTTONUP nor WM_RBUTTONUP after a performed selection
				return TRUE;
			} else if (notificationCode->code == NM_RCLICK) {
				NMITEMACTIVATE *activatePtr = (NMITEMACTIVATE *) lParam;
				if (notificationCode->idFrom == IDC_USERMOUSELIST) {
					HMENU hPopupMenu = CreatePopupMenu();
					wchar_t menuItemText[50];
					MENUITEMINFO mii; memset(&mii, 0, sizeof(mii));
					mii.cbSize = (UINT) sizeof(mii);
					mii.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE;
					mii.dwTypeData = menuItemText;
					mii.fState = MFS_ENABLED;
					mii.wID = WM_USER;
					swprintf_s(menuItemText, 50, L"<none>");
					int itemToCheck = 0;
					InsertMenuItem(hPopupMenu, 0, TRUE, &mii);
					
					UINT numberOfSubjects = ListBox_GetCount(pathList);
					int availableSubjects = 0;
					for (UINT i = 0; i < numberOfSubjects; i++) {
						int associatedMouse = standardRawMouseMessageDispatcher.GetMouseIndexByUser(i + 1);
						if (associatedMouse == -1 || associatedMouse == activatePtr->iItem) {
							mii.wID = WM_USER + i + 1;
							swprintf_s(menuItemText, 50, L"User %d", (int) i + 1);
							availableSubjects++;
							InsertMenuItem(hPopupMenu, availableSubjects, TRUE, &mii);
							if (associatedMouse == activatePtr->iItem)
								itemToCheck = availableSubjects;
						}
					}
					CheckMenuRadioItem(hPopupMenu, 0, availableSubjects, itemToCheck, MF_BYPOSITION);

					mii.fMask = MIIM_STATE | MIIM_TYPE;
					mii.fType = MFT_SEPARATOR;
					InsertMenuItem(hPopupMenu, availableSubjects + 1, TRUE, &mii);

					mii.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE;
					mii.fType = 0;
					mii.wID = WM_USER + numberOfSubjects + 1;
					swprintf_s(menuItemText, 50, L"<none>");
					itemToCheck = availableSubjects + 2;
					InsertMenuItem(hPopupMenu, itemToCheck, TRUE, &mii);

					int numberOfPersons = 0;
					for (std::unordered_map<int, Person>::const_iterator it = mainFaceDatabase.begin(); it != mainFaceDatabase.end(); it++) {
						int associatedMouse = standardRawMouseMessageDispatcher.GetMouseIndexBySubject(it->first);
						if (associatedMouse == -1 || associatedMouse == activatePtr->iItem) {
							mii.wID = WM_USER + numberOfSubjects + 2 + UINT(it->first);
							swprintf_s(menuItemText, 50, L"%S", it->second.name);
							int menuPosition = availableSubjects + 2 + (++numberOfPersons);
							InsertMenuItem(hPopupMenu, menuPosition, TRUE, &mii);
							if (associatedMouse == activatePtr->iItem)
								itemToCheck = menuPosition;
						}
					}
					CheckMenuRadioItem(hPopupMenu, availableSubjects + 2, availableSubjects + 2 + numberOfPersons, itemToCheck, MF_BYPOSITION);
					
					if (activatePtr->iItem != -1)
						RawMouseCallback(GetTickCount(), -1, RAW_MOUSE_LEFT_BUTTON_UP); //Workaround...ListView does not produce WM_LBUTTONUP nor WM_RBUTTONUP after a performed selection
					SetForegroundWindow(hwndDlg);
					POINT cursorPos; GetCursorPos(&cursorPos);
					UINT trackResult = (UINT) TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RETURNCMD | TPM_NONOTIFY, cursorPos.x, cursorPos.y,
						0, hwndDlg, NULL);
					if (trackResult) {
						int selectionID = (int) (trackResult - WM_USER);
						if (selectionID >= 0 && selectionID <= int(numberOfSubjects)) {
							standardRawMouseMessageDispatcher.GetInstalledMice()[activatePtr->iItem].associatedUser = selectionID;
						} else {
							standardRawMouseMessageDispatcher.GetInstalledMice()[activatePtr->iItem].associatedSubject = (selectionID - int(numberOfSubjects) - 2);
						}
						FillUserMouseAssociations(userMouseList, false);
					}
					DestroyMenu(hPopupMenu);
				}
				if (activatePtr->iItem != -1)
					RawMouseCallback(GetTickCount(), -1, RAW_MOUSE_LEFT_BUTTON_UP); //Same workaround as before
				return TRUE;
			} else if (notificationCode->code == LVN_ITEMCHANGED) {
				if (notificationCode->idFrom == IDC_USERMOUSELIST) {
					NMLISTVIEW *itemState = (NMLISTVIEW *) lParam;
					if (itemState->uNewState & LVIS_STATEIMAGEMASK) {
						standardRawMouseMessageDispatcher.GetInstalledMice()[itemState->iItem].suppressSystemEvents = (ListView_GetCheckState(userMouseList, itemState->iItem) ? true : false);
					}
					return TRUE;
				}
			}
		}
		break;
	case WM_COMMAND:
		{
			WORD messageId = HIWORD(wParam);
			WORD controlId = LOWORD(wParam); 
			if (messageId == LBN_SELCHANGE) {
				if (controlId == IDC_PATHLIST) {
					int numberOfSubjects = LOWORD(SendMessage(numberOfSubjectSpin, UDM_GETPOS, 0, 0));
					int numberOfPaths = ListBox_GetSelCount(pathList);

					int newNumberOfSubjects = MIN(MAX(numberOfPaths, 1), numberOfSubjects);
					SendMessage(numberOfSubjectSpin, UDM_SETPOS, 0, newNumberOfSubjects);
					SendMessage(numberOfSubjectSpin, UDM_SETRANGE, 0, MAKELPARAM(MAX(1, numberOfPaths), 1));
					UpdateWindow(numberOfSubjectSpin);

					FillPossibleTests(newNumberOfSubjects, pathList, testList, selectedPathsImage);
					return TRUE;
				}
			} else if (messageId == BN_CLICKED) {
				if (controlId == IDC_OFFLINETEST || controlId == IDC_ONLINETEST) {
					if (currentlySelectedTest.GetPositionTestType() != PositionTest::NOT_INITIALIZED_TEST)
						currentlySelectedTest.SetPositionTestType(controlId == IDC_OFFLINETEST ? PositionTest::OFFLINE_POSITION_TEST : PositionTest::ONLINE_POSITION_TEST);
					return TRUE;
				}
			}
		}
		break;

	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORBTN:
		{
			return (INT_PTR) GetStockObject(WHITE_BRUSH);
		}
	}
	return FALSE;
}

INT_PTR CALLBACK CompletePositionTestProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_ENABLE:
		{
			BOOL bEnable = (BOOL) wParam;
			EnableWindow(offlineTestList, bEnable);
			EnableWindow(offlineTestDetailList, bEnable);
			EnableWindow(GetDlgItem(hwndDlg, IDC_DELETEOFFLINETEST), bEnable);
		}
		return TRUE;
	case WM_INITDIALOG:
		{
			offlineTestList = GetDlgItem(hwndDlg, IDC_OFFLINETESTLIST);
			offlineTestDetailList = GetDlgItem(hwndDlg, IDC_OFFLINETESTDETAILS);

			DWORD dwStyle = (DWORD) SendMessage(offlineTestList, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
			dwStyle |= (LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);
			SendMessage(offlineTestList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwStyle);
			SetWindowTheme(offlineTestList, L"Explorer", NULL);

			ListView_SetImageList(offlineTestDetailList, hSmall, LVSIL_SMALL);
			dwStyle = (DWORD) SendMessage(offlineTestDetailList, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
			dwStyle |= (LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);
			SendMessage(offlineTestDetailList, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dwStyle);
			SetWindowTheme(offlineTestDetailList, L"Explorer", NULL);

			FillStoredExperimentList(offlineTestList, &GetPositionTestVector(), true,
				PositionTest::OFFLINE_POSITION_TEST, mainTestDesigner);
			SelectExperiment(offlineTestDetailList, NULL, NULL, true, false);
		}
		return TRUE;
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
	case WM_CTLCOLORBTN:
		{
			return (INT_PTR) GetStockObject(WHITE_BRUSH);
		}
	case WM_NOTIFY:
		{
			NMHDR *notificationCode = (NMHDR *) lParam;
			if (notificationCode->code == NM_CLICK) {
				NMITEMACTIVATE *activatePtr = (NMITEMACTIVATE *) lParam;
				if (notificationCode->idFrom == IDC_OFFLINETESTLIST) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_SAVETESTTOREPOSITORY), FALSE);
					if (activatePtr->iItem != -1) {
						wchar_t selectedTestName[256]; char selectedTestNameShort[256];
						ListView_GetItemText(offlineTestList, activatePtr->iItem, 0, selectedTestName, 256);
						sprintf_s(selectedTestNameShort, "%S", selectedTestName);
						currentlySelectedTest = GetPositionTestVector()[selectedTestNameShort];
						currentlySelectedTest.SetPositionTestType(PositionTest::COMPLETION_POSITION_TEST);
						swprintf_s(selectedTestName, L"%s - complete", selectedTestName);
						SetDlgItemText(testPositionHwnd, IDC_TESTNAMEEDIT, selectedTestName);
					} else {
						currentlySelectedTest.clear();
						SetDlgItemText(testPositionHwnd, IDC_TESTNAMEEDIT, L"");
					}
					currentlySelectedTest.UpdateImage(currentImage, mainTestDesigner);
					hBitmap = IplImage2HBITMAP(destFrame, hBitmap, currentImage);
					RedrawStaticControl(&destFrame, 1);
					SelectExperiment(offlineTestDetailList, &currentlySelectedTest, &selectedPathsImage, false, false);
					SetDlgItemTextA(hwndDlg, IDC_ADDITIONALDATADIRECTORYEDIT, currentlySelectedTest.GetAdditionalDataDirectory());
					return TRUE;
				}
			}
		}
		break;
	case WM_COMMAND:
		{
			WORD messageId = HIWORD(wParam);
			WORD controlId = LOWORD(wParam);
			if (messageId == BN_CLICKED) {
				if (controlId = IDC_DELETEOFFLINETEST) {
					std::vector<int> selectedIndexes; GetListViewSelectedIndexes(offlineTestList, selectedIndexes);
					if (selectedIndexes.size()) {
						wchar_t wideBuf[256]; char normalBuf[256];
						ListView_GetItemText(offlineTestList, selectedIndexes[0], 0, wideBuf, 256);
						sprintf_s(normalBuf, "%S", wideBuf);
						GetPositionTestVector().erase(GetPositionTestVector().find(normalBuf));
						FillStoredExperimentList(offlineTestList, &GetPositionTestVector(), false,
							PositionTest::OFFLINE_POSITION_TEST, mainTestDesigner);
						currentlySelectedTest.clear();
						currentlySelectedTest.UpdateImage(currentImage, mainTestDesigner);
						hBitmap = IplImage2HBITMAP(destFrame, hBitmap, currentImage);
						RedrawStaticControl(&destFrame, 1);
						SelectExperiment(offlineTestDetailList, &currentlySelectedTest, &selectedPathsImage, false, false);
					}
					return TRUE;
				}
			}
		}
		break;
	}
	return FALSE;
}

INT_PTR CALLBACK PositionTestProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static bool doClipRect = false;
	destFrame = GetDlgItem(hwndDlg, IDC_POSITIONTESTSTATIC);
	backgroundStaticHwnd = GetDlgItem(hwndDlg, IDC_BACKGROUNDSTATIC);
	leftLittleStaticHwnd = GetDlgItem(hwndDlg, IDC_LEFTLITTLESTATIC);
	rightLittleStaticHwnd = GetDlgItem(hwndDlg, IDC_RIGHTLITTLESTATIC);
	HWND tabControl = GetDlgItem(hwndDlg, IDC_POSITIONTESTTAB);
	switch (uMsg) {
	case WM_ACTIVATE:
		DBOUT("WM_ACTIVATE");
		if (wParam != 0 && doClipRect) {
			PerformClipCursor(hwndDlg, false);
		}
		SetWindowLong(hwndDlg, DWLP_MSGRESULT, 0);
		return TRUE;
	case WM_EXITSIZEMOVE:
		{
			if (doClipRect) {
				PerformClipCursor(hwndDlg, false);
			}
		}
		SetWindowLong(hwndDlg, DWLP_MSGRESULT, 0);
		return TRUE;
	case WM_INITDIALOG:
		{
			testPositionHwnd = hwndDlg;
			EnableWindow(GetParent(hwndDlg), FALSE); 

			RECT imageClientRect; GetClientRect(destFrame, &imageClientRect);
			currentImage = cvCreateImage(cvSize(imageClientRect.right, imageClientRect.bottom), IPL_DEPTH_8U, 3);
			hBitmap = NULL;
			HDC frameDC = GetDC(destFrame);
			memDC = CreateCompatibleDC(frameDC);
			ReleaseDC(destFrame, frameDC);

			Button_SetCheck(GetDlgItem(hwndDlg, IDC_BACKGROUNDMODELCHECK), BST_CHECKED);
			Button_SetCheck(GetDlgItem(hwndDlg, IDC_FOREGROUNDCHECK), BST_CHECKED);

			wchar_t *folderName;
			SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &folderName);
			SetDlgItemText(hwndDlg, IDC_DATADIRECTORYEDIT, folderName);
			CoTaskMemFree(folderName);

			currentlySelectedTest.clear();

			int iconWidth = GetSystemMetrics(SM_CXSMICON), iconHeight = GetSystemMetrics(SM_CYSMICON);

			HDC listViewDC = GetDC(NULL);
			BITMAPINFO bi;
			memset(&bi, 0, sizeof(bi));
			bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bi.bmiHeader.biWidth = iconWidth;
			bi.bmiHeader.biHeight = iconHeight;
			bi.bmiHeader.biPlanes = 1;
			bi.bmiHeader.biBitCount = (unsigned short) 24;
			bi.bmiHeader.biCompression = BI_RGB;

			BYTE *colorArray;
			HBITMAP hBitmap = CreateDIBSection(listViewDC, &bi, DIB_RGB_COLORS, (void **) &colorArray, NULL, NULL);
			HBITMAP hBlackMask = CreateBitmap(iconWidth, iconHeight, 1, 1, NULL);
			
			ICONINFO ii;
			memset(&ii, 0, sizeof(ii));
			ii.fIcon = TRUE;
			ii.hbmColor = hBitmap;
			ii.hbmMask = hBlackMask;

			hSmallMouseActivity = ImageList_Create(iconWidth, iconHeight, ILC_COLOR24, 1, 1);

			COLORREF mouseActivityColors[3] = {RGB(0, 0, 0), RGB(255, 0, 0), RGB(0, 255, 0)};
			for (int c = 0; c < 3; c++) {
				BYTE *colorPtr = colorArray;
				for (int i = 0; i < iconHeight; i++) {
					for (int j = 0; j < iconWidth; j++, colorPtr+=3) {
						colorPtr[0] = GetBValue(mouseActivityColors[c]);
						colorPtr[1] = GetGValue(mouseActivityColors[c]);
						colorPtr[2] = GetRValue(mouseActivityColors[c]);
					}
				}
				HICON hIcon = CreateIconIndirect(&ii);
				ImageList_AddIcon(hSmallMouseActivity, hIcon);
				DestroyIcon(hIcon);
			}

			hSmall = ImageList_Create(iconWidth, iconHeight, ILC_COLOR24, 1, 1);

			selectedPathsImage.clear();
			int j = 0;
			for (std::unordered_map<int, TestPath>::const_iterator it = mainTestDesigner.begin(); it != mainTestDesigner.end(); it++, j++) {
				BYTE *colorPtr = colorArray;
				for (int i = 0; i < iconHeight; i++) {
					for (int j = 0; j < iconWidth; j++, colorPtr+=3) {
						colorPtr[0] = GetBValue(it->second.pathColor);
						colorPtr[1] = GetGValue(it->second.pathColor);
						colorPtr[2] = GetRValue(it->second.pathColor);
					}
				}

				HICON hIcon = CreateIconIndirect(&ii);
				ImageList_AddIcon(hSmall, hIcon);
				DestroyIcon(hIcon);
				selectedPathsImage[it->first] = j;
			}
			DeleteObject(hBitmap);
			DeleteObject(hBlackMask);
			ReleaseDC(NULL, listViewDC);

			processingStageOutput.enterTestingPhase();

			processingStageOutput.ImageDestinationInfo(BACKGROUND_STAGE, NULL, 0) = backgroundStaticHwnd;
			processingStageOutput.ImageDestinationInfo(DISPARITY_STAGE, NULL, 0) = NULL;
			processingStageOutput.ImageDestinationInfo(EDGE_ACTIVITY_STAGE, NULL, 0) = NULL;
			
			processingStageOutput.ImageDestinationInfo(RAW_FOREGROUND_STAGE, NULL, 0) = leftLittleStaticHwnd;
			processingStageOutput.ImageDestinationInfo(PLANVIEW_OCCUPANCY_STAGE, NULL, 0) = NULL;

			processingStageOutput.ImageDestinationInfo(FILTERED_FOREGROUND_STAGE, NULL, 0) = rightLittleStaticHwnd;
			processingStageOutput.ImageDestinationInfo(PLANVIEW_HEIGHT_STAGE, NULL, 0) = NULL;

			//Tab control configuration
			TCITEM tie;
			tie.mask = TCIF_TEXT | TCIF_IMAGE; 
			tie.iImage = -1;
			
			if (si->GetStereoRig() && si->GetStereoRig()->IsRunning() &&(AcquisitionCameraFactory::GetRegisteredCameras()[si->GetStereoRig()->GetAcquisitionProperties().cameraModel].availableOptions & AcquisitionCameraFactory::CameraDescription::VIRTUAL_CAMERA)) {
				tie.pszText = L"Complete Offline test";
				currentlyTabbedWindow = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_COMPLETEOFFLINETEST),
					tabControl,	CompletePositionTestProc);
				doClipRect = false;
			} else {
				tie.pszText = L"New Position test";
				currentlyTabbedWindow = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_NEWPOSITIONTESTCHILD),
					tabControl, NewPositionTestProc);
				doClipRect = true;
				PerformClipCursor(hwndDlg, false);
			}
			TabCtrl_InsertItem(tabControl, 0, &tie);

			RECT tabbedWindowRect; GetWindowRect(tabControl, &tabbedWindowRect);
			tabbedWindowRect.right -= tabbedWindowRect.left;
			tabbedWindowRect.bottom -= tabbedWindowRect.top;
			tabbedWindowRect.left = tabbedWindowRect.top = 0;
			TabCtrl_AdjustRect(tabControl, FALSE, &tabbedWindowRect);

			SetWindowPos(currentlyTabbedWindow, NULL, tabbedWindowRect.left, tabbedWindowRect.top,
				tabbedWindowRect.right - tabbedWindowRect.left, tabbedWindowRect.bottom - tabbedWindowRect.top,
				SWP_SHOWWINDOW);
			
		}
		return TRUE;
	case WM_COMMAND:
		{
			WORD messageId = HIWORD(wParam);
			WORD controlId = LOWORD(wParam); 
			if (messageId == BN_CLICKED) {
				if (controlId == IDC_POSITIONTESTCLOSE || controlId == IDCANCEL) {
					PostMessage(hwndDlg, WM_CLOSE, 0, 0);
					return TRUE;
				} else if (controlId == IDC_STARTPOSITIONTEST) {
					if (currentlySelectedTest.GetPositionTestType() == PositionTest::NOT_INITIALIZED_TEST) {
						MessageBox(hwndDlg, L"No test has been selected", L"Check data", MB_OK | MB_ICONWARNING);
						return TRUE;
					}
					
					notYetAnalyzedData.clear();
					notYetAnalyzedData.reserve(200);
					if (currentlySelectedTest.GetPositionTestType() == PositionTest::ONLINE_POSITION_TEST || currentlySelectedTest.GetPositionTestType() == PositionTest::OFFLINE_POSITION_TEST) {
						DBOUT("online offline");
						std::vector<int> usersToSubjects;
						usersToSubjects.resize(currentlySelectedTest.size());
						for (int i = 0; i < currentlySelectedTest.size(); i++) {
							int mouseIndex = standardRawMouseMessageDispatcher.GetMouseIndexByUser(i + 1);
							if (mouseIndex == -1) {
								MessageBox(hwndDlg, L"You do not assigned a mouse to each user", L"Check data", MB_OK | MB_ICONWARNING);
								return TRUE;
							} else if (standardRawMouseMessageDispatcher.GetInstalledMice()[mouseIndex].associatedSubject == -1) {
								MessageBox(hwndDlg, L"Some users has not an associated identity", L"Check data", MB_OK | MB_ICONWARNING);
								return TRUE;
							} else {
								usersToSubjects[i] = standardRawMouseMessageDispatcher.GetInstalledMice()[mouseIndex].associatedSubject;
							}
						}
						if (currentlySelectedTest.GetPositionTestType() == PositionTest::OFFLINE_POSITION_TEST) {
							currentlySelectedTest.GetCvArrStorage().clear();
							si->GetElaborationCore()->SaveCurrentState(currentlySelectedTest.GetCvArrStorage());
							si->GetStereoRig()->StartRecorderMode();
						}
						currentlySelectedTest.StartNewTest(&usersToSubjects);
					} else if (currentlySelectedTest.GetPositionTestType() == PositionTest::COMPLETION_POSITION_TEST) {
						DBOUT("completion");
						if (si->GetElaborationCore())
							si->GetElaborationCore()->LoadSavedState(currentlySelectedTest.GetCvArrStorage());
						char dataDirectory[MAX_PATH * 10]; GetCurrentDirectoryA(MAX_PATH * 10, dataDirectory);
						const char *originalDataDirectory = currentlySelectedTest.GetAdditionalDataDirectory();
						if (originalDataDirectory[0] == '.') {
							strcat(dataDirectory, originalDataDirectory + 1);
						} else {
							strcpy(dataDirectory, originalDataDirectory);
						}
						si->GetStereoRig()->StartPlaybackMode(dataDirectory, false);
						currentlySelectedTest.StartNewTest(NULL);
					}
					currentlySelectedTest.UpdateImage(currentImage, mainTestDesigner);
					hBitmap = IplImage2HBITMAP(destFrame, hBitmap, currentImage);
					RedrawStaticControl(&destFrame, 1);

					EnableWindow(currentlyTabbedWindow, FALSE);
					
					EnableWindow(GetDlgItem(hwndDlg, IDC_STARTPOSITIONTEST), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_STOPPOSITIONTEST), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_POSITIONTESTCLOSE), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_SAVETESTTOREPOSITORY), FALSE);
					
					EnableWindow(GetDlgItem(hwndDlg, IDC_DATADIRECTORYBUTTON), FALSE);
					return TRUE;
				} else if (controlId == IDC_STOPPOSITIONTEST) {
					currentlySelectedTest.StopTest(mainTestDesigner, RoomSettings::GetInstance()->data.texelSide);
					if (currentlySelectedTest.GetPositionTestType() == PositionTest::OFFLINE_POSITION_TEST) {
						si->GetStereoRig()->StopRecorderMode();
					} else if (currentlySelectedTest.GetPositionTestType() == PositionTest::COMPLETION_POSITION_TEST) {
						si->GetStereoRig()->StopPlaybackMode();
					}
					EnableWindow(currentlyTabbedWindow, TRUE);

					EnableWindow(GetDlgItem(hwndDlg, IDC_STARTPOSITIONTEST), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_STOPPOSITIONTEST), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_POSITIONTESTCLOSE), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_SAVETESTTOREPOSITORY), TRUE);
					
					EnableWindow(GetDlgItem(hwndDlg, IDC_DATADIRECTORYBUTTON), TRUE);

					currentlySelectedTest.UpdateImage(currentImage, mainTestDesigner);
					hBitmap = IplImage2HBITMAP(destFrame, hBitmap, currentImage);
					RedrawStaticControl(&destFrame, 1);
					return TRUE;
				} else if (controlId == IDC_BACKGROUNDMODELCHECK) {
					processingStageOutput.ImageDestinationInfo(DISPARITY_STAGE, NULL, 0) = NULL;
					processingStageOutput.ImageDestinationInfo(EDGE_ACTIVITY_STAGE, NULL, 0) = NULL;
					processingStageOutput.ImageDestinationInfo(BACKGROUND_STAGE, NULL, 0) = backgroundStaticHwnd;
					RedrawStaticControl(&backgroundStaticHwnd, 1);
					return TRUE;
				} else if (controlId == IDC_EDGEINTENSITYCHECK) {
					processingStageOutput.ImageDestinationInfo(BACKGROUND_STAGE, NULL, 0) = NULL;
					processingStageOutput.ImageDestinationInfo(DISPARITY_STAGE, NULL, 0) = NULL;
					processingStageOutput.ImageDestinationInfo(EDGE_ACTIVITY_STAGE, NULL, 0) = backgroundStaticHwnd;
					RedrawStaticControl(&backgroundStaticHwnd, 1);
					return TRUE;
				} else if (controlId == IDC_DISPARITYMAPCHECK) {
					processingStageOutput.ImageDestinationInfo(BACKGROUND_STAGE, NULL, 0) = NULL;
					processingStageOutput.ImageDestinationInfo(EDGE_ACTIVITY_STAGE, NULL, 0) = NULL;
					processingStageOutput.ImageDestinationInfo(DISPARITY_STAGE, NULL, 0) = backgroundStaticHwnd;
					RedrawStaticControl(&backgroundStaticHwnd, 1);
					return TRUE;
				} else if (controlId == IDC_FOREGROUNDCHECK) {
					processingStageOutput.ImageDestinationInfo(PLANVIEW_OCCUPANCY_STAGE, NULL, 0) = NULL;
					processingStageOutput.ImageDestinationInfo(RAW_FOREGROUND_STAGE, NULL, 0) = leftLittleStaticHwnd;

					processingStageOutput.ImageDestinationInfo(PLANVIEW_HEIGHT_STAGE, NULL, 0) = NULL;
					processingStageOutput.ImageDestinationInfo(FILTERED_FOREGROUND_STAGE, NULL, 0) = rightLittleStaticHwnd;
					RedrawStaticControl(&leftLittleStaticHwnd, 1);
					RedrawStaticControl(&rightLittleStaticHwnd, 1);
					return TRUE;
				} else if (controlId == IDC_PLANVIEWCHECK) {
					processingStageOutput.ImageDestinationInfo(RAW_FOREGROUND_STAGE, NULL, 0) = NULL;
					processingStageOutput.ImageDestinationInfo(PLANVIEW_OCCUPANCY_STAGE, NULL, 0) = leftLittleStaticHwnd;

					processingStageOutput.ImageDestinationInfo(FILTERED_FOREGROUND_STAGE, NULL, 0) = NULL;
					processingStageOutput.ImageDestinationInfo(PLANVIEW_HEIGHT_STAGE, NULL, 0) = rightLittleStaticHwnd;
					RedrawStaticControl(&leftLittleStaticHwnd, 1);
					RedrawStaticControl(&rightLittleStaticHwnd, 1);
					return TRUE;
				} else if (controlId == IDC_SAVETESTTOREPOSITORY) {
					char testName[256];
					GetDlgItemTextA(hwndDlg, IDC_TESTNAMEEDIT, testName, 256);
					trim(testName);
					if (strcmp(testName, "") == 0) {
						MessageBox(hwndDlg, L"Empty string is not a valid experiment name", L"Error", MB_OK | MB_ICONWARNING);
						return TRUE;
					} else if (GetPositionTestVector().find(testName) != GetPositionTestVector().end()) {
						if (currentlySelectedTest.GetPositionTestType() == PositionTest::COMPLETION_POSITION_TEST) {
							MessageBox(hwndDlg, L"The name for the experiment has been already used. Unable to replace in completion mode", L"Name already exists", MB_OK | MB_ICONERROR);
							return TRUE;
						} else if (MessageBox(hwndDlg, L"The name for the experiment has been already used. Replace it?", L"Name already exists", MB_YESNO | MB_ICONQUESTION) == IDNO) {
							return TRUE;
						}
					}
					bool writableDirectory = true;
					if (currentlySelectedTest.GetPositionTestType() == PositionTest::OFFLINE_POSITION_TEST) {
						char baseDirectory[_MAX_PATH]; GetDlgItemTextA(hwndDlg, IDC_DATADIRECTORYEDIT, baseDirectory, _MAX_PATH);
						sprintf_s(baseDirectory, "%s\\%s - %s", baseDirectory, currentlySelectedTest.GetCompletionDate(),
							currentlySelectedTest.GetCompletionTime());
						currentlySelectedTest.SetAdditionalDataDirectory(baseDirectory);
						if (!CreateDirectoryA(baseDirectory, NULL))
							writableDirectory = false;
						else {
							si->GetStereoRig()->SaveRecordingToDirectory(baseDirectory);
							currentlySelectedTest.GetCvArrStorage().saveToDir(baseDirectory);
						}
					}
					if (writableDirectory)
						GetPositionTestVector()[testName] = currentlySelectedTest;
					else {
						MessageBox(hwndDlg, L"The system was not able to save additional data into the specified folder. Please verify that you have access to the folder.", L"Error", MB_OK | MB_ICONERROR);
					}
					return TRUE;
				} else if (controlId == IDC_DATADIRECTORYBUTTON) {
					char buf[_MAX_PATH];
					if (UseCommonItemDialog(buf, sizeof(buf), hwndDlg, CLSID_FileOpenDialog, FOS_PICKFOLDERS))
						SetDlgItemTextA(hwndDlg, IDC_DATADIRECTORYEDIT, buf);
					return TRUE;
				}
			}
		}
		break;
	case WM_CLOSE:
		{
			if (currentlySelectedTest.IsTestStarted())
				return TRUE;
			processingStageOutput.exitTestingPhase();
			if (hBitmap)
				DeleteObject(hBitmap);
			hBitmap = NULL;
			DeleteDC(memDC);
			currentlySelectedTest.clear();
			ClipCursor(NULL);
			if (currentlyTabbedWindow)
				DestroyWindow(currentlyTabbedWindow);
			currentlyTabbedWindow = NULL;
			ImageList_Destroy(hSmall); hSmall = NULL;
			ImageList_Destroy(hSmallMouseActivity); hSmallMouseActivity = NULL;
			EnableWindow(GetParent(hwndDlg), TRUE);
			DestroyWindow(hwndDlg);
			doClipRect = false;
			SetEvent(eventToWait[1]);
		}
		return TRUE;
	case WM_PAINT:
		{
			DBOUT("WM_PAINT");
			PAINTSTRUCT ps;
			BeginPaint(hwndDlg, &ps);

			HBRUSH blackBrush = (HBRUSH) GetStockObject(BLACK_BRUSH);
			
			RECT frameClientRect; GetClientRect(destFrame, &frameClientRect);
			MapWindowPoints(destFrame, hwndDlg, (POINT *) &frameClientRect, 2);
			if (AreRectIntersected(RECTToLeoRect(frameClientRect), RECTToLeoRect(ps.rcPaint))) {
				if (memDC && hBitmap) {
					SelectObject(memDC, hBitmap);
					BitBlt(ps.hdc, frameClientRect.left, frameClientRect.top, currentImage->width,
						currentImage->height, memDC, 0, 0, SRCCOPY);
				} else {
					FillRect(ps.hdc, &frameClientRect, blackBrush);
				}
			}

			GetClientRect(backgroundStaticHwnd, &frameClientRect);
			MapWindowPoints(backgroundStaticHwnd, hwndDlg, (POINT *) &frameClientRect, 2);
			if (AreRectIntersected(RECTToLeoRect(frameClientRect), RECTToLeoRect(ps.rcPaint)))
				FillRect(ps.hdc, &frameClientRect, blackBrush);

			GetClientRect(leftLittleStaticHwnd, &frameClientRect);
			MapWindowPoints(leftLittleStaticHwnd, hwndDlg, (POINT *) &frameClientRect, 2);
			if (AreRectIntersected(RECTToLeoRect(frameClientRect), RECTToLeoRect(ps.rcPaint)))
				FillRect(ps.hdc, &frameClientRect, blackBrush);

			GetClientRect(rightLittleStaticHwnd, &frameClientRect);
			MapWindowPoints(rightLittleStaticHwnd, hwndDlg, (POINT *) &frameClientRect, 2);
			if (AreRectIntersected(RECTToLeoRect(frameClientRect), RECTToLeoRect(ps.rcPaint)))
				FillRect(ps.hdc, &frameClientRect, blackBrush);

			EndPaint(hwndDlg, &ps);
		}
		return TRUE;
	}
	return FALSE;
}

DWORD WINAPI PositionTestThread(LPVOID lpParameter) {
	testPositionHwnd = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_POSITIONTEST), (HWND) lpParameter, PositionTestProc);
	ShowWindow(testPositionHwnd, SW_SHOW);
	UpdateWindow(testPositionHwnd);

	DWORD waitResult = 2;
	while (waitResult != 1) {
		waitResult = MsgWaitForMultipleObjects(2, eventToWait, FALSE, INFINITE, QS_ALLINPUT) - WAIT_OBJECT_0;
		if (waitResult == 2) {
			//DBOUT("waitResult = 2");
			if (standardRawMouseMessageDispatcher.IsRawMessageAnalysisStarted())
				standardRawMouseMessageDispatcher.ReplacedWindowsInnerLoop();
			else {
				//DBOUT("waitResult - MSG");
				MSG msg;
				while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
					if (!IsDialogMessage(testPositionHwnd, &msg) && (!currentlyTabbedWindow || !IsDialogMessage(currentlyTabbedWindow, &msg))) {
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
			}
		} else if (waitResult == 0) {
			//DBOUT("waitResult = 0");
			currentlySelectedTest.AddLastSoftwareMeasurements(mainTestDesigner, RoomSettings::GetInstance()->data.texelSide, notYetAnalyzedData, notYetAnalyzedDataLock);
		}
	}
	return 0;
}

bool ShowPositionTestWindow(HWND parentWindow) {
	DWORD threadID;
	chBEGINTHREADEX(NULL, 0, PositionTestThread, (LPVOID) parentWindow, 0, &threadID);
	return true;
}