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

#include "TestAnalyzerDialog.h"
#include <Uxtheme.h>
#include <LeoWindowsGUI.h>
#include <LeoLog4CPP.h>

using namespace leostorm::logging;

static PositionTestVector mainPositionTestVector;

static IplImage *currentImage = 0; static HBITMAP hBitmap = NULL; static HDC memDC = NULL;

static std::string currentlyVisualizedExperiment;
static std::vector<std::string> currentlySelectedExperiments;
static PositionTestAnalysisResult currentlyVisualizedTestResult;

PositionTestVector & GetPositionTestVector() {
	return mainPositionTestVector;
}

static void UpdateVisualization(HWND dialogWindow) {
	wchar_t wideBuffer[256];
	char normalBuffer[256];
	
	cvSetZero(currentImage);
	if (strcmp(currentlyVisualizedExperiment.c_str(), "") != 0) {
		int visualizationMask = 0;
		if (IsDlgButtonChecked(dialogWindow, IDC_SHOWGRID) == BST_CHECKED)
			visualizationMask |= PositionTest::SHOW_GRID;
		if (IsDlgButtonChecked(dialogWindow, IDC_SHOWGROUNDTRUTHPATHS) == BST_CHECKED)
			visualizationMask |= PositionTest::SHOW_PATH;
		if (IsDlgButtonChecked(dialogWindow, IDC_SHOWSWMEASUREMENTS) == BST_CHECKED)
			visualizationMask |= PositionTest::SHOW_SOFTWARE_MEASUREMENTS;
		if (IsDlgButtonChecked(dialogWindow, IDC_HIGHLIGHTCORRESPONDENCES) == BST_CHECKED)
			visualizationMask |= PositionTest::SHOW_REAL_SW_CORRESPONDENCE;
		GetDlgItemText(dialogWindow, IDC_GRIDSTEPEDIT, wideBuffer, 256);
		int gridStepString = _wtoi(wideBuffer);

		HWND pathList = GetDlgItem(dialogWindow, IDC_PATHLIST);
		int nextSelectedIndex = ListView_GetNextItem(pathList, -1, LVNI_SELECTED);
		std::vector<int> selectedPathVector;
		while (nextSelectedIndex != -1) {
			ListView_GetItemText(pathList, nextSelectedIndex, 2, wideBuffer, 256);
			sprintf_s(normalBuffer, 256, "%S", wideBuffer);
			selectedPathVector.push_back(mainTestDesigner.FindPathIndexByName(normalBuffer));
			nextSelectedIndex = ListView_GetNextItem(pathList, nextSelectedIndex, LVNI_SELECTED);
		}

		if (!mainPositionTestVector[currentlyVisualizedExperiment].UpdateImage(currentImage, mainTestDesigner,
			RoomSettings::GetInstance()->data.texelSide, visualizationMask,	gridStepString, &selectedPathVector)) {
				MessageBox(dialogWindow, L"Unable to plot the image. Possible causes could be:\r\n\t- Too low grid step",
					L"Error", MB_OK | MB_ICONWARNING);
				return;
		}
	}
	HWND staticControl = GetDlgItem(dialogWindow, IDC_EXECUTEDTESTSTATIC);
	hBitmap = IplImage2HBITMAP(staticControl, hBitmap, currentImage);
	RedrawStaticControl(&staticControl, 1, false);
}

static void UpdateSelection(HWND hwndDlg) {
	HWND storedTestList = GetDlgItem(hwndDlg, IDC_STOREDTESTSLIST);
	HWND selectedExperimentsCombo = GetDlgItem(hwndDlg, IDC_SELECTEDEXPERIMENTSCOMBO);
	HWND selectedStatisticsSubsetCombo = GetDlgItem(hwndDlg, IDC_STATISTICSSUBSETCOMBO);
	HWND pathList = GetDlgItem(hwndDlg, IDC_PATHLIST);

	wchar_t wideBuffer[256];
	char normalBuffer[256];

	std::string oldVisualizedExperiment = currentlyVisualizedExperiment;

	currentlySelectedExperiments.clear();
	ComboBox_ResetContent(selectedExperimentsCombo);

	std::vector<int> selection;
	GetListViewSelectedIndexes(storedTestList, selection);
	for (std::vector<int>::const_iterator it = selection.begin(); it != selection.end(); it++) {
		ListView_GetItemText(storedTestList, *it, 0, wideBuffer, 256);
		sprintf_s(normalBuffer, 256, "%S", wideBuffer);
		currentlySelectedExperiments.push_back(normalBuffer);
		ComboBox_AddString(selectedExperimentsCombo, wideBuffer);
	}

	if (currentlySelectedExperiments.size()) {
		if (std::find(currentlySelectedExperiments.begin(), currentlySelectedExperiments.end(), oldVisualizedExperiment) != currentlySelectedExperiments.end()) {
			currentlyVisualizedExperiment = oldVisualizedExperiment;
		} else {
			currentlyVisualizedExperiment = currentlySelectedExperiments[0];
		}
		swprintf_s(wideBuffer, 256, L"%S", currentlyVisualizedExperiment.c_str());
		ComboBox_SelectString(selectedExperimentsCombo, -1, wideBuffer);

		//***START OF HIGH PRECISION TIMER: MOVE TO LIBRARY***//
		LARGE_INTEGER ticksPerSecond, ticksPerMicrosecond, calculationStartTime, calculationEndTime;
		QueryPerformanceFrequency(&ticksPerSecond);
		ticksPerMicrosecond.QuadPart = ticksPerSecond.QuadPart / 1000000;
		QueryPerformanceCounter(&calculationStartTime);
		currentlyVisualizedTestResult = mainPositionTestVector.CalculateStatisticalResults(currentlySelectedExperiments, mainTestDesigner, RoomSettings::GetInstance()->data.texelSide);
		QueryPerformanceCounter(&calculationEndTime);
		LARGE_INTEGER elapsedTicks; elapsedTicks.QuadPart = calculationEndTime.QuadPart - calculationStartTime.QuadPart;
		Logger::writeToLOG(L"Statistical Result Computation Time: %lu\r\n",  (DWORD) (elapsedTicks.QuadPart / ticksPerMicrosecond.QuadPart));
		//***END OF HIGH PRECISION TIMER: MOVE TO LIBRARY****//

		if (currentlyVisualizedTestResult.errorInAcquiredData) {
			SetDlgItemText(hwndDlg, IDC_STATISTICLABEL, L"Errors detected into acquired data.");
		} else {
			swprintf_s(wideBuffer, 256, L"%S", currentlyVisualizedTestResult.statistics[ComboBox_GetCurSel(selectedStatisticsSubsetCombo)].toString().c_str());
			SetDlgItemText(hwndDlg, IDC_STATISTICLABEL, wideBuffer);
		}
	} else {
		currentlyVisualizedExperiment = "";
		wcscpy_s(wideBuffer, 256, L"");
		ComboBox_SetText(selectedExperimentsCombo, wideBuffer);
		SetDlgItemText(hwndDlg, IDC_STATISTICLABEL, L"Nothing to visualize");
	}
	SelectExperiment(pathList, currentlyVisualizedExperiment.compare("") == 0 ? NULL : &mainPositionTestVector[currentlyVisualizedExperiment.c_str()],
		NULL, false, true);
	UpdateVisualization(hwndDlg);
}

static INT_PTR CALLBACK TestAnalyzerWindowProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	wchar_t wideBuffer[256];
	char normalBuffer[256];
	HWND storedTestList = GetDlgItem(hwndDlg, IDC_STOREDTESTSLIST);
	HWND destFrame = GetDlgItem(hwndDlg, IDC_EXECUTEDTESTSTATIC);
	HWND pathList = GetDlgItem(hwndDlg, IDC_PATHLIST);
	HWND selectedExperimentsCombo = GetDlgItem(hwndDlg, IDC_SELECTEDEXPERIMENTSCOMBO);
	HWND selectedStatisticsSubsetCombo = GetDlgItem(hwndDlg, IDC_STATISTICSSUBSETCOMBO);
	switch (uMsg) {
	case WM_CLOSE:
		cvReleaseImage(&currentImage);
		DeleteObject(hBitmap); hBitmap = NULL;
		DeleteDC(memDC); memDC = NULL;
		EndDialog(hwndDlg, 0);
		return TRUE;
	case WM_COMMAND:
		{
			WORD ctrlId = LOWORD(wParam);
			WORD notificationCode = HIWORD(wParam);
			if (notificationCode == BN_CLICKED) {
				if (ctrlId == IDC_SHOWGROUNDTRUTHPATHS || ctrlId == IDC_SHOWSWMEASUREMENTS || ctrlId == IDC_HIGHLIGHTCORRESPONDENCES || ctrlId == IDC_SHOWGRID) {
					UpdateVisualization(hwndDlg);
				} else if (ctrlId == IDOK && GetFocus() == GetDlgItem(hwndDlg, IDC_GRIDSTEPEDIT)) {
					UpdateVisualization(hwndDlg);
				} else if (ctrlId == IDC_DELETESELECTEDTESTS) {
					for (std::vector<std::string>::const_iterator it = currentlySelectedExperiments.begin(); it != currentlySelectedExperiments.end(); it++) {
						mainPositionTestVector.erase(mainPositionTestVector.find((*it)));
					}
					FillStoredExperimentList(storedTestList, &mainPositionTestVector, false,
						PositionTest::ONLINE_POSITION_TEST | PositionTest::COMPLETION_POSITION_TEST, mainTestDesigner);
					currentlyVisualizedExperiment = "";
					currentlySelectedExperiments.clear();
					UpdateSelection(hwndDlg);
					UpdateVisualization(hwndDlg);
				} else if (ctrlId == IDC_EXPORTSTATS) {
					char buf[260];
					COMDLG_FILTERSPEC filtro[] = {L"CSV Statistics File", L"*.csv"};
					if (UseCommonItemDialog(buf, sizeof(buf), hwndDlg, CLSID_FileSaveDialog, 0, filtro, 1)) {
						DWORD bytesWritten;
						HCURSOR originalCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
						HANDLE hFile = CreateFileA(buf, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
						if (hFile == INVALID_HANDLE_VALUE) {
							MessageBox(hwndDlg, L"Cannot create CSV file", L"Error", MB_ICONERROR);
						} else {
							char valoreCorrente[1024];
							for (int i = 0; i < PositionTestAnalysisResult::numberOfSupportedStatistics; i++) {
								sprintf_s(valoreCorrente, ",%s,,,,", PositionTestAnalysisResult::StatisticsDescription[i].c_str());
								WriteFile(hFile, valoreCorrente, (DWORD) strlen(valoreCorrente), &bytesWritten, NULL);
							}
							WriteFile(hFile, "\r\n", 2, &bytesWritten, NULL);
							for (int i = 0; i < PositionTestAnalysisResult::numberOfSupportedStatistics; i++) {
								sprintf_s(valoreCorrente, ",avg,stdev,min,max,90th");
								WriteFile(hFile, valoreCorrente, (DWORD) strlen(valoreCorrente), &bytesWritten, NULL);
							}
							WriteFile(hFile, "\r\n\r\n", 4, &bytesWritten, NULL);
							for (std::vector<std::string>::const_iterator it = currentlySelectedExperiments.begin(); it != currentlySelectedExperiments.end(); it++) {
								std::vector<std::string> singleTest;
								singleTest.push_back(*it);
								PositionTestAnalysisResult ptar = mainPositionTestVector.CalculateStatisticalResults(singleTest,
									mainTestDesigner, RoomSettings::GetInstance()->data.texelSide);
								sprintf_s(valoreCorrente, "%s", it->c_str());
								WriteFile(hFile, valoreCorrente, (DWORD) strlen(valoreCorrente), &bytesWritten, NULL);
								for (int i = 0; i < ptar.numberOfSupportedStatistics; i++) {
									sprintf_s(valoreCorrente, ",%f,%f,%f,%f,%f", ptar.statistics[i].average, ptar.statistics[i].stdDev,
										ptar.statistics[i].min, ptar.statistics[i].max, ptar.statistics[i].perc90);
									WriteFile(hFile, valoreCorrente, (DWORD) strlen(valoreCorrente), &bytesWritten, NULL);
								}
								WriteFile(hFile, "\r\n\r\n", 4, &bytesWritten, NULL);
							}
							sprintf_s(valoreCorrente, "all");
							WriteFile(hFile, valoreCorrente, (DWORD) strlen(valoreCorrente), &bytesWritten, NULL);
							PositionTestAnalysisResult resumeRes = mainPositionTestVector.CalculateStatisticalResults(currentlySelectedExperiments, mainTestDesigner, RoomSettings::GetInstance()->data.texelSide);
							for (int i = 0; i < resumeRes.numberOfSupportedStatistics; i++) {
								sprintf_s(valoreCorrente, ",%f,%f,%f,%f,%f", resumeRes.statistics[i].average, resumeRes.statistics[i].stdDev,
									resumeRes.statistics[i].min, resumeRes.statistics[i].max, resumeRes.statistics[i].perc90);
								WriteFile(hFile, valoreCorrente, (DWORD) strlen(valoreCorrente), &bytesWritten, NULL);
							}
						}
						CloseHandle(hFile);
						SetCursor(originalCursor);
					}
				}
			} else if (notificationCode == CBN_SELCHANGE) {
				if (ctrlId == IDC_SELECTEDEXPERIMENTSCOMBO) {
					int comboSelection = ComboBox_GetCurSel(selectedExperimentsCombo);
					if (comboSelection == CB_ERR) {
						currentlyVisualizedExperiment == "";
					} else {
						ComboBox_GetLBText(selectedExperimentsCombo, comboSelection, wideBuffer);
						sprintf_s(normalBuffer, 256, "%S", wideBuffer);
						currentlyVisualizedExperiment = normalBuffer;
					}
					SelectExperiment(pathList, currentlyVisualizedExperiment.compare("") == 0 ? NULL : &mainPositionTestVector[currentlyVisualizedExperiment],
						NULL, false, true);
					UpdateVisualization(hwndDlg);
				} else if (ctrlId == IDC_STATISTICSSUBSETCOMBO) {
					if (currentlySelectedExperiments.size()) {
						if (currentlyVisualizedTestResult.errorInAcquiredData) {
							SetDlgItemText(hwndDlg, IDC_STATISTICLABEL, L"Errors detected into acquired data.");
						} else {
							swprintf_s(wideBuffer, 256, L"%S", currentlyVisualizedTestResult.statistics[ComboBox_GetCurSel(selectedStatisticsSubsetCombo)].toString().c_str());
							SetDlgItemText(hwndDlg, IDC_STATISTICLABEL, wideBuffer);
						}
					} else {
						SetDlgItemText(hwndDlg, IDC_STATISTICLABEL, L"Nothing to visualize");
					}
				}
			}
		}
		return TRUE;
	case WM_INITDIALOG:
		{
			DWORD dwStyle = (DWORD) SendMessage(storedTestList, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
			dwStyle |= (LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);
			SendMessage(storedTestList,LVM_SETEXTENDEDLISTVIEWSTYLE,0,dwStyle);
			SetWindowTheme(storedTestList, L"Explorer", NULL);

			FillStoredExperimentList(storedTestList, &mainPositionTestVector, true,
				PositionTest::ONLINE_POSITION_TEST | PositionTest::COMPLETION_POSITION_TEST, mainTestDesigner);
			currentlyVisualizedExperiment = "";
			currentlySelectedExperiments.clear();

			SelectExperiment(pathList, NULL, NULL, true, false);

			dwStyle = (DWORD) SendMessage(pathList, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
			dwStyle |= (LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);
			SendMessage(pathList,LVM_SETEXTENDEDLISTVIEWSTYLE,0,dwStyle);
			SetWindowTheme(pathList, L"Explorer", NULL);

			RECT imageClientRect; GetClientRect(destFrame, &imageClientRect);
			currentImage = cvCreateImage(cvSize(imageClientRect.right, imageClientRect.bottom), IPL_DEPTH_8U, 3);
			cvSetZero(currentImage);
			hBitmap = NULL;
			HDC frameDC = GetDC(destFrame);
			memDC = CreateCompatibleDC(frameDC);
			ReleaseDC(destFrame, frameDC);

			CheckDlgButton(hwndDlg, IDC_SHOWGROUNDTRUTHPATHS, BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWSWMEASUREMENTS, BST_CHECKED);

			SetDlgItemText(hwndDlg, IDC_GRIDSTEPEDIT, L"1");

			for (int i = 0; i < PositionTestAnalysisResult::numberOfSupportedStatistics; i++) {
				swprintf_s(wideBuffer, 256, L"%S", PositionTestAnalysisResult::StatisticsDescription[i].c_str());
				ComboBox_AddString(selectedStatisticsSubsetCombo, wideBuffer);
			}
			ComboBox_SetCurSel(selectedStatisticsSubsetCombo, 0);
		}
		return TRUE;
	case WM_CONTEXTMENU:
		{
			if (currentImage && HWND(wParam) == destFrame) {
				HMENU hPopupMenu = CreatePopupMenu();
				MENUITEMINFO mii; memset(&mii, 0, sizeof(mii));
				mii.cbSize = (UINT) sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE;
				mii.dwTypeData = wideBuffer;
				mii.fState = MFS_ENABLED;

				mii.wID = WM_USER;
				swprintf_s(wideBuffer, 255, L"Save as BMP image...");
				InsertMenuItem(hPopupMenu, 0, TRUE, &mii);

				mii.wID = WM_USER + 1;
				swprintf_s(wideBuffer, 255, L"Copy PS Code to Clipboard");
				InsertMenuItem(hPopupMenu, 1, TRUE, &mii);

				SetForegroundWindow(hwndDlg);
				POINT cursorPos; GetCursorPos(&cursorPos);
				UINT trackResult = (UINT) TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RETURNCMD | TPM_NONOTIFY, cursorPos.x, cursorPos.y,
					0, hwndDlg, NULL);
				DestroyMenu(hPopupMenu);
				if (trackResult == WM_USER) {
					COMDLG_FILTERSPEC filtro[] = {L"BMP Picture", L"*.bmp"};
					if (UseCommonItemDialog(normalBuffer, sizeof(normalBuffer), hwndDlg, CLSID_FileSaveDialog, 0, filtro, 1)) {
						cvSaveImage(normalBuffer, currentImage);
					}
				} else if (trackResult == WM_USER + 1) {
					//COMDLG_FILTERSPEC filtro[] = {L"PS Document", L"*.ps"};
					//if (UseCommonItemDialog(normalBuffer, sizeof(normalBuffer), hwndDlg, CLSID_FileSaveDialog, 0, filtro, 1)) {
						int visualizationMask = 0;
						if (IsDlgButtonChecked(hwndDlg, IDC_SHOWGRID) == BST_CHECKED)
							visualizationMask |= PositionTest::SHOW_GRID;
						if (IsDlgButtonChecked(hwndDlg, IDC_SHOWGROUNDTRUTHPATHS) == BST_CHECKED)
							visualizationMask |= PositionTest::SHOW_PATH;
						if (IsDlgButtonChecked(hwndDlg, IDC_SHOWSWMEASUREMENTS) == BST_CHECKED)
							visualizationMask |= PositionTest::SHOW_SOFTWARE_MEASUREMENTS;
						if (IsDlgButtonChecked(hwndDlg, IDC_HIGHLIGHTCORRESPONDENCES) == BST_CHECKED)
							visualizationMask |= PositionTest::SHOW_REAL_SW_CORRESPONDENCE;
						GetDlgItemText(hwndDlg, IDC_GRIDSTEPEDIT, wideBuffer, 256);
						int gridStepString = _wtoi(wideBuffer);

						HWND pathList = GetDlgItem(hwndDlg, IDC_PATHLIST);
						int nextSelectedIndex = ListView_GetNextItem(pathList, -1, LVNI_SELECTED);
						std::vector<int> selectedPathVector;
						while (nextSelectedIndex != -1) {
							ListView_GetItemText(pathList, nextSelectedIndex, 2, wideBuffer, 256);
							sprintf_s(normalBuffer, 256, "%S", wideBuffer);
							selectedPathVector.push_back(mainTestDesigner.FindPathIndexByName(normalBuffer));
							nextSelectedIndex = ListView_GetNextItem(pathList, nextSelectedIndex, LVNI_SELECTED);
						}
						char destBuffer[65000];
						//CHANGE ORIGIN TO UPPER-LEFT
						sprintf(destBuffer, "0 792 translate\n");
						strcat(destBuffer, "1 -1 scale\n");
						if (!mainPositionTestVector[currentlyVisualizedExperiment].SaveToPS(destBuffer, currentImage->width,
							currentImage->height, mainTestDesigner,
							RoomSettings::GetInstance()->data.texelSide, visualizationMask,	gridStepString, &selectedPathVector)) {
								MessageBox(hwndDlg, L"Unable to plot the image. Possible causes could be:\r\n\t- Too low grid step",
									L"Error", MB_OK | MB_ICONWARNING);
						} else {
							const size_t len = strlen(destBuffer) + 1;
							HGLOBAL hMem =  GlobalAlloc(GMEM_MOVEABLE, len);
							memcpy(GlobalLock(hMem), destBuffer, len);
							GlobalUnlock(hMem);
							OpenClipboard(0);
							EmptyClipboard();
							SetClipboardData(CF_TEXT, hMem);
							CloseClipboard();
						}
					//}
				}
			}
		}
		return TRUE;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hwndDlg, &ps);

			HDC frameDC = GetDC(destFrame);
			RECT frameClientRect; GetClientRect(destFrame, &frameClientRect);
			if (memDC && hBitmap) {
				SelectObject(memDC, hBitmap);
				BitBlt(frameDC, 0, 0, currentImage->width, currentImage->height, memDC, 0, 0, SRCCOPY);
			} else {
				FillRect(frameDC, &frameClientRect, (HBRUSH) GetStockObject(BLACK_BRUSH));
			}
			ReleaseDC(destFrame, frameDC);

			EndPaint(hwndDlg, &ps);
		}
		return TRUE;
	case WM_NOTIFY:
		{
			NMHDR *notificationCode = (NMHDR *) lParam;
			if (notificationCode->code == NM_CLICK) {
				NMITEMACTIVATE *activatePtr = (NMITEMACTIVATE *) lParam;
				if (notificationCode->idFrom == IDC_STOREDTESTSLIST) {
					UpdateSelection(hwndDlg);
					//UpdateVisualization(hwndDlg);
				} else if (notificationCode->idFrom == IDC_PATHLIST) {
					UpdateVisualization(hwndDlg);
					//UpdateSelection(hwndDlg);
				}
			}
		}
		return TRUE;
	}
	return FALSE;
}

void ShowTestAnalyzerWindow(HWND parentWindow) {
	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_STATISTICDIALOG), parentWindow, TestAnalyzerWindowProc);
}
