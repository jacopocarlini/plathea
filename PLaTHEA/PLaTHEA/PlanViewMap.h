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

#ifndef PLAN_VIEW_MAP
#define PLAN_VIEW_MAP

#include "CommonHeader.h"
#include "munkres.h"
#include "StereoCalibration.h"
#include "RoomSettings.h"

#include <LeoSettingsPersistence.h>

struct BGRTriple {
	uchar b, g, r;
};

struct ObjectTemplate {
	CvMat *occupancyTemplate, *heightTemplate;
	int *colorTemplate, colorTemplateSum;
	
	CvMat* currentMeasurement;
	int xSideBox, ySideBox;

	ObjectTemplate(int templateXLength, int templateYLength) {
		xSideBox = templateXLength;
		ySideBox = templateYLength;
		occupancyTemplate = cvCreateMat(ySideBox, xSideBox, CV_32FC1);
		heightTemplate = cvCreateMat(ySideBox, xSideBox, CV_32FC1);
		colorTemplate = new int[4096];
		cvZero(occupancyTemplate);
		cvZero(heightTemplate);
		ZeroMemory(colorTemplate, 4096*sizeof(int));
		colorTemplateSum = 0;
		currentMeasurement = cvCreateMat(3, 1, CV_32FC1);
	}
	~ObjectTemplate() {
		if (occupancyTemplate) {
			cvReleaseMat(&occupancyTemplate);
		}
		if (heightTemplate) {
			cvReleaseMat(&heightTemplate);
		}
		if (colorTemplate) {
			delete[] colorTemplate;
		}
		if (currentMeasurement)
			cvReleaseMat(&currentMeasurement);
	}
};

enum TrackedObjectType{NEW_OBJECT, TRACKED, LOST, STALE};

struct TrackedObject {
	static int staticID;
	int ID;
	bool justIdentified;
	char name[256];
	int nameID;
	int suspectID, numberOfProbation;
	DWORD creationTimestamp;
	DWORD noUpdateDelay;

	CvKalman *kalmanFilter, *movementKalmanFilter;
	CvMat *occupancyTemplate, *heightTemplate;
	int *colorTemplate, colorTemplateSum;
	CvMat *lastMeasurement;
	
	int consecutiveEvents;
	CvPoint upperLeft, bottomRight;
	TrackedObjectType type;
	CvScalar color;

	void NoCorrespondence(DWORD dt) {
		if (type == TRACKED) {
			consecutiveEvents = 1;
			type = LOST;
		} else if (type == LOST) {
			consecutiveEvents++;
			if (consecutiveEvents>=20)
				type = STALE;
		} else if (type == NEW_OBJECT) {
			consecutiveEvents = 1;
			type = STALE;
		}
		noUpdateDelay+=dt;
		//Enable the following lines to follow Kalman filter even without evidence
		cvCopy(movementKalmanFilter->state_pre, movementKalmanFilter->state_post);
		cvCopy(movementKalmanFilter->error_cov_pre, movementKalmanFilter->error_cov_post);
		cvKalmanCorrect(kalmanFilter, lastMeasurement);
		UpdateRepresentation(NULL, false);
	}
	void UpdateRepresentation(ObjectTemplate *ot, bool pre = false) {
		if (ot) {
			upperLeft.x = int(kalmanFilter->state_post->data.fl[0] - (ot->xSideBox / 2.f));
			upperLeft.y = int(kalmanFilter->state_post->data.fl[1] - (ot->ySideBox / 2.f));
			bottomRight.x = upperLeft.x + ot->xSideBox;  bottomRight.y = upperLeft.y + ot->ySideBox;
		} else {
			int xSideBox = bottomRight.x - upperLeft.x; int ySideBox = bottomRight.y - upperLeft.y;
			if (pre) {
				upperLeft.x = int(kalmanFilter->state_pre->data.fl[0] - (xSideBox / 2.f));
				upperLeft.y = int(kalmanFilter->state_pre->data.fl[1] - (ySideBox / 2.f));
			} else {
				upperLeft.x = int(kalmanFilter->state_post->data.fl[0] - (xSideBox / 2.f));
				upperLeft.y = int(kalmanFilter->state_post->data.fl[1] - (ySideBox / 2.f));
			}
			bottomRight.x = upperLeft.x + xSideBox;  bottomRight.y = upperLeft.y + ySideBox;
		}
	}
	void SetTemplate(ObjectTemplate *ot, int associationResult) {
		Clear();
		
		noUpdateDelay = 0;

		occupancyTemplate = cvCloneMat(ot->occupancyTemplate);
		heightTemplate = cvCloneMat(ot->heightTemplate);
		colorTemplateSum = 0;
		int *myColorTemplatePtr = colorTemplate, *otColorTemplatePtr = ot->colorTemplate;
		for (int i = 0; i < 4096; i++, myColorTemplatePtr++, otColorTemplatePtr++) {
			*myColorTemplatePtr = int(0.5*(*myColorTemplatePtr)+0.5*(*otColorTemplatePtr));
			colorTemplateSum += (*myColorTemplatePtr);
		}

		//if (associationResult == 1) {
			cvKalmanCorrect(kalmanFilter, ot->currentMeasurement);
			//*movementKalmanFilter = *kalmanFilter;
		//} else if (associationResult == -1) {
			cvKalmanCorrect(movementKalmanFilter, ot->currentMeasurement);
			//*kalmanFilter = *movementKalmanFilter;
		//}
		cvCopy(ot->currentMeasurement, lastMeasurement);

		UpdateRepresentation(ot);

		/*writeToConsole(L"Measure: %f, %f, %f Kalman State: %f, %f, %f, %f, %f\r\n",
			ot->currentMeasurement->data.fl[0], ot->currentMeasurement->data.fl[1], ot->currentMeasurement->data.fl[2],
			kalmanFilter->state_post->data.fl[0], kalmanFilter->state_post->data.fl[1], kalmanFilter->state_post->data.fl[2],
			kalmanFilter->state_post->data.fl[3], kalmanFilter->state_post->data.fl[4]);*/
		
		if (type == NEW_OBJECT) {
			consecutiveEvents++;
			if (consecutiveEvents>=5)
				type = TRACKED;
		} else if (type == LOST) {
			type = TRACKED;
			consecutiveEvents = 1;
		}
	}
	TrackedObject(ObjectTemplate *ot);
	~TrackedObject();
private:
	void prepareTrackingFilter(float startXPosition, float startYPosition, float startHeight);
	void Clear();
};

struct TrackedFaceCorrespondence;

class PlanViewMapController {
private:
	HANDLE hUnblockEvent;
	virtual void CurrentFrameUpdate() = 0;
public:
	PlanViewMapController() {
		hUnblockEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	}
	~PlanViewMapController() {
		CloseHandle(hUnblockEvent);
	}
	void WaitForController() {
		CurrentFrameUpdate();
		WaitForSingleObject(hUnblockEvent, INFINITE);
	}
	void WaitForController(std::vector<ObjectTemplate *>* candidateObjects, std::vector<TrackedObject *>* trackedObjects, CvMat* componentValues) {
		currentAssociation.clear();
		currentAssociation.resize(trackedObjects->size(), -1);
		this->candidateObjects = candidateObjects;
		this->trackedObjects = trackedObjects;
		this->componentValues = componentValues;
		WaitForController();
	}
	void UnBlockPlanViewMap() {
		SetEvent(hUnblockEvent);
	}
	virtual void AccuracyCallback(int correct, int total, int appliedCorrect, int appliedTotal) = 0;
	std::vector<int> currentAssociation;
	std::vector<ObjectTemplate *>* candidateObjects;
	std::vector<TrackedObject *>* trackedObjects;
	CvMat* componentValues;
	bool accuracyController;
};

class PlanViewMap {
private:
	RoomSettingsStruct *rss;

	int ytexels /*Y direction number of texels*/, xtexels /*X direction number of texels*/;

	CvMat *occupancyImage, *heightImage, *tempPvm;
	CvFont font;
	int bytes_occupation; //Number of bytes in the float vectors
	int totTexels;

	/*START OPTICAL FLOW*/
	IplImage *AOFImage, *BOFImage, *eigOFTemp, *tempOFTemp, *AOFPyr, *BOFPyr;
	CvPoint2D32f *AOFCorners, *BOFCorners;
	bool currentlyAOF;
	int numberOfCornersOF;
	/*END OPTICAL FLOW*/

	//int personBoxSide, filterSize; //Texel side of averaged width person template containment box
	
	float z_cam_normalization;
	float &occ_threshold; //New object detection threshold

	//Variable settings
	float &chi; //Height map pruning value

	//Working Variables
	float *workOccupancy, *workHeight;
	CvMat workOccupancyImage, workHeightImage;
	std::vector<ObjectTemplate *> candidateObject;
	
	int *foregroundPixelsForTexels; BGRTriple *colorsForTexels;

	//Contour scanner utility
	CvMemStorage *memStorage;
	CvMat *contourScratch;
	CvSeq *currentStereoContours;

	PlanViewMapController *pvmc;

	cv::Ptr<cv::ml::SVM> currentSVM;

	static char svmClassifierFileName[MAX_PATH];
public:
	PlanViewMap();
	~PlanViewMap();
	std::vector<TrackedObject *> trackedPersons;
	void UpdatePlanViewMap(CvMat *projection3D, IplImage *foreground, CvMat *disparity, IplImage *leftBGR, CvPoint *imagePixelMapping);
	void IdentifyVisibleSubjects();
	void Track_em(std::vector<TrackedFaceCorrespondence> *tfc, DWORD dt);
	void InterpolateIdentity(std::vector<TrackedFaceCorrespondence> &tfc);
	void Reset();
	bool RegisterPlanViewMapController(PlanViewMapController *pvmc) {
		this->pvmc = pvmc;
		return true;
	}
	PlanViewMapController * GetPlanViewMapController() {
		return pvmc;
	}
	CvMat *GetOccupancyImage() {
		return occupancyImage;
	}
	CvMat *GetHeightImage() {
		return heightImage;
	}
	int GetXTexelsNumber() {
		return xtexels;
	}
	int GetYTexelsNumber() {
		return ytexels;
	}
	static char *GetSVMClassifierFileName();
	static void SetSVMClassifierFileName(char *fileName);
};

#endif //PLAN_VIEW_MAP