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

#ifndef LEOTTA_BACKGROUND_MODEL
#define LEOTTA_BACKGROUND_MODEL

#include "CommonHeader.h"
#include "ConnectedComponents.h"

#include <LeoColorSpaces.h>
#include <LeoSettingsPersistence.h>
#include <LeoOpenCVCvArrStorage.h>

class EdgeIntensityModel {
private:
	float *vertActivity, *horzActivity;
	CvMat *runningAverage, *differential, *currEdgeIntensity;
	float &beta, &minRumour;
public:
	EdgeIntensityModel(int height, int width);
	~EdgeIntensityModel();
	void UpdateEdgeIntensityModel(const IplImage * vEdge, const IplImage * hEdge);
	float *GetVertActivityVector() {
		return vertActivity;
	}
	float *GetHorzActivityVector() {
		return horzActivity;
	}
	CvMat *GetCurrentEdgeIntensity() {
		return currEdgeIntensity;
	}
	CvMat *GetDifferential() {
		return differential;
	}
	CvMat *GetRunningAverage() {
		return runningAverage;
	}
};

class ActivityModel {
private:
	float &lambda;
	CvMat *runningAverage;
public:
	ActivityModel(int height, int width);
	~ActivityModel();
	void UpdateActivityModel(float *vertActivity, float *horzActivity);
	CvMat *GetRunningAverage() {
		return runningAverage;
	}
};

class ColorModel {
private:
	CvMat *average, *variance;
	CvMat *rawDistortion, *adjustedDistortion;
	float &alphaNom, &activityThreshold;
public:
	ColorModel(int height, int width);
	~ColorModel();
	CvMat *GetAveragePtr() {
		return average;
	}
	CvMat *GetVariancePtr() {
		return variance;
	}
	CvMat *GetRawDistortionPtr() {
		return rawDistortion;
	}
	CvMat *GetAdjustedDistortionPtr() {
		return adjustedDistortion;
	}
	void UpdateColorModel(const IplImage *bgrImage, const CvMat *activity, bool initPhase);
	void cloneColorModel(CvArrStorage &stateContainer);
	void replaceColorModel(CvArrStorage &stateContainer);
};

class ForegroundSegmentation {
private:
	IplImage *foreground;
	float &minColorDistortion;
	float &lowRGBMaxBrightnessDistortion;
	float &minDarkening, &maxDarkening, &ts, &th;
	bool &useShadowSuppressor;
public:
	ForegroundSegmentation(int height, int width);
	~ForegroundSegmentation();
	IplImage *GetCurrentForegroundPtr() {
		return foreground;
	}
	void UpdateForeground(ColorModel *cm, const IplImage *bgrImage);
};

class BackgroundModeling {
private:
	EdgeIntensityModel *edgeIntensityModel;
	ActivityModel *activityModel;
	ColorModel *colorModel;
	ForegroundSegmentation *foreSegmentation;
	FindConnectedComponents *fcc;
	int height, width;
public:
	BackgroundModeling(int height, int width);
	~BackgroundModeling();
	ActivityModel *GetActivityModelPtr() {
		return activityModel;
	}
	EdgeIntensityModel *GetEdgeIntensityModelPtr() {
		return edgeIntensityModel;
	}
	ColorModel *GetColorModelPtr() {
		return colorModel;
	}
	ForegroundSegmentation *GetForegroundSegmentationPtr() {
		return foreSegmentation;
	}
	FindConnectedComponents *GetFindConnectedComponents() {
		return fcc;
	}
	CvSeq *UpdateForeground(const IplImage *bgrImage, const IplImage *vEdge, const IplImage *hEdge, bool initPhase);
};

#endif //LEOTTA_BACKGROUND_MODEL