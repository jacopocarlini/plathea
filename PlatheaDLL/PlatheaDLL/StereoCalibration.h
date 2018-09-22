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

#ifndef STEREO_CALIBRATION
#define STEREO_CALIBRATION

#include "CommonHeader.h"
#include <LeoWindowsThread.h>
#include "NetworkCamera.h"

class StereoCalibration: public Thread {
public:
	~StereoCalibration();
	bool SaveToFolder(const char * folderName, int mask);
	bool LoadFromFolder(const char * folderName, int mask);
	void RectificationBouguet(bool singleCalibration);
	void RectificationHartley(bool singleCalibration);
	void StartLeft();
	void StartRight();
	void StopCalibration();
	CvMat *M_LEFT, *M_RIGHT; //Cameras intrinsics matrix
	CvMat *D_LEFT, *D_RIGHT; //Distortion vectors
	CvMat *R; //Rotation matrix
	CvMat *T; //Traslation vector
	CvMat *E; //Essential matrix
	CvMat *F; //Fundamental matrix
	CvMat *Q; //3D Reprojection Matrix
	CvMat *mx_LEFT; //Left Undistortion/Rectify Map for x
	CvMat *my_LEFT; //Left Undistortion/Rectify Map for y
	CvMat *mx_RIGHT; //Right Undistortion/Rectify Map for x
	CvMat *my_RIGHT; //Right Undistortion/Rectify Map for y
	static StereoCalibration *GetInstance();
	bool IsComplete();
	//void OpenDialogWindow(HWND parentWnd);
private:
	enum CalibrationType{LEFT_INTERNAL = 1, RIGHT_INTERNAL = 2, STEREO = 4, ALL_CALIBRATION = 8};
	enum RectificationType{BOUGUET_RECTIFICATION, HARTLEY_RECTIFICATION};
	struct {
		CalibrationType lastCalibrationSelected;
		RectificationType lastRectificationSelected;
		HWND destinationFrames[2];
		float squareSize; //Size in cm of chessboard square
		int nx, ny; //Number of corners on the chessboard
		int maxScale; //Maximum scaling factor during chessboard pattern recognition
	} calibrationSettings;
	StereoCalibration(const int nx, const int ny,
		const float squareSize, const int maxScale);
	void CleanMemory(CalibrationType ct = ALL_CALIBRATION);
	bool NewPair(IplImage *images[2]);
	std::vector<CvPoint2D32f> points[2]; //Here we store for each view and for both cameras chessboard's corners position
	static StereoCalibration *instance;
	//static INT_PTR CALLBACK StereoDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void StartNewCalibration(CalibrationType ct);
	void StopCurrentCalibration();
	//void UpdateLabels();
	HANDLE stopEvent;
	virtual void Run(void *param = NULL);
	int calibrationStatus;
	HWND currentDialog;
protected:
	virtual bool StopPreprocedure();
};

class ExternalCalibration {
public:
	~ExternalCalibration();
	static ExternalCalibration * GetInstance();
	//void OpenDialogWindow(HWND parentWnd);
	CvMat *Rotation_Matrix;
	CvMat *Traslation_Vector;
	void CleanMemory();
	bool SaveToFolder(const char * folderName);
	bool LoadFromFolder(const char * folderName);
	bool IsComplete();
private:
	ExternalCalibration();
	//static INT_PTR CALLBACK DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	//static INT_PTR CALLBACK DlgWorldProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void MouseCallback(int ev, int x, int y, int flags, void *param);
	static ExternalCalibration *instance;
	int FillVectors(std::vector<CvPoint2D32f> *pixels, std::vector<CvPoint3D32f> *worldCoo);
	void AddToListView(CvPoint2D32f pixel, CvPoint3D32f world);
	HWND hwndDlg, hwndImage; HANDLE hLeftImageReady;
	IplImage *currentSnapshot;
	bool loaded;
};

#endif //STEREO_CALIBRATION