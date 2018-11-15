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

#include "MLDialog.h"
#include "Shared.h"
#include "ElaborationCore.h"
#include "PlanViewMap.h"
#include "SVMDialog.h"
#include <Uxtheme.h>

#include <LeoWindowsGUI.h>

class MLDialogPlanViewMapController: public PlanViewMapController {
private:
	virtual void CurrentFrameUpdate();
public:
	MLDialogPlanViewMapController();
	void (* newFrameOccurredCallback) ();
	void AccuracyCallback(int correct, int total, int appliedCorrect, int appliedTotal);
	int currentCorrect, currentTotal, appliedCorrect, appliedTotal;
	HWND staticUpdate;
};

MLDialogPlanViewMapController::MLDialogPlanViewMapController(): PlanViewMapController() {
	newFrameOccurredCallback = NULL;
}

void MLDialogPlanViewMapController::CurrentFrameUpdate() {
	newFrameOccurredCallback();
}

void MLDialogPlanViewMapController::AccuracyCallback(int correct, int total, int appliedCorrect, int appliedTotal) {
	wchar_t buffer[100];
	currentCorrect+=correct;
	currentTotal+=total;
	this->appliedCorrect+=appliedCorrect;
	this->appliedTotal+=appliedTotal;
	swprintf_s(buffer, 100, L"svm: %.2f - track: %.2f", double(currentCorrect)/double(currentTotal),
		double(this->appliedCorrect)/double(this->appliedTotal));
	SetWindowText(staticUpdate, buffer);
}

static MLDialogPlanViewMapController mdpvmc;

static HWND mlDialogHwnd = NULL;

CvMat *trackedAgentsImage = NULL, *detectedObjectsImage = NULL;

extern SystemInfo* si;

static int currentTrackedObject;

static HDC memDC; static HBITMAP hBitmap;

static bool controlStarted;

static void UpdateWindowContent() {
	wchar_t buffer[10];
	currentTrackedObject++;

	EnableWindow(GetDlgItem(mlDialogHwnd, IDC_BUTTON2), TRUE);
	if (currentTrackedObject == int(mdpvmc.trackedObjects->size())) {
		SetDlgItemText(mlDialogHwnd, IDC_BUTTON2, L"Next Frame");
		SetDlgItemText(mlDialogHwnd, IDC_TRACKEDSTATIC, L"-");
		SetDlgItemText(mlDialogHwnd, IDC_CURRENTTRACKEDSTATIC, L"");
	} else {
		SetDlgItemText(mlDialogHwnd, IDC_BUTTON2, L"-->");
		swprintf_s(buffer, L"%C", 'A' + currentTrackedObject);
		SetDlgItemText(mlDialogHwnd, IDC_TRACKEDSTATIC, buffer);
		swprintf_s(buffer, L"%d/%d", currentTrackedObject + 1, int(mdpvmc.trackedObjects->size()));
		SetDlgItemText(mlDialogHwnd, IDC_CURRENTTRACKEDSTATIC, buffer);
	}
}

static void PrepareComponentValuesListView(HWND listViewControl, CvMat *componentValues) {
	wchar_t buffer[100];
	DWORD dwStyle = (DWORD) SendMessage(listViewControl, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
			dwStyle |= (LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);
	SendMessage(listViewControl,LVM_SETEXTENDEDLISTVIEWSTYLE,0,dwStyle);
	SetWindowTheme(listViewControl, L"Explorer", NULL);

	ListView_DeleteAllItems(listViewControl);
	while (ListView_DeleteColumn(listViewControl, 0)) {}

	if (componentValues) {
		LVCOLUMN lc;
		lc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lc.fmt = LVCFMT_LEFT;

		lc.cx = 30;
		lc.pszText = L"#";
		lc.iSubItem = 0;
		ListView_InsertColumn(listViewControl, 0, &lc);

		lc.cx = 200;
		lc.pszText = buffer;
		for (int i = 1; i <= componentValues->cols; i++) {
			swprintf_s(buffer, 100, L"%d", i - 1);
			lc.iSubItem = i;
			ListView_InsertColumn(listViewControl, i, &lc);
		}

		for (int i = 0; i < componentValues->rows; i++) {
			float *cmpPtr = (float *) (componentValues->data.ptr + i * componentValues->step);
			LVITEM li;
			li.mask = 0;
			li.state = 0; li.stateMask = 0;
			li.iItem = i;
			li.iSubItem = 0;
			ListView_InsertItem(listViewControl, &li);
			swprintf_s(buffer, 100, L"%C", 'A' + i);
			ListView_SetItemText(listViewControl, i, 0, buffer);
			for (int j = 0; j < componentValues->cols; j++, cmpPtr+=3) {
				//swprintf_s(buffer, 100, L"%f,%f,%f", (double) cmpPtr[0], (double) cmpPtr[1], (double) cmpPtr[2]);
				swprintf_s(buffer, 100, L"%f,%f,%f", (double) cmpPtr[0], (double) cmpPtr[1], (double) cmpPtr[2]);
				ListView_SetItemText(listViewControl, i, j + 1, buffer);
			}
		}
	}
}

static void NewFrameOccurredCallback() {
	wchar_t buffer[10];
	char buffer_c[10];

	CvFont selectedFont; cvInitFont(&selectedFont, CV_FONT_HERSHEY_PLAIN, 1.0, 1.0, 0.0, 2);

	currentTrackedObject = -1;
	int i = 0;
	ComboBox_ResetContent(GetDlgItem(mlDialogHwnd, IDC_COMBOLIST));
	ComboBox_AddString(GetDlgItem(mlDialogHwnd, IDC_COMBOLIST), L"");
	//cvZero(detectedObjectsImage);
	cvCopy(si->GetElaborationCore()->GetPlanViewMap()->GetOccupancyImage(), detectedObjectsImage);
	for (std::vector<ObjectTemplate *>::const_iterator it = mdpvmc.candidateObjects->begin(); it != mdpvmc.candidateObjects->end(); it++) {
		swprintf_s(buffer, L"%d", i++);
		ComboBox_AddString(GetDlgItem(mlDialogHwnd, IDC_COMBOLIST), buffer);
		sprintf_s(buffer_c, "%S", buffer);
		CvPoint upperLeft = cvPoint(int((*it)->currentMeasurement->data.fl[0] - ((*it)->xSideBox / 2)),
			int((*it)->currentMeasurement->data.fl[1] - ((*it)->ySideBox / 2)));
		CvPoint bottomRight = cvPoint(upperLeft.x + (*it)->xSideBox, upperLeft.y + (*it)->ySideBox);
		cvRectangle(detectedObjectsImage, upperLeft, bottomRight, cvScalar(255, 255, 255), 1);
		cvPutText(detectedObjectsImage, buffer_c,
			cvPoint(int((*it)->currentMeasurement->data.fl[0]), int((*it)->currentMeasurement->data.fl[1])),
			&selectedFont, cvScalar(255,255,255));
	}
	ComboBox_AddString(GetDlgItem(mlDialogHwnd, IDC_COMBOLIST), L"-");
	ComboBox_SetCurSel(GetDlgItem(mlDialogHwnd, IDC_COMBOLIST), 0);

	cvZero(trackedAgentsImage);
	i = 0;
	for (std::vector<TrackedObject *>::const_iterator it = mdpvmc.trackedObjects->begin(); it != mdpvmc.trackedObjects->end(); it++) {
		cvRectangle(trackedAgentsImage, (*it)->upperLeft, (*it)->bottomRight, (*it)->color, 1);
		sprintf_s(buffer_c, "%c", 'A' + i);
		cvPutText(trackedAgentsImage, buffer_c,
			cvPoint(int((*it)->kalmanFilter->state_post->data.fl[0]), int((*it)->kalmanFilter->state_post->data.fl[1])),
			&selectedFont, cvScalar(255,255,255));
		i++;
	}

	UpdateWindowContent();
	HWND controlsToRedraw[] = {GetDlgItem(mlDialogHwnd, IDC_TRACKEDPICTURESTATIC), GetDlgItem(mlDialogHwnd, IDC_DETECTEDOBJECTSTATIC)};
	RedrawStaticControl(controlsToRedraw, 2, false, true);

	PrepareComponentValuesListView(GetDlgItem(mlDialogHwnd, IDC_LIST2), mdpvmc.componentValues);
}

static void PrepareListViewControl(HWND listViewControl) {
	DWORD dwStyle = (DWORD) SendMessage(listViewControl, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
			dwStyle |= (LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);
	SendMessage(listViewControl,LVM_SETEXTENDEDLISTVIEWSTYLE,0,dwStyle);
	SetWindowTheme(listViewControl, L"Explorer", NULL);

	ListView_DeleteAllItems(listViewControl);
	while (ListView_DeleteColumn(listViewControl, 0)) {}

	LVCOLUMN lc;
	lc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lc.fmt = LVCFMT_LEFT;

	lc.cx = 80;
	lc.pszText = L"Color";
	lc.iSubItem = 0;
	ListView_InsertColumn(listViewControl, 0, &lc);

	lc.cx = 80;
	lc.pszText = L"Position";
	lc.iSubItem = 1;
	ListView_InsertColumn(listViewControl, 1, &lc);

	lc.cx = 80;
	lc.pszText = L"Angle";
	lc.iSubItem = 2;
	ListView_InsertColumn(listViewControl, 2, &lc);

	lc.cx = 30;
	lc.pszText = L"Classification";
	lc.iSubItem = 3;
	ListView_InsertColumn(listViewControl, 3, &lc);
}

bool TrainingDataListViewToFile(HWND listViewHandle, char* fileName) {
	wchar_t buffer[30];
	TrainingDataFileRecord tdfr;
	HANDLE hFile = CreateFileA(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	int numberOfElements = ListView_GetItemCount(listViewHandle);
	for (int i = 0; i < numberOfElements; i++) {
		ListView_GetItemText(listViewHandle, i, 0, buffer, 30);
		tdfr.colorDifference = (float) _wtof(buffer);
		ListView_GetItemText(listViewHandle, i, 1, buffer, 30);
		tdfr.positionDifference = (float) _wtof(buffer);
		ListView_GetItemText(listViewHandle, i, 2, buffer, 30);
		tdfr.angleDifference = (float) _wtof(buffer);
		ListView_GetItemText(listViewHandle, i, 3, buffer, 30);
		tdfr.classification = _wtoi(buffer);
		DWORD bytesWritten;
		WriteFile(hFile, &tdfr, sizeof(tdfr), &bytesWritten, NULL);
	}
	CloseHandle(hFile);
	return true;
}

bool TrainingDataFileToListView(char* fileName, HWND listViewHandle) {
	wchar_t buffer[30];
	TrainingDataFileRecord tdfr;
	HANDLE hFile = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD fileDimension = GetFileSize(hFile, NULL);
	int numberOfRecords = fileDimension / sizeof(tdfr);
	for (int i = 0; i < numberOfRecords; i++) {
		DWORD bytesRead;
		ReadFile(hFile, &tdfr, sizeof(tdfr), &bytesRead, NULL);
		LVITEM li;
		li.mask = 0;
		li.state = 0; li.stateMask = 0;
		li.iItem = ListView_GetItemCount(listViewHandle);
		li.iSubItem = 0;

		ListView_InsertItem(listViewHandle, &li);

		swprintf_s(buffer, 30, L"%f", (double) tdfr.colorDifference);
		ListView_SetItemText(listViewHandle, li.iItem, 0, buffer);
		swprintf_s(buffer, 30, L"%f", (double) tdfr.positionDifference);
		ListView_SetItemText(listViewHandle, li.iItem, 1, buffer);
		swprintf_s(buffer, 30, L"%f", (double) tdfr.angleDifference);
		ListView_SetItemText(listViewHandle, li.iItem, 2, buffer);

		swprintf_s(buffer, 30, L"%d", (int) tdfr.classification);
		ListView_SetItemText(listViewHandle, li.iItem, 3, buffer);
	}
	CloseHandle(hFile);
	int numberOfLearningMeasurement = ListView_GetItemCount(listViewHandle);
	if (numberOfLearningMeasurement > 0)
		ListView_EnsureVisible(listViewHandle, numberOfLearningMeasurement - 1, FALSE);
	return true;
}

bool TrainingDataListViewToMemory(HWND listViewHandle, CvMat **trainingDataMatrix, CvMat **trainingClassificationMatrix) {
	wchar_t buffer[30];
	int numberOfElements = ListView_GetItemCount(listViewHandle);
	*trainingDataMatrix = cvCreateMat(numberOfElements, 3, CV_32FC1);
	*trainingClassificationMatrix = cvCreateMat(numberOfElements, 1, CV_32FC1);
	for (int i = 0; i < numberOfElements; i++) {
		float *trainingPtr = (float *) ((*trainingDataMatrix)->data.ptr + i * (*trainingDataMatrix)->step);
		float *classPtr = (float *) ((*trainingClassificationMatrix)->data.ptr + i * (*trainingClassificationMatrix)->step);
		for (int j = 0; j < 3; j++) {
			ListView_GetItemText(listViewHandle, i, j, buffer, 30);
			trainingPtr[j] = (float) _wtof(buffer);
		}
		ListView_GetItemText(listViewHandle, i, 3, buffer, 30);
		classPtr[0] = (float) _wtof(buffer);
	}
	return true;
}

bool TrainingDataFileToMemory(char *fileName, CvMat **trainingDataMatrix, CvMat **trainingClassificationMatrix) {
	TrainingDataFileRecord tdfr;
	HANDLE hFile = CreateFileA(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD fileDimension = GetFileSize(hFile, NULL);
	int numberOfRecords = fileDimension / sizeof(tdfr);
	*trainingDataMatrix = cvCreateMat(numberOfRecords, 3, CV_32FC1);
	*trainingClassificationMatrix = cvCreateMat(numberOfRecords, 1, CV_32FC1);
	for (int i = 0; i < numberOfRecords; i++) {
		float *trainingPtr = (float *) ((*trainingDataMatrix)->data.ptr + i * (*trainingDataMatrix)->step);
		float *classPtr = (float *) ((*trainingClassificationMatrix)->data.ptr + i * (*trainingClassificationMatrix)->step);
		DWORD bytesRead;
		ReadFile(hFile, &tdfr, sizeof(tdfr), &bytesRead, NULL);
		trainingPtr[0] = tdfr.colorDifference;
		trainingPtr[1] = tdfr.positionDifference;
		trainingPtr[2] = tdfr.angleDifference;
		classPtr[0] = (float) tdfr.classification;
	}
	CloseHandle(hFile);
	return true;
}

static INT_PTR CALLBACK MLDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	wchar_t buffer[30];
	char buf[_MAX_PATH];
	HWND trackedAgentsStaticFrame = GetDlgItem(hwndDlg, IDC_TRACKEDPICTURESTATIC);
	HWND detectedObjectsStaticFrame = GetDlgItem(hwndDlg, IDC_DETECTEDOBJECTSTATIC);
	HWND learningPointList = GetDlgItem(hwndDlg, IDC_LIST1);
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			hBitmap = NULL;
			controlStarted = false;
			HDC frameDC = GetDC(trackedAgentsStaticFrame);
			memDC = CreateCompatibleDC(frameDC);
			ReleaseDC(trackedAgentsStaticFrame, frameDC);

			PrepareListViewControl(GetDlgItem(hwndDlg, IDC_LIST1));

			mdpvmc.newFrameOccurredCallback = NewFrameOccurredCallback;
		}
		return TRUE;
	case WM_CLOSE:
		if (controlStarted) {
			MessageBox(hwndDlg, L"System is still running", L"Error", MB_OK | MB_ICONINFORMATION);
		} else {
			CloseMLDialog();
		}
		return TRUE;
	case WM_DESTROY:
		hBitmap = NULL;
		DeleteDC(memDC);
		return TRUE;
	case WM_COMMAND:
		{
			WORD messageId = HIWORD(wParam);
			WORD controlId = LOWORD(wParam);
			if (controlId == IDC_BUTTON2) {
				if (currentTrackedObject == int(mdpvmc.trackedObjects->size())) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON2), FALSE);
					SetDlgItemText(hwndDlg, IDC_BUTTON2, L"");
					currentTrackedObject = -2;
					if (mdpvmc.componentValues)
						cvReleaseMat(&mdpvmc.componentValues);
					mdpvmc.UnBlockPlanViewMap();
				} else {
					int selectedIndex = ComboBox_GetCurSel(GetDlgItem(hwndDlg, IDC_COMBOLIST));
					ComboBox_GetText(GetDlgItem(hwndDlg, IDC_COMBOLIST), buffer, 10);
					int selectedValue = _wtoi(buffer);
					int numberOfElementsInCombo = ListView_GetItemCount(GetDlgItem(hwndDlg, IDC_COMBOLIST));
					if (wcscmp(buffer, L"-")) { //We do not want the current element is put into the training set
						if (selectedIndex != CB_ERR && selectedIndex != 0) {
							mdpvmc.currentAssociation[currentTrackedObject] = selectedValue;
						}
						if (mdpvmc.componentValues) {
							float *componentValuesPtr = (float *) (mdpvmc.componentValues->data.ptr + (currentTrackedObject*mdpvmc.componentValues->step));
							for (int i = 0; i < int(mdpvmc.candidateObjects->size()); i++, componentValuesPtr+=3) {
								LVITEM li;
								li.mask = 0;
								li.state = 0; li.stateMask = 0;
								li.iItem = ListView_GetItemCount(learningPointList);
								li.iSubItem = 0;

								ListView_InsertItem(learningPointList, &li);

								for (int j = 0; j < 3; j++) {
									swprintf_s(buffer, 30, L"%f", (double) componentValuesPtr[j]);
									ListView_SetItemText(learningPointList, li.iItem, j, buffer);
								}

								if (selectedIndex != 0 && i == selectedValue) {
									ListView_SetItemText(learningPointList, li.iItem, 3, L"1");
								} else {
									ListView_SetItemText(learningPointList, li.iItem, 3, L"0");
								}
							}
							int numberOfLearningMeasurement = ListView_GetItemCount(learningPointList);
							if (numberOfLearningMeasurement > 0)
								ListView_EnsureVisible(learningPointList, numberOfLearningMeasurement - 1, FALSE);
						}
					}
					ComboBox_SetCurSel(GetDlgItem(hwndDlg, IDC_COMBOLIST), 0);
					UpdateWindowContent();
				}
			} else if (controlId == IDC_BUTTON1) {
				if (MessageBox(hwndDlg, L"Are you sure you want to delete all training data currently accumulated?",
					L"Question", MB_YESNO | MB_ICONQUESTION) == IDYES) {
						ListView_DeleteAllItems(learningPointList);
				}
			} else if (controlId == IDC_BUTTON3) {
				COMDLG_FILTERSPEC filtro[] = {L"ML Training Set File", L"*.mlt"};
				if (UseCommonItemDialog(buf, sizeof(buf), hwndDlg, CLSID_FileOpenDialog, 0, filtro, 1)) {
					HCURSOR originalCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
					TrainingDataFileToListView(buf, learningPointList);
					SetCursor(originalCursor);
				}
			} else if (controlId == IDC_BUTTON5) {
				COMDLG_FILTERSPEC filtro[] = {L"ML Training Set File", L"*.mlt"};
				if (UseCommonItemDialog(buf, sizeof(buf), hwndDlg, CLSID_FileSaveDialog, 0, filtro, 1)) {
					HCURSOR originalCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
					TrainingDataListViewToFile(learningPointList, buf);
					SetCursor(originalCursor);
				}
			} else if (controlId == IDC_BUTTON6) {
				if (controlStarted) {
					MessageBox(hwndDlg, L"System still running", L"Error", MB_OK | MB_ICONERROR);
				} else {
					CvMat *trainingDataMat, *trainingClassMat;
					TrainingDataListViewToMemory(learningPointList, &trainingDataMat, &trainingClassMat);
					ShowSVMDialog(hwndDlg, trainingDataMat, trainingClassMat);
				}
			}
		}
		return TRUE;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hwndDlg, &ps);

			RECT frameClientRect;

			HBRUSH blackBrush = (HBRUSH) GetStockObject(BLACK_BRUSH);
			
			GetClientRect(trackedAgentsStaticFrame, &frameClientRect);
			MapWindowPoints(trackedAgentsStaticFrame, hwndDlg, (POINT *) &frameClientRect, 2);
			if (AreRectIntersected(RECTToLeoRect(frameClientRect), RECTToLeoRect(ps.rcPaint))) {
				if (memDC && trackedAgentsImage) {
					hBitmap = IplImage2HBITMAP(trackedAgentsStaticFrame, hBitmap, trackedAgentsImage);
					SelectObject(memDC, hBitmap);
					AdaptiveBitBlt(ps.hdc, frameClientRect.left, frameClientRect.top, frameClientRect.right - frameClientRect.left,
						frameClientRect.bottom - frameClientRect.top, memDC, trackedAgentsImage->width,
						trackedAgentsImage->height);
				} else {
					FillRect(ps.hdc, &frameClientRect, blackBrush);
				}
			}

			GetClientRect(detectedObjectsStaticFrame, &frameClientRect);
			MapWindowPoints(detectedObjectsStaticFrame, hwndDlg, (POINT *) &frameClientRect, 2);
			if (AreRectIntersected(RECTToLeoRect(frameClientRect), RECTToLeoRect(ps.rcPaint))) {
				if (memDC && detectedObjectsImage) {
					hBitmap = IplImage2HBITMAP(detectedObjectsStaticFrame, hBitmap, detectedObjectsImage);
					SelectObject(memDC, hBitmap);
					AdaptiveBitBlt(ps.hdc, frameClientRect.left, frameClientRect.top, frameClientRect.right - frameClientRect.left,
						frameClientRect.bottom - frameClientRect.top, memDC, detectedObjectsImage->width,
						detectedObjectsImage->height);
				} else {
					FillRect(ps.hdc, &frameClientRect, blackBrush);
				}
			}

			EndPaint(hwndDlg, &ps);
		}
		return TRUE;
	}
	return FALSE;
}

void ShowMLDialog(HWND parentWindow, bool accuracyController) {
	mdpvmc.accuracyController = accuracyController;
	mlDialogHwnd = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_MLDIALOG), parentWindow, MLDialogProc);
	ShowWindow(mlDialogHwnd, SW_SHOW);
}

void CloseMLDialog() {
	if (controlStarted)
		VideoPlaybackStop();
	if (mlDialogHwnd) {
		DestroyWindow(mlDialogHwnd);
		mlDialogHwnd = NULL;
	}
}

bool VideoPlaybackStarted() {
	if (mlDialogHwnd == NULL || controlStarted)
		return false;
	if (si->GetElaborationCore() && si->GetElaborationCore()->IsRunning()) {
		trackedAgentsImage = cvCreateMat(si->GetElaborationCore()->GetPlanViewMap()->GetYTexelsNumber(),
			si->GetElaborationCore()->GetPlanViewMap()->GetXTexelsNumber(), CV_8UC3);
		cvZero(trackedAgentsImage);
		detectedObjectsImage = cvCreateMat(si->GetElaborationCore()->GetPlanViewMap()->GetYTexelsNumber(),
			si->GetElaborationCore()->GetPlanViewMap()->GetXTexelsNumber(), CV_8UC3);
		cvZero(detectedObjectsImage);
		HWND staticUpdate = GetDlgItem(mlDialogHwnd, IDC_ACCURACYSTATIC);
		mdpvmc.currentCorrect = mdpvmc.currentTotal = mdpvmc.appliedCorrect = mdpvmc.appliedTotal = 0;
		mdpvmc.staticUpdate = staticUpdate;
		SetWindowText(staticUpdate, L"");
		si->GetElaborationCore()->GetPlanViewMap()->RegisterPlanViewMapController(&mdpvmc);
		currentTrackedObject = -2;
		controlStarted = true;
		return true;
	} else {
		return false;
	}
}

bool VideoPlaybackStop() {
	if (mlDialogHwnd == NULL || !controlStarted)
		return false;
	if (si->GetElaborationCore() && si->GetElaborationCore()->IsRunning()) {
		si->GetElaborationCore()->GetPlanViewMap()->RegisterPlanViewMapController(NULL);
		if (mdpvmc.componentValues)
			cvReleaseMat(&mdpvmc.componentValues);
		if (trackedAgentsImage)
			cvReleaseMat(&trackedAgentsImage);
		if (detectedObjectsImage)
			cvReleaseMat(&detectedObjectsImage);
		if (hBitmap) {
			DeleteObject(hBitmap);
			hBitmap = NULL;
		}
		mdpvmc.UnBlockPlanViewMap();
		currentTrackedObject = -2;
		controlStarted = false;
		HWND controlsToRedraw[] = {GetDlgItem(mlDialogHwnd, IDC_TRACKEDPICTURESTATIC), GetDlgItem(mlDialogHwnd, IDC_DETECTEDOBJECTSTATIC)};
		RedrawStaticControl(controlsToRedraw, 2, false, true);
		ComboBox_ResetContent(GetDlgItem(mlDialogHwnd, IDC_COMBOLIST));
		SetDlgItemText(mlDialogHwnd, IDC_BUTTON2, L"");
		EnableWindow(GetDlgItem(mlDialogHwnd, IDC_BUTTON2), false);
		SetDlgItemText(mlDialogHwnd, IDC_TRACKEDSTATIC, L"-");
		SetDlgItemText(mlDialogHwnd, IDC_CURRENTTRACKEDSTATIC, L"");
		return true;
	} else {
		return false;
	}
}