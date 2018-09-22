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

#include "SyncTest.h"
//#include "resource.h"
#include <LeoWindowsThread.h>
#include "NetworkCamera.h"
#include <LeoWindowsGDI.h>
#include <LeoWindowsOpenCV.h>
#include <LeoWindowsGUI.h>
#include <vector>

static int lightPos = 255;
static int threshPos = 50;

enum VisualizationChoice {NONE_VISUALIZATION, RAW, THRESHOLDED, EXPLORER} currentChoice;

struct ImagePair {
	long frameNumber;
	struct {
		bool triggered;
		IplImage *image;
	} images[2];
};

class SynchronizationStore: public std::vector<ImagePair> {
private:
	HWND contentLabel, triggerLabel, coupleIDlabel;
	int currentlyVisualized;
	std::vector<StereoRig::StereoTimestampsStruct> stereoTimeStamps;
	bool validData, recordingStarted;
	void updateLabel() {
		if (contentLabel) {
			wchar_t buffer[24]; swprintf_s(buffer, 24, L"%d/%d", currentlyVisualized + 1, (int) size());
			SetWindowText(contentLabel, buffer);
			if (currentlyVisualized >= 0 && currentlyVisualized < size()) {
				swprintf_s(buffer, 24, L"%ld", (*this)[currentlyVisualized].frameNumber);
				SetWindowText(coupleIDlabel, buffer);
				if ((*this)[currentlyVisualized].images[0].triggered && (*this)[currentlyVisualized].images[1].triggered) {
					SetWindowText(triggerLabel, L"L+R");
				} else if ((*this)[currentlyVisualized].images[0].triggered) {
					SetWindowText(triggerLabel, L"L");
				} else if ((*this)[currentlyVisualized].images[1].triggered) {
					SetWindowText(triggerLabel, L"R");
				}
			} else {
				SetWindowText(coupleIDlabel, L"N/A");
				SetWindowText(triggerLabel, L"N/A");
			}
		}
	}
public:
	SynchronizationStore() {
		contentLabel = NULL;
		currentlyVisualized = 0;
		validData = false;
	}
	~SynchronizationStore() {
		clear();
	}
	void StartRecording() {
		recordingStarted = true;
		validData = true;
	}
	void StopRecording() {
		recordingStarted = false;
	}
	bool ContainsData() {
		return validData;
	}
	bool IsRecording() {
		return recordingStarted;
	}
	void setContentLabel(HWND contentLabel, HWND triggerLabel = NULL, HWND coupleIDlabel = NULL) {
		this->contentLabel = contentLabel;
		this->coupleIDlabel = coupleIDlabel;
		this->triggerLabel = triggerLabel;
		updateLabel();
	}
	void clear() {
		recordingStarted = false;
		std::vector<ImagePair>::iterator it = begin();
		while (it != end()) {
			cvReleaseImage(&it->images[0].image);
			cvReleaseImage(&it->images[1].image);
			it++;
		}
		currentlyVisualized = -1;
		std::vector<ImagePair>::clear();
		stereoTimeStamps.clear();
		updateLabel();
	}
	void push_back(ImagePair &value) {
		std::vector<ImagePair>::push_back(value);
		if (size() == 1)
			showFirst();
		updateLabel();
	}
	void erase_current() {
		if (size() && currentlyVisualized >= 0)
			erase(begin() + currentlyVisualized);
		currentlyVisualized = MIN(currentlyVisualized, int(size()) - 1);
		updateLabel();
	}
	void push_back(StereoRig::StereoTimestampsStruct &stereoTimeStamp) {
		stereoTimeStamps.push_back(stereoTimeStamp);
	}

	//VISUALIZATION FUNCTIONS
	void showCurrentImage(HWND leftPic, HWND rightPic) {
		static HBITMAP hVisualizationBitmaps[2] = {NULL, NULL};
		static HDC memoryDC = CreateCompatibleDC(NULL);
		//This routine has to be called at every acquisition and will visualize the currently selected image
		if (size()) {
			int imageToVisualize = currentlyVisualized;
			hVisualizationBitmaps[0] = IplImage2HBITMAP(leftPic, hVisualizationBitmaps[0], (*this)[imageToVisualize].images[0].image);
			
			for (int i = 0; i < 2; i++) {
				hVisualizationBitmaps[i] = IplImage2HBITMAP(i == 0 ? leftPic : rightPic,
					hVisualizationBitmaps[i], (*this)[imageToVisualize].images[i].image);
				BITMAP infoBmp; GetObject(hVisualizationBitmaps[i], sizeof(infoBmp), &infoBmp);
				SelectObject(memoryDC, hVisualizationBitmaps[i]);
				HDC destDC = GetDC(i == 0 ? leftPic : rightPic);
				RECT currentRect; GetClientRect(i == 0 ? leftPic : rightPic, &currentRect);
				float widthRatio = float(currentRect.right) / float(infoBmp.bmWidth);
				SetStretchBltMode(destDC, HALFTONE);
				StretchBlt(destDC, 0, 0, currentRect.right, int(infoBmp.bmHeight * widthRatio), memoryDC, 0, 0, infoBmp.bmWidth, infoBmp.bmHeight, SRCCOPY); 
				ReleaseDC(i == 0 ? leftPic : rightPic, destDC);
			}
		}
	}
	void showPairAt(int index) {
		currentlyVisualized = (size() ? index : -1);
		updateLabel();
	}
	void showFirst() {
		showPairAt(0);
	}
	void showNext() {
		showPairAt(MIN(currentlyVisualized+1, int(size()) - 1));
	}
	void showPrevious() {
		showPairAt(MAX(0, currentlyVisualized -1));
	}
	void showLast() {
		showPairAt(int(size()) - 1);
	}
	SynchronizationTestResults Analyze() {
		//This function is devoted to the calculation of the statistics about the data stored in the SynchronizationStore
		SynchronizationTestResults result;
		//Here we calculate the average stereo frame rate
		int totalNumberOfStereoFrames = (int) stereoTimeStamps.size();
		DWORD elapsedTime = stereoTimeStamps.back().stereoTimeStamp - stereoTimeStamps.front().stereoTimeStamp;
		double elapsedTimeSecond = elapsedTime / 1000.0;
		result.stereoFrameRate = totalNumberOfStereoFrames / elapsedTimeSecond;

		//Here we calculate the absolute statistics about component frames

		{
			std::vector<StereoRig::StereoTimestampsStruct>::const_iterator it = stereoTimeStamps.begin();
			result.absoluteDifference.maxDifference = -1;
			result.absoluteDifference.minDifference = -1;
			DWORD totalDifference = 0, totalSquareDifference = 0;
			while (it != stereoTimeStamps.end()) {
				StereoRig::StereoTimestampsStruct sts = *it;
				DWORD currentDifference = MAX(sts.cameraTimeStamps[0], sts.cameraTimeStamps[1]) - MIN(sts.cameraTimeStamps[0], sts.cameraTimeStamps[1]);
				LEO_CONSTRAINED_MAX_UPDATE(result.absoluteDifference.maxDifference, int(currentDifference), -1);
				LEO_CONSTRAINED_MIN_UPDATE(result.absoluteDifference.minDifference, int(currentDifference), -1);
				totalDifference+= currentDifference;
				totalSquareDifference+= (currentDifference*currentDifference);
				it++;
			}
			result.absoluteDifference.averageDifference = double(totalDifference) / double(totalNumberOfStereoFrames);
			result.absoluteDifference.varianceDifference = double(totalSquareDifference) / double(totalNumberOfStereoFrames);
			result.absoluteDifference.varianceDifference-= (result.absoluteDifference.averageDifference * result.absoluteDifference.averageDifference);
			result.absoluteDifference.varianceDifference = sqrt(result.absoluteDifference.varianceDifference);
		}
		
		//Here we calculate the synchronicity using an external light source
		{
			enum ASFStates{NO_LIGHT, LEFTON_RIGHTOFF, LEFTOFF_RIGHTON, BOTH_LIGHT, ERROR_LIGHT} currentState = NO_LIGHT;
			enum ASFEvent{LEFT_TRIGGER, RIGHT_TRIGGER, BOTH_TRIGGER, NO_TRIGGER};
			ASFStates transitionMatrix[4][3] = {{LEFTON_RIGHTOFF, LEFTOFF_RIGHTON, BOTH_LIGHT}, {NO_LIGHT, BOTH_LIGHT, ERROR_LIGHT},
			{BOTH_LIGHT, NO_LIGHT, ERROR_LIGHT}, {LEFTOFF_RIGHTON, LEFTON_RIGHTOFF, NO_LIGHT}};
			std::vector<ImagePair>::const_iterator it = begin();
			long lastFrameID = 0;
			long totalDifference = 0;
			result.synchronizationStats.maxDifference = -1;
			result.synchronizationStats.minDifference = -1;
			result.synchronizationStats.numberOfDirectTransitions = 0;
			result.synchronizationStats.numberOfHalfTransitions = 0;
			while (it != end()) {
				ImagePair ip = *it;
				ASFEvent currentEvent = NO_TRIGGER;
				if (ip.images[0].triggered && ip.images[1].triggered)
					currentEvent = BOTH_TRIGGER;
				else if (ip.images[0].triggered)
					currentEvent = LEFT_TRIGGER;
				else if (ip.images[1].triggered)
					currentEvent = RIGHT_TRIGGER;
				ASFStates nextState = transitionMatrix[currentState][currentEvent];
				if (nextState == ERROR_LIGHT || currentState == nextState)
					//BOTH_TRIGGER in and intermediate state OR two equal single trigger in a row
					currentState = NO_LIGHT;
				else {
					if (currentState == LEFTON_RIGHTOFF || currentState == LEFTOFF_RIGHTON) {
						//One complete transition...we register the difference
						int currentDifference = ip.frameNumber-lastFrameID;
						totalDifference+=currentDifference;
						result.synchronizationStats.numberOfHalfTransitions++;
						LEO_CONSTRAINED_MAX_UPDATE(result.synchronizationStats.maxDifference, currentDifference, -1);
						LEO_CONSTRAINED_MIN_UPDATE(result.synchronizationStats.minDifference, currentDifference, -1);
					} else {
						if (nextState == NO_LIGHT || nextState == BOTH_LIGHT) {
							//Direct transition
							result.synchronizationStats.numberOfDirectTransitions++;
							LEO_CONSTRAINED_MAX_UPDATE(result.synchronizationStats.maxDifference, 0, -1);
							LEO_CONSTRAINED_MIN_UPDATE(result.synchronizationStats.minDifference, 0, -1);
						} else {
							//We are in an intermediate state...let's store the frame number
							lastFrameID = ip.frameNumber;
						}
					}
					currentState = nextState;
				}
				it++;
			}
			result.synchronizationStats.averageDifferece = (result.synchronizationStats.numberOfHalfTransitions == 0 ? -1.0 : (double(totalDifference)/double(result.synchronizationStats.numberOfHalfTransitions + result.synchronizationStats.numberOfDirectTransitions)));
		}

		return result;
	}
} synchronizationStore;

static struct SyncSource {
	WNDPROC previousFunction;
	HWND staticHandle;
	struct SelectedArea {
		CvPoint p1, p2;
	} selectedArea;
	CvRect selection;
	bool selectionChanged;
	bool drawingBox;
	HWND destHandle;
	float percentage;
	bool previousState;
} syncSource[2];

void RedrawBothStaticControl() {
	HWND statics[2] = {syncSource[0].destHandle, syncSource[1].destHandle};
	RedrawStaticControl(statics, 2);
}

long currentFrameNumber = 0;

static HWND createdDialog = NULL;

class SyncTestThread: public Thread {
private:
	EventRaiser *er;
	SyncSource* syncSource;
	HANDLE hEvent;
	bool testRunning;
public:
	SyncTestThread(EventRaiser *er, SyncSource* syncSource) {
		hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		this->er = er;
		this->syncSource = syncSource;
	}
	~SyncTestThread() {
		Stop();
		CloseHandle(hEvent);
	}
protected:
	void Run(void *param) {
		testRunning = true;
		ResetEvent(hEvent);
		er->SubscribeEvent(STEREO_IMAGE_READY, hEvent);

		StereoRig *asr = (StereoRig *) er;
		int imgHeight, imgWidth; asr->GetAcquisitionProperties().GetResolution(&imgHeight, &imgWidth, true);
		IplImage *originalColorImage[2] = {cvCreateImage(cvSize(imgWidth, imgHeight), IPL_DEPTH_8U, 3),
			cvCreateImage(cvSize(imgWidth, imgHeight), IPL_DEPTH_8U, 3)};
		IplImage *grayImage[2] = {cvCreateImage(cvSize(imgWidth, imgHeight), IPL_DEPTH_8U, 1),
			cvCreateImage(cvSize(imgWidth, imgHeight), IPL_DEPTH_8U, 1)};
		IplImage *maskImage[2] = {cvCreateImage(cvSize(imgWidth, imgHeight), IPL_DEPTH_8U, 1),
			cvCreateImage(cvSize(imgWidth, imgHeight), IPL_DEPTH_8U, 1)}; 
		HBITMAP hBitmap[2] = {NULL, NULL}; IplImage *iplBitmap[2];

		HDC dcs[2] = {GetDC(syncSource[0].destHandle), GetDC(syncSource[1].destHandle)};
		HDC memDcs[2] = {CreateCompatibleDC(dcs[0]), CreateCompatibleDC(dcs[1])};
		RECT retClient[2]; GetClientRect(syncSource[0].destHandle, &retClient[0]); GetClientRect(syncSource[1].destHandle, &retClient[1]);

		VisualizationChoice currentChoiceCopy = NONE_VISUALIZATION;
		while (testRunning) {
			DWORD res = WaitForSingleObject(hEvent, INFINITE);

			if (currentChoice != currentChoiceCopy)
				RedrawBothStaticControl();
			currentChoiceCopy = currentChoice;
			
			asr->GetStereoImages(&iplBitmap[0], &iplBitmap[1], false);
			asr->StereoLock.AcquireReadLock();
				StereoRig::StereoTimestampsStruct stereoTimestamps = asr->GetStereoImages(NULL, NULL, false);
				cvCopy(iplBitmap[0], originalColorImage[0]);
				cvCopy(iplBitmap[1], originalColorImage[1]);
			asr->StereoLock.ReleaseReadLock();

			ImagePair nextPair; ZeroMemory(&nextPair, sizeof(nextPair));

			for (int i = 0; i < 2; i++) {
				cvCvtColor(originalColorImage[i], grayImage[i], CV_BGR2GRAY);
				cvThreshold(grayImage[i], maskImage[i], int(lightPos) - 1, 255.0, CV_THRESH_BINARY);
				hBitmap[i] = IplImage2HBITMAP(syncSource[i].destHandle, hBitmap[i], currentChoiceCopy == RAW ? grayImage[i] : maskImage[i]);
				CvRect selection = syncSource[i].selection;
				if (syncSource[i].selectionChanged) {
					RedrawStaticControl(&syncSource[i].destHandle, 1);
					syncSource[i].selectionChanged = false;
				}
				if (selection.width && selection.height) {
					int numberOfHitters = 0;
					for (int row = 0; row < selection.height; row++) {
						const uchar* imgPtr = (const uchar*) (maskImage[i]->imageData + (row + selection.y)*maskImage[i]->widthStep + selection.x);
						for (int col = 0; col < selection.width; col++, imgPtr++) {
							if (*imgPtr > 0)
								numberOfHitters++;
						}
					}
					syncSource[i].percentage = float(numberOfHitters)/float(selection.height * selection.width) * 100.f;

					bool lightOn = (int(syncSource[i].percentage) >= threshPos);
					nextPair.images[i].triggered = LEO_XOR(lightOn, syncSource[i].previousState);
					syncSource[i].previousState = lightOn;

					if (currentChoiceCopy != EXPLORER) {
						SelectObject(memDcs[i], hBitmap[i]);
						BitBlt(dcs[i], 0, 0, MIN(selection.width, retClient[i].right-retClient[i].left),
							MIN(selection.height, retClient[i].bottom-retClient[i].top), memDcs[i],
							selection.x, selection.y, SRCCOPY);
						wchar_t percentageText[24]; swprintf_s(percentageText, 24, L"%.2f", syncSource[i].percentage);
						TextOut(dcs[i], retClient[i].right - 50, retClient[i].bottom-25, percentageText, int(wcslen(percentageText)));
					}
				} else {
					syncSource[i].percentage = 0.f;
					nextPair.images[i].triggered = false;
				}
				if (currentChoiceCopy == EXPLORER)
					synchronizationStore.showCurrentImage(syncSource[0].destHandle, syncSource[1].destHandle);
			}

			if (synchronizationStore.IsRecording()) {
				synchronizationStore.push_back(stereoTimestamps);
				nextPair.frameNumber = currentFrameNumber++;
				if (nextPair.images[0].triggered || nextPair.images[1].triggered) {
					nextPair.images[0].image = cvCloneImage(originalColorImage[0]);
					nextPair.images[1].image = cvCloneImage(originalColorImage[1]);
					synchronizationStore.push_back(nextPair);
				}
			}
		}
		ReleaseDC(syncSource[0].staticHandle, dcs[0]); ReleaseDC(syncSource[1].staticHandle, dcs[1]);
		DeleteDC(memDcs[0]); DeleteDC(memDcs[1]);
		DeleteObject(hBitmap[0]); DeleteObject(hBitmap[1]);
		er->UnSubscribeEvent(STEREO_IMAGE_READY, hEvent);
		cvReleaseImage(&grayImage[0]); cvReleaseImage(&grayImage[1]); cvReleaseImage(&maskImage[0]); cvReleaseImage(&maskImage[1]);
	}
	bool StopPreprocedure() {
		testRunning = false;
		return true;
	}
} *currentThread = NULL;

LRESULT CALLBACK AreaSelectionProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	int indexPic = (hwnd == syncSource[0].staticHandle ? 0 : 1);
	switch(uMsg) {
	case WM_SETCURSOR:
		{
			HCURSOR hc= LoadCursor(NULL, MAKEINTRESOURCE(IDC_CROSS));
			SetCursor(hc);
			return TRUE;
		}
	case WM_LBUTTONDOWN:
		{
			syncSource[indexPic].drawingBox = true;
			syncSource[indexPic].selectedArea.p1.x = GET_X_LPARAM(lParam);
			syncSource[indexPic].selectedArea.p1.y = GET_Y_LPARAM(lParam);
			syncSource[indexPic].selectedArea.p2 = syncSource[indexPic].selectedArea.p1;
			syncSource[indexPic].selection.x = syncSource[indexPic].selectedArea.p1.x;
			syncSource[indexPic].selection.y = syncSource[indexPic].selectedArea.p1.y;
			syncSource[indexPic].selection.width = syncSource[indexPic].selection.height = 0;
			syncSource[indexPic].selectionChanged = true;
			return 0;
		}
	case WM_LBUTTONUP:
		{
			syncSource[indexPic].drawingBox = false;
			return 0;
		}
	case WM_MOUSEMOVE:
		{
			if (syncSource[indexPic].drawingBox) {
				syncSource[indexPic].selectedArea.p2.x = MAX(0, GET_X_LPARAM(lParam));
				syncSource[indexPic].selectedArea.p2.y = MAX(0, GET_Y_LPARAM(lParam));
				syncSource[indexPic].selection.x = MIN(syncSource[indexPic].selectedArea.p1.x, syncSource[indexPic].selectedArea.p2.x);
				syncSource[indexPic].selection.y = MIN(syncSource[indexPic].selectedArea.p1.y, syncSource[indexPic].selectedArea.p2.y);
				syncSource[indexPic].selection.width = abs(syncSource[indexPic].selectedArea.p1.x - syncSource[indexPic].selectedArea.p2.x);
				syncSource[indexPic].selection.height = abs(syncSource[indexPic].selectedArea.p1.y - syncSource[indexPic].selectedArea.p2.y);
				syncSource[indexPic].selectionChanged = true;
			}
			return 0;
		}
	}
	return CallWindowProc(syncSource[indexPic].previousFunction, hwnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK SyncTestStatsProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
	case WM_INITDIALOG:
		{
			SynchronizationTestResults inputParams = *((SynchronizationTestResults *) lParam);
			wchar_t textBuf[256];
			/*
			swprintf_s(textBuf, 256, L"%f frames/sec", inputParams.stereoFrameRate);
			SetDlgItemText(hwndDlg, IDC_AVGFRAMERATE, textBuf);
			swprintf_s(textBuf, 256, L"%f msec", inputParams.absoluteDifference.averageDifference);
			SetDlgItemText(hwndDlg, IDC_AVGSTEREODIFFERENCE, textBuf);
			swprintf_s(textBuf, 256, L"%f msec", inputParams.absoluteDifference.varianceDifference);
			SetDlgItemText(hwndDlg, IDC_VARSTEREODIFFERENCE, textBuf);
			swprintf_s(textBuf, 256, L"%d msec", inputParams.absoluteDifference.maxDifference);
			SetDlgItemText(hwndDlg, IDC_MAXSTEREODIFFERENCE, textBuf);
			swprintf_s(textBuf, 256, L"%d msec", inputParams.absoluteDifference.minDifference);
			SetDlgItemText(hwndDlg, IDC_MINSTEREODIFFERENCE, textBuf);
			swprintf_s(textBuf, 256, L"%f frames", inputParams.synchronizationStats.averageDifferece);
			SetDlgItemText(hwndDlg, IDC_AVGDIFFSYNC, textBuf);
			swprintf_s(textBuf, 256, L"%d frames", inputParams.synchronizationStats.maxDifference);
			SetDlgItemText(hwndDlg, IDC_MAXDIFFERENCESYNC, textBuf);
			swprintf_s(textBuf, 256, L"%d frames", inputParams.synchronizationStats.minDifference);
			SetDlgItemText(hwndDlg, IDC_MINDIFFERENCESYNC, textBuf);
			swprintf_s(textBuf, 256, L"%d", inputParams.synchronizationStats.numberOfHalfTransitions);
			SetDlgItemText(hwndDlg, IDC_UNSYNCTRANSITIONS, textBuf);
			swprintf_s(textBuf, 256, L"%d", inputParams.synchronizationStats.numberOfDirectTransitions);
			SetDlgItemText(hwndDlg, IDC_DIRECTTRANSITIONS, textBuf);
			*/
			return TRUE;
		}
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		return TRUE;
	case WM_COMMAND:
		{
			WORD ctrlId = LOWORD(wParam);
			if (ctrlId == IDOK)
				EndDialog(hwndDlg, 0);
			return TRUE;
		}
	}
	return FALSE;
}
/*
INT_PTR CALLBACK SyncTestProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
	case WM_ACTIVATE:
		if (wParam == WA_INACTIVE)
			hActiveModelessWindow = NULL;
		else
			hActiveModelessWindow = hwndDlg;
		return TRUE;
	case WM_INITDIALOG:
		{
			synchronizationStore.clear();
			synchronizationStore.setContentLabel(GetDlgItem(hwndDlg, IDC_STATIC_POS), GetDlgItem (hwndDlg, IDC_COUPLETRIGGERS), GetDlgItem(hwndDlg, IDC_COUPLEID));
			SendDlgItemMessage(hwndDlg, IDC_SLIDER1, TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG(0, 255));
			SendDlgItemMessage(hwndDlg, IDC_SLIDER1, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) lightPos);
			SendDlgItemMessage(hwndDlg, IDC_SLIDER2, TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG(0, 100));
			SendDlgItemMessage(hwndDlg, IDC_SLIDER2, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) threshPos);
			EnableWindow(GetDlgItem(hwndDlg, IDC_STOPTEST), FALSE);
			PostMessage(hwndDlg, WM_HSCROLL, 0, 0);
			syncSource[0].destHandle = GetDlgItem(hwndDlg, IDC_STATIC_LEFT);
			syncSource[1].destHandle = GetDlgItem(hwndDlg, IDC_STATIC_RIGHT);
			CheckRadioButton(hwndDlg, IDC_RADIO1, IDC_RADIO3, IDC_RADIO1);
			currentChoice = RAW;
			currentThread->Start();
			return TRUE;
		}
	case WM_CLOSE:
		{
			if (!synchronizationStore.IsRecording()) {
				SetWindowLongPtr(syncSource[0].staticHandle, GWLP_WNDPROC, (LONG_PTR) syncSource[0].previousFunction);
				SetWindowLongPtr(syncSource[1].staticHandle, GWLP_WNDPROC, (LONG_PTR) syncSource[1].previousFunction);
				DestroyWindow(createdDialog);
				createdDialog = NULL;
				currentThread->Stop();
				delete currentThread; currentThread = NULL;
			}
			return TRUE;
		}
	case WM_HSCROLL:
		{
			wchar_t buf[16];
			lightPos = (DWORD) SendDlgItemMessage(hwndDlg, IDC_SLIDER1, TBM_GETPOS, 0, 0);
			swprintf_s(buf, 16, L"%u", lightPos); SetDlgItemText(hwndDlg, IDC_STATIC_LIGHT, buf);
			threshPos = (DWORD) SendDlgItemMessage(hwndDlg, IDC_SLIDER2, TBM_GETPOS, 0, 0);
			swprintf_s(buf, 16, L"%u%%", threshPos); SetDlgItemText(hwndDlg, IDC_STATIC_THRESH, buf);
			return TRUE;
		}
	case WM_COMMAND:
		{
			WORD ctrlId = LOWORD(wParam);
			if (ctrlId == IDC_RADIO1) {
				currentChoice = RAW;
			} else if (ctrlId == IDC_RADIO2) {
				currentChoice = THRESHOLDED;
			} else if (ctrlId == IDC_RADIO3) {
				currentChoice = EXPLORER;
			} else if (ctrlId == IDC_STARTTEST) {
				if (synchronizationStore.ContainsData()) {
					int yesno = MessageBox(hwndDlg, L"All the data previously saved will be lost. Continue?", L"Warning", MB_YESNO | MB_ICONWARNING);
					if (yesno == IDNO)
						return TRUE;
				}
				if (currentChoice == EXPLORER) {
					currentChoice = THRESHOLDED;
					CheckRadioButton(hwndDlg, IDC_RADIO1, IDC_RADIO3, IDC_RADIO2);
				}
				EnableWindow(GetDlgItem(hwndDlg, IDC_STARTTEST), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_STOPTEST), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_RADIO3), FALSE);
				synchronizationStore.clear();
				currentFrameNumber = 0;
				synchronizationStore.StartRecording();
			} else if (ctrlId == IDC_STOPTEST) {
				synchronizationStore.StopRecording();
				EnableWindow(GetDlgItem(hwndDlg, IDC_STARTTEST), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_STOPTEST), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_RADIO3), TRUE);
			} else if (ctrlId == IDC_FIRST) {
				synchronizationStore.showFirst();
			} else if (ctrlId == IDC_PREVIOUS) {
				synchronizationStore.showPrevious();
			} else if (ctrlId == IDC_NEXT) {
				synchronizationStore.showNext();
			} else if (ctrlId == IDC_LAST) {
				synchronizationStore.showLast();
			} else if (ctrlId == IDC_DELETE) {
				synchronizationStore.erase_current();
			} else if (ctrlId == IDC_ANALYZE) {
				SynchronizationTestResults syncStats = synchronizationStore.Analyze();
				//DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SYNCSTATS), hwndDlg, SyncTestStatsProc, (LPARAM) &syncStats);
			}
		}
		return TRUE;
	}
	return FALSE;
}
*/
void OpenSynchronizationTest(EventRaiser *er, HWND staticHandle[2]) {
	if (createdDialog == NULL) {
		ZeroMemory(syncSource, sizeof(syncSource));
		syncSource[0].staticHandle = staticHandle[0];
		syncSource[1].staticHandle = staticHandle[1];
		syncSource[0].previousFunction = (WNDPROC) SetWindowLongPtr(syncSource[0].staticHandle, GWLP_WNDPROC, (LONG_PTR) AreaSelectionProc);
		syncSource[1].previousFunction = (WNDPROC) SetWindowLongPtr(syncSource[1].staticHandle, GWLP_WNDPROC, (LONG_PTR) AreaSelectionProc);
		currentThread = new SyncTestThread(er, syncSource);
		//createdDialog = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SYNCTEST), NULL, SyncTestProc);
		ShowWindow(createdDialog, SW_SHOW);
	}
}