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
#include "stdafx.h"

#include "StereoCalibration.h"
#include "ApplicationWorkFlow.h"
#include <LeoWindowsGDI.h>
#include <LeoWindowsOpenCV.h>
#include <LeoLog4CPP.h>
#include <LeoWindowsGUI.h>
#include <LeoOpenCVExtension.h>
#include <Uxtheme.h>
#include "Shared.h"

#define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s << "\n";                   \
   OutputDebugStringW( os_.str().c_str() );  \
}


using namespace leostorm::logging;

extern SystemInfo *si;

StereoCalibration *StereoCalibration::instance = NULL;

StereoCalibration *StereoCalibration::GetInstance() {
	if (StereoCalibration::instance == NULL)
		StereoCalibration::instance = new StereoCalibration(8, 6, 40.f, 1);
	return StereoCalibration::instance;
}

StereoCalibration::StereoCalibration(const int nx, const int ny, const float squareSize, const int maxScale) {
	this->calibrationSettings.nx = nx; this->calibrationSettings.ny = ny;
	this->calibrationSettings.squareSize = squareSize;
	this->calibrationSettings.maxScale = maxScale;
	this->M_LEFT = NULL;
	this->M_RIGHT = NULL;
	this->D_LEFT = NULL;
	this->D_RIGHT = NULL;
	this->R = NULL;
	this->T = NULL;
	this->E = NULL;
	this->F = NULL;
	this->Q = NULL;
	this->mx_LEFT = NULL; this->my_LEFT = NULL;
	this->mx_RIGHT = NULL; this->my_RIGHT = NULL;
	stopEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	calibrationStatus = 0;
	currentDialog = NULL;
}

StereoCalibration::~StereoCalibration() {
	CloseHandle(stopEvent);
	CleanMemory();
}


void StereoCalibration::CleanMemory(CalibrationType ct) {
	if (ct == ALL_CALIBRATION) {
		calibrationStatus = 0;
		DBOUT("ct if");
	}
	else {
		calibrationStatus &= (~ct);
		DBOUT("ct else");
	}
	if (ct == LEFT_INTERNAL || ct == ALL_CALIBRATION) {
		DBOUT("LEFT");
		if (this->M_LEFT) cvReleaseMat(&this->M_LEFT);
		if (this->D_LEFT) cvReleaseMat(&this->D_LEFT);
	}
	if (ct == RIGHT_INTERNAL || ct == ALL_CALIBRATION) {
		DBOUT("RIGHT");
		if (this->M_RIGHT) cvReleaseMat(&this->M_RIGHT);
		if (this->D_RIGHT) cvReleaseMat(&this->D_RIGHT);
		calibrationStatus &= (~RIGHT_INTERNAL);
	}
	if (ct == STEREO || ct == ALL_CALIBRATION) {
		DBOUT("ALL");
		if (this->R) cvReleaseMat(&this->R);
		if (this->E) cvReleaseMat(&this->E);
		if (this->F) cvReleaseMat(&this->F);
		if (this->T) cvReleaseMat(&this->T);
		if (this->mx_LEFT) cvReleaseMat(&this->mx_LEFT);
		if (this->my_LEFT) cvReleaseMat(&this->my_LEFT);
		if (this->mx_RIGHT) cvReleaseMat(&this->mx_RIGHT);
		if (this->my_RIGHT) cvReleaseMat(&this->my_RIGHT);
		if (this->Q) cvReleaseMat(&this->Q);
	}
}


bool StereoCalibration::NewPair(IplImage *images[2]) {
	static HBITMAP winImage[2] = {NULL, NULL};
	int lrBase = 0; int lrMax = 2;
	if (calibrationSettings.lastCalibrationSelected == LEFT_INTERNAL) {
		lrMax = 1;
	} else if (calibrationSettings.lastCalibrationSelected == RIGHT_INTERNAL) {
		lrBase = 1;
	}
	int n = calibrationSettings.nx*calibrationSettings.ny, result, count;
	std::vector<CvPoint2D32f> temp[2];
	temp[0].resize(n); temp[1].resize(n);

	IplImage *gray[2] = {cvCreateImage(cvSize(images[0]->width, images[0]->height), IPL_DEPTH_8U, 1),
		cvCreateImage(cvSize(images[0]->width, images[0]->height), IPL_DEPTH_8U, 1)};
	cvCvtColor(images[LEFT_SIDE_CAMERA], gray[LEFT_SIDE_CAMERA], CV_BGR2GRAY);
	cvCvtColor(images[RIGHT_SIDE_CAMERA], gray[RIGHT_SIDE_CAMERA], CV_BGR2GRAY);

	for (int lr = lrBase; lr < lrMax; lr++) {
		std::vector<CvPoint2D32f> &tmp = temp[lr];
		result = false;
		for (int s = 1; s<=calibrationSettings.maxScale && !result; s++) {
			IplImage *timg = gray[lr];
			if (s > 1) {
				timg = cvCreateImage(cvSize(gray[lr]->width * s, gray[lr]->height * s),
					gray[lr]->depth, gray[lr]->nChannels);
				cvResize(gray[lr], timg, CV_INTER_AREA);
			}
			result = cvFindChessboardCorners(timg, cvSize(calibrationSettings.nx, calibrationSettings.ny), &tmp[0], &count,
				CV_CALIB_CB_ADAPTIVE_THRESH | CV_CALIB_CB_FILTER_QUADS);
			if (s > 1) {
				cvReleaseImage(&timg);
				if (result) {
					for (int j = 0; j < count; j++) {
						tmp[j].x /= s;
						tmp[j].y /= s;
					}
				}
			}
		}
		if (!result)
			return false;
	}

	IplImage *cimg = cvCreateImage(cvSize(images[0]->width, images[0]->height), IPL_DEPTH_8U, 3);
	for (int lr = 0; lr < 2; lr++) {
		std::vector<CvPoint2D32f> &tmp = temp[lr];
		cvCopy(images[lr], cimg);
		if (lr >= lrBase && lr < lrMax)
			cvDrawChessboardCorners(cimg, cvSize(calibrationSettings.nx, calibrationSettings.ny), &tmp[0], n, 1);
		winImage[lr] = IplImage2HBITMAP(calibrationSettings.destinationFrames[lr], winImage[lr], cimg);
		//Visualization
		RECT destRect; GetClientRect(calibrationSettings.destinationFrames[lr], &destRect);
		HDC chessboardDC = GetDC(calibrationSettings.destinationFrames[lr]);
		HDC memoryDC = CreateCompatibleDC(chessboardDC);
		SelectObject(memoryDC, winImage[lr]);
		AdaptiveBitBlt(chessboardDC, 0, 0, destRect.right-destRect.left, destRect.bottom-destRect.top, memoryDC,
			images[lr]->width, images[lr]->height);
		DeleteDC(memoryDC);
		ReleaseDC(calibrationSettings.destinationFrames[lr], chessboardDC);
	}
	cvReleaseImage(&cimg);

	for (int lr = lrBase; lr < lrMax; lr++) {
		std::vector<CvPoint2D32f> &pts = points[lr];
		std::vector<CvPoint2D32f> &tmp = temp[lr];
		int N = (int) pts.size();
		pts.resize(N + n, cvPoint2D32f(0, 0));
		cvFindCornerSubPix(gray[lr], &tmp[0], n, cvSize(11, 11), cvSize(-1, -1),
			cvTermCriteria(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS, 30, 0.01));
		std::copy(tmp.begin(), tmp.end(), pts.begin() + N);
	}

	cvReleaseImage(&gray[0]); cvReleaseImage(&gray[1]);

	return true;
}



bool StereoCalibration::StopPreprocedure() {
	SetEvent(stopEvent);
	return true;
}

void StereoCalibration::Run(void *param) {
	StereoRig *asr = si->GetStereoRig();

	ResetEvent(stopEvent);

	int n_frames = 0;
	points[0].resize(0);
	points[1].resize(0);
	int height, width;
	asr->GetAcquisitionProperties().GetResolution(&height, &width, true);
	HANDLE hStereoEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	asr->SubscribeEvent(STEREO_IMAGE_READY, hStereoEvent);
	IplImage * clonedColorImages[2] = {cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3),
		cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3)};
	char buf[1024];
	while (n_frames < 14 && WaitForSingleObject(stopEvent, 2000)==WAIT_TIMEOUT) {
		WaitForSingleObject(hStereoEvent, INFINITE);
		IplImage *images[2];
		Beep(750, 750);
		asr->StereoLock.AcquireReadLock();
			asr->GetStereoImages(&images[LEFT_SIDE_CAMERA], &images[RIGHT_SIDE_CAMERA], false);
			cvCopy(images[LEFT_SIDE_CAMERA], clonedColorImages[LEFT_SIDE_CAMERA]);
			cvCopy(images[RIGHT_SIDE_CAMERA], clonedColorImages[RIGHT_SIDE_CAMERA]);
		asr->StereoLock.ReleaseReadLock();
		if (NewPair(clonedColorImages))
			n_frames++;
	}
	
	cvReleaseImage(&clonedColorImages[LEFT_SIDE_CAMERA]);
	cvReleaseImage(&clonedColorImages[RIGHT_SIDE_CAMERA]);
	asr->UnSubscribeEvent(STEREO_IMAGE_READY, hStereoEvent);
	CloseHandle(hStereoEvent);

	if (n_frames < 14) {
		return;
	}

	CleanMemory(calibrationSettings.lastCalibrationSelected);

	int n = calibrationSettings.nx*calibrationSettings.ny;
	int N = n_frames*n;

	std::vector<CvPoint3D32f> objectPoints(N);
	for (int i = 0; i<calibrationSettings.ny; i++) {
		for (int j = 0; j<calibrationSettings.nx; j++) {
			objectPoints[i*calibrationSettings.nx + j] = cvPoint3D32f(i*calibrationSettings.squareSize, j*calibrationSettings.squareSize, 0);
		}
	}
	for (int i = 1; i<n_frames; i++) {
		std::copy(objectPoints.begin(), objectPoints.begin() + n, objectPoints.begin() + n * i);
	}

	std::vector<int> nPoints;
	nPoints.resize(n_frames, n);
	CvMat _nPoints = cvMat(1, (int) nPoints.size(), CV_32S, &nPoints[0]);

	if (calibrationSettings.lastCalibrationSelected == STEREO || calibrationSettings.lastCalibrationSelected == ALL_CALIBRATION) {	
		int stereoCalibrateFlags = (this->calibrationSettings.lastCalibrationSelected == STEREO ? CV_CALIB_FIX_INTRINSIC : CV_CALIB_FIX_ASPECT_RATIO);
		this->R = cvCreateMat(3, 3, CV_64FC1);
		this->T = cvCreateMat(3, 1, CV_64FC1);
		this->E = cvCreateMat(3, 3, CV_64FC1);
		this->F = cvCreateMat(3, 3, CV_64FC1);
		this->Q = cvCreateMat(4, 4, CV_64FC1);

		if (calibrationSettings.lastCalibrationSelected == ALL_CALIBRATION) {
			this->M_LEFT = cvCreateMat(3, 3, CV_64FC1); cvSetIdentity(this->M_LEFT);
			this->D_LEFT = cvCreateMat(1, 5, CV_64FC1); cvZero(this->D_LEFT);
			this->M_RIGHT = cvCreateMat(3, 3, CV_64FC1); cvSetIdentity(this->M_RIGHT);
			this->D_RIGHT = cvCreateMat(1, 5, CV_64FC1); cvZero(this->D_RIGHT);
		}

		CvMat _objectPoints = cvMat(1, N, CV_32FC3, &objectPoints[0]);
		CvMat _imagePointsL = cvMat(1, N, CV_32FC2, &points[LEFT_SIDE_CAMERA][0]);
		CvMat _imagePointsR = cvMat(1, N, CV_32FC2, &points[RIGHT_SIDE_CAMERA][0]);

		cvStereoCalibrate(& _objectPoints, & _imagePointsL, & _imagePointsR, & _nPoints, M_LEFT, D_LEFT, M_RIGHT,
			D_RIGHT, cvSize(width, height), R, T, E, F, stereoCalibrateFlags, cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 100, 1e-5));

		std::vector<CvPoint3D32f> lines[2];
		lines[0].resize(N);
		lines[1].resize(N);
		CvMat _L1 = cvMat(1, N, CV_32FC3, &lines[0][0]);
		CvMat _L2 = cvMat(1, N, CV_32FC3, &lines[1][0]);
		cvUndistortPoints(& _imagePointsL, & _imagePointsL, M_LEFT, D_LEFT, 0, M_LEFT);
		cvUndistortPoints(& _imagePointsR, & _imagePointsR, M_RIGHT, D_RIGHT, 0, M_RIGHT);
		cvComputeCorrespondEpilines(& _imagePointsL, 1, F, & _L1);
		cvComputeCorrespondEpilines(& _imagePointsR, 2, F, & _L2);
		double avgerr = 0;
		for (int i = 0; i<N; i++) {
			double err = fabs(points[0][i].x*lines[1][i].x + points[0][i].y*lines[1][i].y + lines[1][i].z)
				+ fabs(points[1][i].x*lines[0][i].x + points[1][i].y*lines[0][i].y + lines[0][i].z);
			avgerr+=err;
		}
		//Logger::writeToLOG(L"Stereo Calibration Average Error = %g\r\n", avgerr/N);

		this->mx_LEFT = cvCreateMat(height, width, CV_32F);
		this->my_LEFT = cvCreateMat(height, width, CV_32F);
		this->mx_RIGHT = cvCreateMat(height, width, CV_32F);
		this->my_RIGHT = cvCreateMat(height, width, CV_32F);

		double R1[3][3], R2[3][3];
		CvMat _R1 = cvMat(3, 3, CV_64F, R1);
		CvMat _R2 = cvMat(3, 3, CV_64F, R2);

		if (calibrationSettings.lastRectificationSelected == BOUGUET_RECTIFICATION) {
			// Bouguet Method 
			double P1[3][4], P2[3][4];
			CvMat _P1 = cvMat(3, 4, CV_64F, P1);
			CvMat _P2 = cvMat(3, 4, CV_64F, P2);
			cvStereoRectify(M_LEFT, M_RIGHT, D_LEFT, D_RIGHT, cvSize(width, height), R, T, & _R1, & _R2, & _P1, & _P2,
				Q, CV_CALIB_ZERO_DISPARITY);
			cvInitUndistortRectifyMap(M_LEFT, D_LEFT, & _R1, & _P1, mx_LEFT, my_LEFT);
			cvInitUndistortRectifyMap(M_RIGHT, D_RIGHT, & _R2, & _P2, mx_RIGHT, my_RIGHT);
		} else if (calibrationSettings.lastRectificationSelected == HARTLEY_RECTIFICATION) {
			// Hartley Method
			double H1[3][3], H2[3][3], iM[3][3];
			CvMat _H1 = cvMat(3, 3, CV_64F, H1);
			CvMat _H2 = cvMat(3, 3, CV_64F, H2);
			CvMat _iM = cvMat(3, 3, CV_64F, iM);
			cvStereoRectifyUncalibrated(& _imagePointsL, & _imagePointsR, F, cvSize(width, height), & _H1, & _H2, 3);
			cvInvert(M_LEFT, & _iM);
			cvMatMul(& _H1, M_LEFT, & _R1);
			cvMatMul(& _iM, & _R1, & _R1);
			cvInvert(M_RIGHT, & _iM);
			cvMatMul(& _H2, M_RIGHT, & _R2);
			cvMatMul(& _iM, & _R2, & _R2);
			cvInitUndistortRectifyMap(M_LEFT, D_LEFT, & _R1, M_LEFT, mx_LEFT, my_LEFT);
			cvInitUndistortRectifyMap(M_RIGHT, D_RIGHT, & _R2, M_RIGHT, mx_RIGHT, my_RIGHT);
		}
		sprintf_s(buf, 1024, "Stereo Calibration Error: %f", double(avgerr/N));
		MessageBoxA(currentDialog, buf, "Info", MB_OK | MB_ICONINFORMATION);
	} else {
		CvMat *intrinsicMatrix = NULL, *distortionMatrix = NULL;
		int chosenCamera = (calibrationSettings.lastCalibrationSelected == LEFT_INTERNAL ? LEFT_SIDE_CAMERA : RIGHT_SIDE_CAMERA);

		if (chosenCamera == LEFT_SIDE_CAMERA) {
			this->M_LEFT = intrinsicMatrix = cvCreateMat(3, 3, CV_64FC1);
			this->D_LEFT = distortionMatrix = cvCreateMat(1, 5, CV_64FC1);
		} else if (chosenCamera == RIGHT_SIDE_CAMERA) {
			this->M_RIGHT = intrinsicMatrix = cvCreateMat(3, 3, CV_64FC1);
			this->D_RIGHT = distortionMatrix = cvCreateMat(1, 5, CV_64FC1);
		}

		CvMat _objectPoints = cvMat(1, N, CV_32FC3, &objectPoints[0]);
		CvMat _imagePoints = cvMat(1, N, CV_32FC2, &points[chosenCamera][0]);
		cvSetIdentity(intrinsicMatrix);
		cvZero(distortionMatrix);

		double reprojectionError =cvCalibrateCamera2(&_objectPoints, &_imagePoints, &_nPoints, cvSize(width, height),
			intrinsicMatrix, distortionMatrix, 0, 0, CV_CALIB_FIX_ASPECT_RATIO);
		//Logger::writeToLOG(L"Single camera reprojection error: %f\r\n", reprojectionError);
		sprintf_s(buf, 1024, "Camera Projection Error: %f", reprojectionError);
		MessageBoxA(currentDialog, buf, "Info", MB_OK | MB_ICONINFORMATION);
	}
	if (calibrationSettings.lastCalibrationSelected == ALL_CALIBRATION) {
		calibrationStatus = (LEFT_INTERNAL | RIGHT_INTERNAL | STEREO);
	} else {
		calibrationStatus |= calibrationSettings.lastCalibrationSelected;
	}
	//UpdateLabels();
	StopCurrentCalibration();
	runningFlag = false;
}



void StereoCalibration::StartNewCalibration(CalibrationType ct) {
	if (ct==STEREO && (!(calibrationStatus & LEFT_INTERNAL) || !(calibrationStatus & RIGHT_INTERNAL))) {
		//MessageBox(currentDialog, L"Left and/or Right internal calibration data are not loaded", L"Error", MB_OK | MB_ICONERROR);
		return;
	}
	char buf[260];
	calibrationSettings.lastCalibrationSelected = ct;
	//GetDlgItemTextA(currentDialog, IDC_TEXELSIDE, buf, 260); calibrationSettings.squareSize = (float) atof(buf);
	//GetDlgItemTextA(currentDialog, IDC_ROWNUMBER, buf, 260); calibrationSettings.ny = atoi(buf);
	//GetDlgItemTextA(currentDialog, IDC_COLNUMBER, buf, 260); calibrationSettings.nx = atoi(buf);
	//EnableWindow(GetDlgItem(currentDialog, IDC_STARTLEFTINTERNAL), FALSE);
	//EnableWindow(GetDlgItem(currentDialog, IDC_STARTRIGHTINTERNAL), FALSE);
	//EnableWindow(GetDlgItem(currentDialog, IDC_STARTSTEREO), FALSE);
	//EnableWindow(GetDlgItem(currentDialog, IDC_STOPCURRENTCALIBRATION), TRUE);
	Start();
}


void StereoCalibration::StopCurrentCalibration() {
	/*
	if (IsDlgButtonChecked(currentDialog, IDC_SINGLECALIBRATIONCHECK)==BST_UNCHECKED) {
		EnableWindow(GetDlgItem(currentDialog, IDC_STARTLEFTINTERNAL), TRUE);
		EnableWindow(GetDlgItem(currentDialog, IDC_STARTRIGHTINTERNAL), TRUE);
	}
	EnableWindow(GetDlgItem(currentDialog, IDC_STARTSTEREO), TRUE);
	EnableWindow(GetDlgItem(currentDialog, IDC_STOPCURRENTCALIBRATION), FALSE);
	*/
}

void StereoCalibration::RectificationBouguet(bool singleCalibration) {
	StereoCalibration *sc = StereoCalibration::GetInstance();
	sc->calibrationSettings.lastRectificationSelected = BOUGUET_RECTIFICATION;
	if (!singleCalibration)
		sc->StartNewCalibration(ALL_CALIBRATION);
	else
		sc->StartNewCalibration(STEREO);
}


void StereoCalibration::RectificationHartley(bool singleCalibration) {
	StereoCalibration *sc = GetInstance();
	sc->calibrationSettings.lastRectificationSelected = HARTLEY_RECTIFICATION;
	if (!singleCalibration)
		sc->StartNewCalibration(ALL_CALIBRATION);
	else
		sc->StartNewCalibration(STEREO);
}

void StereoCalibration::StartLeft() {
	StereoCalibration *sc = GetInstance();
	sc->StartNewCalibration(LEFT_INTERNAL);
}
void StereoCalibration::StartRight() {
	StereoCalibration *sc = GetInstance();
	sc->StartNewCalibration(RIGHT_INTERNAL);
}
void StereoCalibration::StopCalibration() {
	StereoCalibration *sc = GetInstance();
	sc->Stop();
	sc->StopCurrentCalibration();
}

/*

void StereoCalibration::UpdateLabels() {
	if (calibrationStatus & LEFT_INTERNAL) {
		SetDlgItemText(currentDialog, IDC_LEFTINTERNALSTATIC, L"Loaded");
	} else {
		SetDlgItemText(currentDialog, IDC_LEFTINTERNALSTATIC, L"Not Loaded");
	}
	if (calibrationStatus & RIGHT_INTERNAL) {
		SetDlgItemText(currentDialog, IDC_RIGHTINTERNALSTATIC, L"Loaded");
	} else {
		SetDlgItemText(currentDialog, IDC_RIGHTINTERNALSTATIC, L"Not Loaded");
	}
	if (calibrationStatus & STEREO) {
		SetDlgItemText(currentDialog, IDC_STEREOSTATIC, L"Loaded");
	} else {
		SetDlgItemText(currentDialog, IDC_STEREOSTATIC, L"Not Loaded");
	}
}


INT_PTR CALLBACK StereoCalibration::StereoDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	StereoCalibration *sc = GetInstance();
	char buf[260];
	switch (uMsg) {
	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		return TRUE;
	case WM_INITDIALOG:
		{
			sc->currentDialog = hwndDlg;
			InternalState currentState = ApplicationWorkFlow::GetInstance()->GetCurrentState();
			EnableWindow(GetDlgItem(hwndDlg, IDC_STOPCURRENTCALIBRATION), FALSE);
			//Button_SetStyle(GetDlgItem(hwndDlg, IDC_STARTSTEREO), BS_SPLITBUTTON, TRUE);
			if (currentState == STANDBY) {
				EnableWindow(GetDlgItem(hwndDlg, IDC_STARTLEFTINTERNAL), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_STARTRIGHTINTERNAL), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_STARTSTEREO), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_SINGLECALIBRATIONCHECK), FALSE);
			}
			CheckDlgButton(hwndDlg, IDC_LEFTCHECK, BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_RIGHTCHECK, BST_CHECKED);
			CheckDlgButton(hwndDlg, IDC_STEREOCHECK, BST_CHECKED);
			sc->UpdateLabels();
			sc->calibrationSettings.destinationFrames[LEFT_SIDE_CAMERA] = GetDlgItem(hwndDlg, IDC_CHESSBOARDLEFT);
			sc->calibrationSettings.destinationFrames[RIGHT_SIDE_CAMERA] = GetDlgItem(hwndDlg, IDC_CHESSBOARDRIGHT);
			sprintf_s(buf, 260, "%.2f", sc->calibrationSettings.squareSize); SetDlgItemTextA(hwndDlg, IDC_TEXELSIDE, buf);
			sprintf_s(buf, 260, "%d", sc->calibrationSettings.ny); SetDlgItemTextA(hwndDlg, IDC_ROWNUMBER, buf);
			sprintf_s(buf, 260, "%d", sc->calibrationSettings.nx); SetDlgItemTextA(hwndDlg, IDC_COLNUMBER, buf);

			return TRUE;
		}
	case WM_COMMAND:
		{
			WORD ctrlId = LOWORD(wParam);
			int selectedMask = 0;
			if (IsDlgButtonChecked(hwndDlg, IDC_LEFTCHECK)==BST_CHECKED)
				selectedMask |= LEFT_INTERNAL;
			if (IsDlgButtonChecked(hwndDlg, IDC_RIGHTCHECK)==BST_CHECKED)
				selectedMask |= RIGHT_INTERNAL;
			if (IsDlgButtonChecked(hwndDlg, IDC_STEREOCHECK)==BST_CHECKED)
				selectedMask |= STEREO;
			if (ctrlId == IDC_SAVECALIBRATION && selectedMask != 0) {
				if (UseCommonItemDialog(buf, sizeof(buf), hwndDlg, CLSID_FileOpenDialog, FOS_PICKFOLDERS))
					GetInstance()->SaveToFolder(buf, selectedMask);
			} else if (ctrlId == IDC_LOADCALIBRATION && selectedMask != 0) {
				if (UseCommonItemDialog(buf, sizeof(buf), hwndDlg, CLSID_FileOpenDialog, FOS_PICKFOLDERS))
					GetInstance()->LoadFromFolder(buf, selectedMask);
					DBOUT(buf)
					DBOUT(selectedMask)
			} else if (ctrlId == IDC_STARTSTEREO) {
				HMENU hPopupMenu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_RECTIFICATIONMENU));
				SetForegroundWindow(hwndDlg);
				RECT buttonRect; GetWindowRect((HWND) lParam, &buttonRect);
				TrackPopupMenu(GetSubMenu(hPopupMenu, 0), TPM_TOPALIGN | TPM_LEFTALIGN, buttonRect.left, buttonRect.bottom,
					0, hwndDlg, NULL);
				DestroyMenu(hPopupMenu);
			} else if (ctrlId == ID_RECTIFICATION_BOUGUETRECTIFICATION) {
				sc->calibrationSettings.lastRectificationSelected = BOUGUET_RECTIFICATION;
				if (IsDlgButtonChecked(hwndDlg, IDC_SINGLECALIBRATIONCHECK)==BST_CHECKED)
					sc->StartNewCalibration(ALL_CALIBRATION);
				else
					sc->StartNewCalibration(STEREO);
			} else if (ctrlId == ID_RECTIFICATION_HARTLEYRECTIFICATION) {
				sc->calibrationSettings.lastRectificationSelected = HARTLEY_RECTIFICATION;
				if (IsDlgButtonChecked(hwndDlg, IDC_SINGLECALIBRATIONCHECK)==BST_CHECKED)
					sc->StartNewCalibration(ALL_CALIBRATION);
				else
					sc->StartNewCalibration(STEREO);
			} else if (ctrlId == IDC_STARTLEFTINTERNAL) {
				sc->StartNewCalibration(LEFT_INTERNAL);
			} else if (ctrlId == IDC_STARTRIGHTINTERNAL) {
				sc->StartNewCalibration(RIGHT_INTERNAL);
			} else if (ctrlId == IDC_STOPCURRENTCALIBRATION) {
				sc->Stop();
				sc->StopCurrentCalibration();
			} else if (ctrlId == IDC_SINGLECALIBRATIONCHECK) {
				if (IsDlgButtonChecked(hwndDlg, IDC_SINGLECALIBRATIONCHECK)==BST_CHECKED) {
					EnableWindow(GetDlgItem(hwndDlg, IDC_STARTLEFTINTERNAL), FALSE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_STARTRIGHTINTERNAL), FALSE);
				} else {
					EnableWindow(GetDlgItem(hwndDlg, IDC_STARTLEFTINTERNAL), TRUE);
					EnableWindow(GetDlgItem(hwndDlg, IDC_STARTRIGHTINTERNAL), TRUE);
				}
			} else if (ctrlId == IDC_RECTIFICATIONBYPARAMETERS) {
				StereoCalibration *csc = StereoCalibration::GetInstance();
				if (csc->D_LEFT == NULL || csc->M_LEFT == NULL || csc->D_RIGHT == NULL || csc->M_RIGHT == NULL || csc->R == NULL || csc->T == NULL) {
					MessageBox(hwndDlg, L"Internals or Rotation or Traslation Matrix have not been loaded yet", L"Error", MB_ICONERROR);
					return TRUE;
				}

				csc->Q = cvCreateMat(4, 4, CV_64FC1);

				csc->mx_LEFT = cvCreateMat(240, 320, CV_32F);
				csc->my_LEFT = cvCreateMat(240, 320, CV_32F);
				csc->mx_RIGHT = cvCreateMat(240, 320, CV_32F);
				csc->my_RIGHT = cvCreateMat(240, 320, CV_32F);

				double R1[3][3], R2[3][3];
				CvMat _R1 = cvMat(3, 3, CV_64F, R1);
				CvMat _R2 = cvMat(3, 3, CV_64F, R2);

				// Bouguet Method 
				double P1[3][4], P2[3][4];
				CvMat _P1 = cvMat(3, 4, CV_64F, P1);
				CvMat _P2 = cvMat(3, 4, CV_64F, P2);
				cvStereoRectify(csc->M_LEFT, csc->M_RIGHT, csc->D_LEFT, csc->D_RIGHT, cvSize(320, 240),
					csc->R, csc->T, & _R1, & _R2, & _P1, & _P2, csc->Q, CV_CALIB_ZERO_DISPARITY);
				cvInitUndistortRectifyMap(csc->M_LEFT, csc->D_LEFT, & _R1, & _P1, csc->mx_LEFT, csc->my_LEFT);
				cvInitUndistortRectifyMap(csc->M_RIGHT, csc->D_RIGHT, & _R2, & _P2, csc->mx_RIGHT, csc->my_RIGHT);

				MessageBox(hwndDlg, L"Bouget Rectification Done", L"OK", MB_ICONINFORMATION);
			}
			return TRUE;
		}
	}
	return FALSE;
}


void StereoCalibration::OpenDialogWindow(HWND parentWindow) {
	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_STEREO_CALIBRATION), parentWindow, StereoDialogProc);
}
*/

bool StereoCalibration::IsComplete() {
	return (calibrationStatus & LEFT_INTERNAL) && (calibrationStatus & RIGHT_INTERNAL) && (calibrationStatus & STEREO);
}

bool StereoCalibration::SaveToFolder(const char *folderName, int mask) {
	int maxLen = ((int) strlen(folderName)) + 255;
	char * tempStr = new char[maxLen];
	if ((mask & LEFT_INTERNAL) && (calibrationStatus & LEFT_INTERNAL)) {
		sprintf_s(tempStr, maxLen, "%s\\LeftIntrinsics.xml", folderName); cvSave(tempStr, M_LEFT);
		sprintf_s(tempStr, maxLen, "%s\\LeftDistortion.xml", folderName); cvSave(tempStr, D_LEFT);
	}
	if ((mask & RIGHT_INTERNAL) && (calibrationStatus & RIGHT_INTERNAL)) {
		sprintf_s(tempStr, maxLen, "%s\\RightIntrinsics.xml", folderName); cvSave(tempStr, M_RIGHT);
		sprintf_s(tempStr, maxLen, "%s\\RightDistortion.xml", folderName); cvSave(tempStr, D_RIGHT);
	}
	if ((mask & STEREO) && (calibrationStatus & STEREO)) {
		sprintf_s(tempStr, maxLen, "%s\\Rotation.xml", folderName); cvSave(tempStr, R);
		sprintf_s(tempStr, maxLen, "%s\\Traslation.xml", folderName); cvSave(tempStr, T);
		sprintf_s(tempStr, maxLen, "%s\\Fundamental.xml", folderName); cvSave(tempStr, F);
		sprintf_s(tempStr, maxLen, "%s\\Essential.xml", folderName); cvSave(tempStr, E);
		sprintf_s(tempStr, maxLen, "%s\\mx_LEFT.xml", folderName); cvSave(tempStr, mx_LEFT);
		sprintf_s(tempStr, maxLen, "%s\\my_LEFT.xml", folderName); cvSave(tempStr, my_LEFT);
		sprintf_s(tempStr, maxLen, "%s\\mx_RIGHT.xml", folderName); cvSave(tempStr, mx_RIGHT);
		sprintf_s(tempStr, maxLen, "%s\\my_RIGHT.xml", folderName); cvSave(tempStr, my_RIGHT);
		sprintf_s(tempStr, maxLen, "%s\\3DReprojection.xml", folderName); cvSave(tempStr, Q);
	}
	delete[] tempStr;
	return true;
}

bool StereoCalibration::LoadFromFolder(const char *folderName, int mask) {
	DBOUT(mask);
	DBOUT(LEFT_INTERNAL);
	DBOUT(RIGHT_INTERNAL);
	DBOUT(STEREO);
	DBOUT(M_LEFT);
	DBOUT(D_LEFT);
	DBOUT(R);
	DBOUT(F);
	DBOUT(E);
	DBOUT(mx_LEFT);
	DBOUT(my_LEFT);
	DBOUT(mx_RIGHT);
	DBOUT(my_RIGHT);
	DBOUT(calibrationStatus);
	int maxLen = int(strlen(folderName)) + 255;
	char * tempStr = new char[maxLen];
	if (mask & LEFT_INTERNAL) {
		CleanMemory(LEFT_INTERNAL);
		sprintf_s(tempStr, maxLen, "%s\\LeftIntrinsics.xml", folderName);
		DBOUT(tempStr);
		M_LEFT = (CvMat *) cvLoad(tempStr);
		sprintf_s(tempStr, maxLen, "%s\\LeftDistortion.xml", folderName); D_LEFT = (CvMat *) cvLoad(tempStr);
		DBOUT(calibrationStatus);
		DBOUT(M_LEFT);
		DBOUT(D_LEFT);
		if (M_LEFT && D_LEFT) {
			calibrationStatus |= LEFT_INTERNAL;
			DBOUT(calibrationStatus);
		}
		DBOUT(calibrationStatus);
	}
	if (mask & RIGHT_INTERNAL) {
		CleanMemory(RIGHT_INTERNAL);
		sprintf_s(tempStr, maxLen, "%s\\RightDistortion.xml", folderName); D_RIGHT = (CvMat *) cvLoad(tempStr);
		sprintf_s(tempStr, maxLen, "%s\\RightIntrinsics.xml", folderName); M_RIGHT = (CvMat *) cvLoad(tempStr);
		DBOUT(calibrationStatus);
		if (M_RIGHT && D_RIGHT)
			calibrationStatus |= RIGHT_INTERNAL;
		DBOUT(calibrationStatus);

	}
	if (mask & STEREO) {
		CleanMemory(STEREO);
		sprintf_s(tempStr, maxLen, "%s\\Rotation.xml", folderName); R = (CvMat *) cvLoad(tempStr);
		sprintf_s(tempStr, maxLen, "%s\\Traslation.xml", folderName); T = (CvMat *) cvLoad(tempStr);
		sprintf_s(tempStr, maxLen, "%s\\Fundamental.xml", folderName); F = (CvMat *) cvLoad(tempStr);
		sprintf_s(tempStr, maxLen, "%s\\Essential.xml", folderName); E = (CvMat *) cvLoad(tempStr);
		sprintf_s(tempStr, maxLen, "%s\\mx_LEFT.xml", folderName); mx_LEFT = (CvMat *) cvLoad(tempStr);
		sprintf_s(tempStr, maxLen, "%s\\my_LEFT.xml", folderName); my_LEFT = (CvMat *) cvLoad(tempStr);
		sprintf_s(tempStr, maxLen, "%s\\mx_RIGHT.xml", folderName); mx_RIGHT = (CvMat *) cvLoad(tempStr);
		sprintf_s(tempStr, maxLen, "%s\\my_RIGHT.xml", folderName); my_RIGHT = (CvMat *) cvLoad(tempStr);
		sprintf_s(tempStr, maxLen, "%s\\3DReprojection.xml", folderName); Q = (CvMat *) cvLoad(tempStr);
		DBOUT(calibrationStatus);
		if (R && R && F && E && mx_LEFT && my_LEFT && mx_RIGHT && my_RIGHT)
			calibrationStatus |= STEREO;
		DBOUT(calibrationStatus);

		//WARNING: VERY BAD CODE TO CHANGE MAPPING
		/*for (int i = 0; i < my_RIGHT->rows; i++) {
			float *my_right_ptr = (float *) (my_RIGHT->data.ptr + (i * my_RIGHT->step));
			for (int j = 0; j < my_RIGHT->cols; j++, my_right_ptr++) {
				*my_right_ptr= *my_right_ptr - 3.f;
			}
		}*/
		//END OF MAPPING CODE
	}
	printf("%d\n",calibrationStatus);
	delete[] tempStr;
	//UpdateLabels();
	return true;
}

//External Calibration Implementation

ExternalCalibration *ExternalCalibration::instance = NULL;

ExternalCalibration::ExternalCalibration() {
	hLeftImageReady = CreateEvent(NULL, FALSE, FALSE, NULL);
	currentSnapshot = cvCreateImage(cvSize(320, 240), IPL_DEPTH_8U, 3);
	Rotation_Matrix = Traslation_Vector = NULL;
	loaded = false;
}

ExternalCalibration::~ExternalCalibration() {
	CloseHandle(hLeftImageReady);
	if (this->currentSnapshot)
		cvReleaseImage(&currentSnapshot);
	CleanMemory();

}

void ExternalCalibration::CleanMemory() {
	if (Rotation_Matrix) cvReleaseMat(&Rotation_Matrix);
	if (Traslation_Vector) cvReleaseMat(&Traslation_Vector);
}

ExternalCalibration * ExternalCalibration::GetInstance() {
	if (ExternalCalibration::instance==NULL) {
		ExternalCalibration::instance = new ExternalCalibration();
	}
	return ExternalCalibration::instance;
}

/*

void ExternalCalibration::OpenDialogWindow(HWND parentWnd) {
	hwndDlg = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_EXTERNAL_CALIB), parentWnd,
		ExternalCalibration::DlgProc);
	cvNamedWindow("External Calibration View", CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback("External Calibration View", ExternalCalibration::MouseCallback, this);
	hwndImage = (HWND) cvGetWindowHandle("External Calibration View");
	hwndImage = (HWND) GetWindowLongPtr(hwndImage, GWLP_HWNDPARENT);
	ShowWindow(hwndImage, SW_HIDE);

	SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR) this);
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST1);
	LVCOLUMN lc;
	lc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lc.fmt = LVCFMT_LEFT;
	
	lc.cx = 60;
	lc.pszText = L"px";
	lc.iSubItem = 0;
	ListView_InsertColumn(hwndList, 0, &lc);
	lc.pszText = L"py";
	lc.iSubItem = 1;
	ListView_InsertColumn(hwndList, 1, &lc);
	lc.pszText = L"X";
	lc.iSubItem = 2;
	ListView_InsertColumn(hwndList, 2, &lc);
	lc.pszText = L"Y";
	lc.iSubItem = 3;
	ListView_InsertColumn(hwndList, 3, &lc);
	lc.pszText = L"Z";
	lc.iSubItem = 4;
	ListView_InsertColumn(hwndList, 4, &lc);
	
	ShowWindow(hwndDlg, SW_SHOW);
}


INT_PTR CALLBACK ExternalCalibration::DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	ExternalCalibration *ec = (ExternalCalibration *) GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (uMsg) {
	case WM_ACTIVATE:
		if (wParam == WA_INACTIVE)
			hActiveModelessWindow = NULL;
		else
			hActiveModelessWindow = hwndDlg;
		return TRUE;
	case WM_INITDIALOG:
		{
			HWND pointList = GetDlgItem(hwndDlg, IDC_LIST1);
			DWORD dwStyle = (DWORD) SendMessage(pointList, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
			dwStyle |= (LVS_EX_DOUBLEBUFFER | LVS_EX_FULLROWSELECT);
			SendMessage(pointList,LVM_SETEXTENDEDLISTVIEWSTYLE,0,dwStyle);
			SetWindowTheme(pointList, L"Explorer", NULL);
			ApplicationWorkFlow::GetInstance()->UpdateSystemState(EXTERNAL_CALIBRATION_STARTED);
		}
		return TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL) {
			cvDestroyWindow("External Calibration View");
			cvDestroyWindow("Zoom");
			EndDialog(hwndDlg, 0);
			ApplicationWorkFlow::GetInstance()->UpdateSystemState(CALIBRATION_ENDED);
		} else if (LOWORD(wParam) == IDOK) {
			std::vector<CvPoint2D32f> pixels; std::vector<CvPoint3D32f> worldCoo;
			int N = ec->FillVectors(&pixels, &worldCoo);
			if (N>=4 && StereoCalibration::GetInstance()->IsComplete()) {
				CvMat *Rot_Vector = cvCreateMat(3, 1, CV_64FC1);
				ec->CleanMemory();
				ec->Rotation_Matrix = cvCreateMat(3, 3, CV_64FC1);
				ec->Traslation_Vector = cvCreateMat(3, 1, CV_64FC1);
				CvMat wc = cvMat(1, N, CV_32FC3, &worldCoo[0]);
				CvMat pc = cvMat(1, N, CV_32FC2, &pixels[0]);
				cvFindExtrinsicCameraParams2(&wc, &pc, StereoCalibration::GetInstance()->M_LEFT,
					StereoCalibration::GetInstance()->D_LEFT, Rot_Vector, ec->Traslation_Vector);
				cvRodrigues2(Rot_Vector, ec->Rotation_Matrix);
				cvReleaseMat(&Rot_Vector);
				cvInvert(ec->Rotation_Matrix, ec->Rotation_Matrix);
				Logger::writeToLOG(L"External calibration successfully completed\r\n");
				cvDestroyWindow("Zoom");
				cvDestroyWindow("External Calibration View");
				EndDialog(hwndDlg, 0);
				ec->loaded = true;
				ApplicationWorkFlow::GetInstance()->UpdateSystemState(CALIBRATION_ENDED);
			} else {
				if (N<4)
					MessageBox(hwndDlg, L"The algorithm needs at least 4 points", L"Incomplete Data", MB_OK);
				else
					MessageBox(hwndDlg, L"You have to load stereo calibration data", L"Incomplete Data", MB_OK);
			}
		} else if (LOWORD(wParam) == IDOK2) {
			for (int itemToDelete = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_LIST1), -1, LVNI_SELECTED); itemToDelete != -1; itemToDelete = ListView_GetNextItem(GetDlgItem(hwndDlg, IDC_LIST1), itemToDelete, LVNI_SELECTED)) {
				ListView_DeleteItem(GetDlgItem(hwndDlg, IDC_LIST1), itemToDelete);
			}
		} else if (LOWORD(wParam) == IDC_BUTTON1) {
			if (si->GetStereoRig()) {
				ShowWindow(ec->hwndImage, SW_SHOW);
				ListView_DeleteAllItems(GetDlgItem(hwndDlg, IDC_LIST1));
				si->GetStereoRig()->GetAcquisitionCamera(LEFT_SIDE_CAMERA)->SubscribeEvent(IMAGE_READY, ec->hLeftImageReady);
				WaitForSingleObject(ec->hLeftImageReady, INFINITE);
				si->GetStereoRig()->GetAcquisitionCamera(LEFT_SIDE_CAMERA)->ImageLock.AcquireReadLock();
				cvResize(si->GetStereoRig()->GetAcquisitionCamera(LEFT_SIDE_CAMERA)->GetImage(), ec->currentSnapshot,
					CV_INTER_AREA);
				//si->GetStereoRig()->GetStereoImages(&ec->currentSnapshot, NULL, false);
				si->GetStereoRig()->GetAcquisitionCamera(LEFT_SIDE_CAMERA)->ImageLock.ReleaseReadLock();
				si->GetStereoRig()->GetAcquisitionCamera(RIGHT_SIDE_CAMERA)->UnSubscribeEvent(IMAGE_READY, ec->hLeftImageReady);

				cvShowImage("External Calibration View", ec->currentSnapshot);

			}
		}
		return TRUE;
	}
	return FALSE;
}

INT_PTR CALLBACK ExternalCalibration::DlgWorldProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static CvPoint3D32f *punto = NULL;
	switch (uMsg) {
		case WM_INITDIALOG:
			{
				punto = (CvPoint3D32f *) lParam;
				SetFocus(GetDlgItem(hwndDlg, IDC_EDIT2));
			}
			return FALSE; //We return FALSE because we want the focus set to a desired control
		case WM_COMMAND:
			if (LOWORD(wParam) == IDCANCEL) {
				EndDialog(hwndDlg, 0);
			} else if (LOWORD(wParam) == IDOK) {
				wchar_t temp[20];
				GetDlgItemText(hwndDlg, IDC_EDIT2, temp, 20);
				punto->x = (float) _wtof(temp);
				GetDlgItemText(hwndDlg, IDC_EDIT3, temp, 20);
				punto->y = (float) _wtof(temp);
				GetDlgItemText(hwndDlg, IDC_EDIT4, temp, 20);
				punto->z = (float) _wtof(temp);
				EndDialog(hwndDlg, 1);
			}
			return TRUE;
	}

	return FALSE;
}


void ExternalCalibration::MouseCallback(int ev, int x, int y, int flags, void *param) {
	ExternalCalibration *ec = (ExternalCalibration *) param;
	switch (ev) {
		case CV_EVENT_LBUTTONDOWN:
			{
				CvPoint2D32f p; p.x = (float) x; p.y = (float) y;
				CvPoint3D32f w;
				int res = (int) DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_WORLDCOO),
					ec->hwndDlg, ExternalCalibration::DlgWorldProc, (LPARAM) &w);
				if (res == 1)
					ec->AddToListView(p, w);
			}
			break;
		case CV_EVENT_MOUSEMOVE:
			{
				wchar_t temp[20];
				swprintf_s(temp, 20, L"%d,%d", x, y);
				SetDlgItemText(ec->hwndDlg, IDC_STATIC1, temp);
				IplImage *zoomImg = cvCreateImage(cvSize(153, 153), IPL_DEPTH_8U, 3);
				IplImage *frameImg = cvCreateImage(cvSize(ec->currentSnapshot->width+50, ec->currentSnapshot->height+50), IPL_DEPTH_8U, 3);
				CreateFrame(ec->currentSnapshot, frameImg, cvScalar(0,0,0));
				cvSetImageROI(frameImg, cvRect(x, y, 51, 51));
				cvResize(frameImg, zoomImg, CV_INTER_AREA);
				cvResetImageROI(frameImg);
				cvLine(zoomImg, cvPoint(76, 0), cvPoint(76, 152), cvScalar(0,0,0));
				cvLine(zoomImg, cvPoint(0, 76), cvPoint(152, 76), cvScalar(0,0,0));
				cvShowImage("Zoom", zoomImg);
				cvReleaseImage(&zoomImg);
				cvReleaseImage(&frameImg);
			}
			break;
	}
}

void ExternalCalibration::AddToListView(CvPoint2D32f pixel, CvPoint3D32f world) {
	wchar_t temp[20];
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST1);
	int newPos = ListView_GetItemCount(hwndList);
	
	if (currentSnapshot) {
		CvPoint p; p.x = (int) pixel.x; p.y = (int) pixel.y;
		cvCircle(currentSnapshot, p, 5, CV_RGB(255, 0, 0));
		cvShowImage("External Calibration View", currentSnapshot);
	}

	LVITEM li;
	li.mask = LVIF_TEXT | LVIF_STATE;
	li.state = 0; li.stateMask = 0;
	li.iItem = newPos;
	li.pszText = temp;
	swprintf_s(temp, 20, L"%.2f", pixel.x);
	li.iSubItem = 0;
	ListView_InsertItem(hwndList, &li);
	
	swprintf_s(temp, 20, L"%.2f", pixel.y);
	ListView_SetItemText(hwndList, newPos, 1, temp);

	swprintf_s(temp, 20, L"%.2f", world.x);
	ListView_SetItemText(hwndList, newPos, 2, temp);

	swprintf_s(temp, 20, L"%.2f", world.y);
	ListView_SetItemText(hwndList, newPos, 3, temp);

	swprintf_s(temp, 20, L"%.2f", world.z);
	ListView_SetItemText(hwndList, newPos, 4, temp);

	int numberOfLearningMeasurement = ListView_GetItemCount(hwndList);
	if (numberOfLearningMeasurement > 0)
		ListView_EnsureVisible(hwndList, numberOfLearningMeasurement - 1, FALSE);
}

int ExternalCalibration::FillVectors(std::vector<CvPoint2D32f> *pixels, std::vector<CvPoint3D32f> *worldCoo) {
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST1);
	int totElem = ListView_GetItemCount(hwndList);
	pixels->resize(totElem); worldCoo->resize(totElem);
	wchar_t temp[20];

	for (int i = 0; i<totElem; i++) {
		CvPoint2D32f pix;
		ListView_GetItemText(hwndList, i, 0, temp, 20);
		pix.x = (float) _wtof(temp);
		ListView_GetItemText(hwndList, i, 1, temp, 20);
		pix.y = (float) _wtof(temp);
		(*pixels)[i] = pix;

		CvPoint3D32f wor;
		ListView_GetItemText(hwndList, i, 2, temp, 20);
		wor.x = (float) _wtof(temp);
		ListView_GetItemText(hwndList, i, 3, temp, 20);
		wor.y = (float) _wtof(temp);
		ListView_GetItemText(hwndList, i, 4, temp, 20);
		wor.z = (float) _wtof(temp);
		(*worldCoo)[i] = wor;
	}

	return totElem;
}
*/

bool ExternalCalibration::SaveToFolder(const char *folderName) {
	if (Rotation_Matrix && Traslation_Vector) {
		int maxLen = ((int) strlen(folderName)) + 255;
		char * tempStr = new char[maxLen];
		sprintf_s(tempStr, maxLen, "%s\\External_Rotation.xml", folderName); cvSave(tempStr, Rotation_Matrix);
		sprintf_s(tempStr, maxLen, "%s\\External_Traslation.xml", folderName); cvSave(tempStr, Traslation_Vector);
		delete[] tempStr;
		return true;
	} else {
		return false;
	}
}



bool ExternalCalibration::LoadFromFolder(const char *folderName) {
	CvMat *rm, *tv;
	int maxLen = int(strlen(folderName)) + 255;
	char * tempStr = new char[maxLen];
	sprintf_s(tempStr, maxLen, "%s\\External_Rotation.xml", folderName); rm = (CvMat *) cvLoad(tempStr);
	sprintf_s(tempStr, maxLen, "%s\\External_Traslation.xml", folderName); tv = (CvMat *) cvLoad(tempStr);
	delete[] tempStr;
	if ((rm && (rm->rows != 3 || rm->cols != 3)) ||	(tv && (tv->rows != 3 || tv->cols != 1))) {
		cvReleaseMat(&rm);
		cvReleaseMat(&tv);
		return false;
	} else {
		CleanMemory();
		Rotation_Matrix = rm;
		Traslation_Vector = tv;
		loaded = true;
		return true;
	}
}

bool ExternalCalibration::IsComplete() {
	return loaded;
}