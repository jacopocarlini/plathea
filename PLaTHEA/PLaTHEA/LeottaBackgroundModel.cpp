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

#include "LeottaBackgroundModel.h"
#include "Shared.h"

using namespace leostorm::settingspersistence;
using namespace leostorm::colorspaces;

/**** EdgeIntensityModel Implementation ****/

class EIMParametersSetter: public ParameterSetter {
public:
	float beta;
	float minRumour;
	EIMParametersSetter();
	void ModifyParameter(char *var, int delta); //For perimScale
};

void EIMParametersSetter::ModifyParameter(char *var, int delta) {
	if (strcmp(var, "beta")==0) {
		beta+=(delta*0.02f);
		this->beta = ApplyThreshold(beta, LEO_APPLYTHRESHOLD_MIN | LEO_APPLYTHRESHOLD_MAX, 0.f, 1.f);
	} else if (strcmp(var, "minRumour")==0) {
		minRumour+=delta;
		this->minRumour = ApplyThreshold(minRumour, LEO_APPLYTHRESHOLD_MIN | LEO_APPLYTHRESHOLD_MAX, 0.f, 100.f);
	}
}

EIMParametersSetter::EIMParametersSetter(): ParameterSetter("edgeIntensityModel") {
	AddParameter("beta", &beta);
	beta = 0.08f;
	AddParameter("minRumour", &minRumour);
	minRumour = 35.f;
}

static EIMParametersSetter eimParameters;

EdgeIntensityModel::EdgeIntensityModel(int height, int width): minRumour(eimParameters.minRumour), beta(eimParameters.beta) {
	this->vertActivity = new float[width];
	this->horzActivity = new float[height];
	this->differential = cvCreateMat(height, width, CV_32FC1);
	cvZero(differential);
	this->currEdgeIntensity = cvCreateMat(height, width, CV_32FC1);
	cvZero(currEdgeIntensity);
	this->runningAverage = cvCreateMat(height, width, CV_32FC1);
	cvZero(runningAverage);
}

EdgeIntensityModel::~EdgeIntensityModel() {
	delete[] vertActivity;
	delete[] horzActivity;
	cvReleaseMat(&differential);
	cvReleaseMat(&currEdgeIntensity);
	cvReleaseMat(&runningAverage);
}

void EdgeIntensityModel::UpdateEdgeIntensityModel(const IplImage *vEdge, const IplImage *hEdge) {
	int height = runningAverage->height, width = runningAverage->width;
	memset(vertActivity, 0, width*sizeof(float));
	memset(horzActivity, 0, height*sizeof(float));
	for (int row = 0; row<height; row++) {
		const short *vEdgePtr = (const short *) (vEdge->imageData + row*vEdge->widthStep);
		const short *hEdgePtr = (const short *) (hEdge->imageData + row*hEdge->widthStep);
		float *avgPtr = (float *) (runningAverage->data.ptr + row*runningAverage->step);
		float *difPtr = (float *) (differential->data.ptr + row*differential->step);
		float *currEdgePtr = (float *) (currEdgeIntensity->data.ptr + row*currEdgeIntensity->step);
		for (int col = 0; col<width; col++, vEdgePtr++, hEdgePtr++, avgPtr++, difPtr++, currEdgePtr++) {
			*currEdgePtr = sqrt(float(((*vEdgePtr)*(*vEdgePtr)) + ((*hEdgePtr)*(*hEdgePtr))));
			*avgPtr = (1-beta)*(*avgPtr) + beta*(*currEdgePtr);
			*difPtr = abs((*currEdgePtr) - (*avgPtr));
			if (*difPtr <= minRumour)
				*difPtr = 0.f;
			horzActivity[row] += *difPtr;
			vertActivity[col] += *difPtr;
		}
	}
}

/**** ActivityModel Implementation ****/

class AMParametersSetter: public ParameterSetter {
public:
	float lambda;
	AMParametersSetter();
	void ModifyParameter(char *var, int delta); //For perimScale
};

AMParametersSetter::AMParametersSetter(): ParameterSetter("activityModel") {
	AddParameter("lambda", &lambda);
	lambda = 0.2f;
}

void AMParametersSetter::ModifyParameter(char *var, int delta) {
	if (strcmp(var, "lambda") == 0) {
		lambda+=(delta*0.02f);
		this->lambda = ApplyThreshold(lambda, LEO_APPLYTHRESHOLD_MIN | LEO_APPLYTHRESHOLD_MAX, 0.f, 1.f);
	}
}

static AMParametersSetter amParameters;

ActivityModel::ActivityModel(int height, int width): lambda(amParameters.lambda) {
	runningAverage = cvCreateMat(height, width, CV_32FC1);
	cvZero(runningAverage);
}

ActivityModel::~ActivityModel() {
	cvReleaseMat(&runningAverage);
}

void ActivityModel::UpdateActivityModel(float *vertActivity, float *horzActivity) {
	int height = runningAverage->height, width = runningAverage->width;
	for (int row = 0; row<height; row++) {
		float *avgPtr = (float *) (runningAverage->data.ptr + row*runningAverage->step);
		for (int col = 0; col<width; col++, avgPtr++) {
			*avgPtr = (1-lambda)*(*avgPtr) + lambda*horzActivity[row]*vertActivity[col];
		}
	}
}

/**** ColorModel Implementation ****/

class CMParametersSetter: public ParameterSetter {
public:
	float alphaNom, activityThreshold;
	CMParametersSetter();
	void ModifyParameter(char *var, int delta); //For perimScale
};

CMParametersSetter::CMParametersSetter(): ParameterSetter("colorModel") {
	AddParameter("alphaNom", &alphaNom);
	alphaNom = 0.1f;
	AddParameter("activityThreshold", &activityThreshold);
	activityThreshold = 850.f;
}

void CMParametersSetter::ModifyParameter(char *var, int delta) {
	if (strcmp(var, "alphaNom") == 0) {
		alphaNom+=(delta*0.02f);
		this->alphaNom = ApplyThreshold(alphaNom, LEO_APPLYTHRESHOLD_MIN | LEO_APPLYTHRESHOLD_MAX, 0.f, 1.f);
	} else if (strcmp(var, "activityThreshold") == 0) {
		activityThreshold+=delta;
		this->activityThreshold = ApplyThreshold(activityThreshold, LEO_APPLYTHRESHOLD_MIN, 1.f, 0.f);
	}
}

static CMParametersSetter cmParameters;

ColorModel::ColorModel(int height, int width): alphaNom(cmParameters.alphaNom), activityThreshold(cmParameters.activityThreshold) {
	this->average = cvCreateMat(height, width, CV_32FC3);
	cvZero(average);
	this->variance = cvCreateMat(height, width, CV_32FC3);
	cvZero(variance);
	this->rawDistortion = cvCreateMat(height, width, CV_32FC2);
	cvZero(rawDistortion);
	this->adjustedDistortion = cvCreateMat(height, width, CV_32FC2);
	cvZero(adjustedDistortion);
}

ColorModel::~ColorModel() {
	cvReleaseMat(&average);
	cvReleaseMat(&variance);
	cvReleaseMat(&rawDistortion);
	cvReleaseMat(&adjustedDistortion);
}

void ColorModel::UpdateColorModel(const IplImage *bgrImage, const CvMat *activity, bool initPhase) {
	int height = bgrImage->height;
	int width = bgrImage->width;
	for (int row = 0; row < height; row++) {
		const uchar *bgrPtr = (const uchar *) (bgrImage->imageData + row*bgrImage->widthStep);
		const float *activityPtr = (const float *) (activity->data.ptr + row*activity->step);
		float *avgPtr = (float *) (average->data.ptr + row*average->step);
		float *varPtr = (float *) (variance->data.ptr + row*variance->step);
		float *rawPtr = (float *) (rawDistortion->data.ptr + row*rawDistortion->step);
		float *adjPtr = (float *) (adjustedDistortion->data.ptr + row*adjustedDistortion->step);
		for (int col = 0; col < width; col++, bgrPtr+=3, activityPtr++, avgPtr+=3, varPtr+=3, rawPtr+=2, adjPtr+=2) {
			float alphaNom_mod = initPhase ? alphaNom*2 : alphaNom;
			float alpha = alphaNom_mod*(1 - (*activityPtr)/activityThreshold);
			if (alpha > 0.f) {
				for (int i = 0; i < 3; i++) {
					avgPtr[i] = (1 - alpha)*(avgPtr[i]) + alpha*(bgrPtr[i]);
					float absDif = abs(float(bgrPtr[i]-(avgPtr[i])));
					varPtr[i] = (1 - alpha)*(varPtr[i]) + alpha*(absDif)*(absDif);
				}
			}

			if (varPtr[0] > FLT_EPSILON && varPtr[1] > FLT_EPSILON && varPtr[2] > FLT_EPSILON) {
					float alpha_num[] = {(bgrPtr[0]*avgPtr[0])/varPtr[0], (bgrPtr[1]*avgPtr[1])/varPtr[1],
						(bgrPtr[2]*avgPtr[2])/varPtr[2]};
					float alpha_den[] = {(avgPtr[0]*avgPtr[0])/varPtr[0], (avgPtr[1]*avgPtr[1])/varPtr[1],
						(avgPtr[2]*avgPtr[2])/varPtr[2]};
					rawPtr[0] = (alpha_num[0] + alpha_num[1] + alpha_num[2])/(alpha_den[0] + alpha_den[1] + alpha_den[2]);
					if (alpha>0.f) {
						float absDif = (1 - rawPtr[0]);
						adjPtr[0] = (1 - alpha)*(adjPtr[0]) + alpha*(absDif)*(absDif);
					}

					float CD_num[] = {bgrPtr[0] - (rawPtr[0]*avgPtr[0]), bgrPtr[1] - (rawPtr[0]*avgPtr[1]),
						bgrPtr[2] - (rawPtr[0]*avgPtr[2])};
					CD_num[0]*=CD_num[0]; CD_num[1]*=CD_num[1]; CD_num[2]*=CD_num[2];
					rawPtr[1] = cvSqrt((CD_num[0]/varPtr[0]) + (CD_num[1]/varPtr[1]) + (CD_num[2]/varPtr[2]));
					if (alpha>0.f)
						adjPtr[1] = (1 - alpha)*(adjPtr[1]) + alpha*(rawPtr[1])*(rawPtr[1]);
			}
		}
	}
}

void ColorModel::cloneColorModel(CvArrStorage &stateContainer) {
	stateContainer["colorModelAverage"] = cvClone(average);
	stateContainer["colorModelVariance"] = cvClone(variance);
	stateContainer["colorModelAdjustedDistortion"] = cvClone(adjustedDistortion);
}

void ColorModel::replaceColorModel(CvArrStorage &stateContainer) {
	cvCopy(stateContainer["colorModelAverage"], average);
	cvCopy(stateContainer["colorModelVariance"], variance);
	cvCopy(stateContainer["colorModelAdjustedDistortion"], adjustedDistortion);
}

/**** ForegroundSegmentation Implementation ****/

class FSParametersSetter: public ParameterSetter {
public:
	float minColorDistortion, lowRGBMaxBrightnessDistortion, minDarkening, maxDarkening, th, ts;
	bool useShadowSuppressor;
	FSParametersSetter();
	void ModifyParameter(char *var, int delta); //For perimScale
};

FSParametersSetter::FSParametersSetter(): ParameterSetter("foregroundSegmentation") {
	AddParameter("minColorDistortion", &minColorDistortion);
	minColorDistortion = 4.f;
	AddParameter("lowRGBMaxBrightnessDistortion", &lowRGBMaxBrightnessDistortion);
	lowRGBMaxBrightnessDistortion = -1000.f;
	AddParameter("minDarkening", &minDarkening);
	minDarkening = 0.8f;
	AddParameter("maxDarkening", &maxDarkening);
	maxDarkening = 1.f;
	AddParameter("th", &th);
	th = 160.f;
	AddParameter("ts", &ts);
	ts = 55.f;
	AddParameter("useShadowSuppressor", &useShadowSuppressor);
	useShadowSuppressor = true;
}

void FSParametersSetter::ModifyParameter(char *var, int delta) {
	if (strcmp(var, "minColorDistortion") == 0) {
		minColorDistortion+=(delta*0.02f);
		this->minColorDistortion = ApplyThreshold(minColorDistortion, LEO_APPLYTHRESHOLD_MIN, 0.f, 0.f);
	} else if (strcmp(var, "lowRGBMaxBrightnessDistortion") == 0) {
		lowRGBMaxBrightnessDistortion+=delta;
		this->lowRGBMaxBrightnessDistortion = ApplyThreshold(lowRGBMaxBrightnessDistortion, LEO_APPLYTHRESHOLD_MAX, 0.f, 0.f);
	} else if (strcmp(var, "minDarkening") == 0) {
		minDarkening+=(delta*0.01f);
		this->minDarkening = ApplyThreshold(minDarkening, LEO_APPLYTHRESHOLD_MIN | LEO_APPLYTHRESHOLD_MAX, 0.f, 1.f);
	} else if (strcmp(var, "maxDarkening") == 0) {
		maxDarkening+=(delta*0.01f);
		this->maxDarkening = ApplyThreshold(maxDarkening, LEO_APPLYTHRESHOLD_MIN | LEO_APPLYTHRESHOLD_MAX, 0.f, 1.f);
	} else if (strcmp(var, "th") == 0) {
		th+=delta;
		this->th = ApplyThreshold(th, LEO_APPLYTHRESHOLD_MIN | LEO_APPLYTHRESHOLD_MAX, 0.f, 360.f);
	} else if (strcmp(var, "ts") == 0) {
		ts+=delta;
		this->ts = ApplyThreshold(ts, LEO_APPLYTHRESHOLD_MIN | LEO_APPLYTHRESHOLD_MAX, 0.f, 255.f);
	}
}

static FSParametersSetter fsParameters;

ForegroundSegmentation::ForegroundSegmentation(int height, int width): minColorDistortion(fsParameters.minColorDistortion),
	lowRGBMaxBrightnessDistortion(fsParameters.lowRGBMaxBrightnessDistortion), minDarkening(fsParameters.minDarkening),
	maxDarkening(fsParameters.maxDarkening), th(fsParameters.th), ts(fsParameters.ts), useShadowSuppressor(fsParameters.useShadowSuppressor) {

	foreground = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	cvZero(foreground);
}

ForegroundSegmentation::~ForegroundSegmentation() {
	cvReleaseImage(&foreground);
}

void ForegroundSegmentation::UpdateForeground(ColorModel *cm, const IplImage *bgrImage) {
	CvMat *rawDistortion = cm->GetRawDistortionPtr();
	CvMat *adjDistortion = cm->GetAdjustedDistortionPtr();
	CvMat *avgImage = cm->GetAveragePtr();
	bool uss = useShadowSuppressor; //We store this value for multithreading
	int height = rawDistortion->height, width = rawDistortion->width;
	if (uss) {
		for (int row = 0; row < height; row++) {
			uchar *forPtr = (uchar *) (foreground->imageData + row*foreground->widthStep);
			const float *rawPtr = (const float *) (rawDistortion->data.ptr + row*rawDistortion->step);
			const float *adjPtr = (const float *) (adjDistortion->data.ptr + row*adjDistortion->step);
			const uchar *bgrPtr = (const uchar *) (bgrImage->imageData + row*bgrImage->widthStep);
			const float *avgPtr = (const float *) (avgImage->data.ptr + row*avgImage->step);
			for (int col = 0; col < width; col++, forPtr++, rawPtr+=2, adjPtr+=2, bgrPtr+=3, avgPtr+=3) {
				float raw_brightness_mod = (rawPtr[0] - 1)/adjPtr[0];
				float raw_color_mod = rawPtr[1]/adjPtr[1];
				/* TODO: At the actual stage the first algorithm classify as temporary foreground a pixel whose color distortion is over a **
				** specific threshold (minColorDistortion) or some other consideration. We can apply the equation (11) of Horpraset and    **
				** then apply the paper by cucchiara to verify that it is a shadow                                                         */
				/*if (raw_color_mod > minColorDistortion)
					*forPtr = 255;
				else if (raw_brightness_mod < lowRGBMaxBrightnessDistortion && raw_brightness_mod > -100000.f) {
					float hsvPtr[3], hsvAvg[3];
					BGR2HSV(bgrPtr, hsvPtr, UCHARSRC_FLOATDST);
					BGR2HSV(avgPtr, hsvAvg, FLOATSRC_FLOATDST);
					float darkFactor = hsvPtr[2]/hsvAvg[2];
					float satDiff = abs(hsvPtr[1] - hsvAvg[1]);
					float hueDiff = abs(hsvPtr[0] - hsvAvg[0]);
					hueDiff = MIN(hueDiff, 360 - hueDiff);
					if (darkFactor > minDarkening && darkFactor < maxDarkening && satDiff < ts && hueDiff < th)
						*forPtr = 0;
					else
						*forPtr = 255;
				} else
					*forPtr = 0;*/
				float hsvPtr[3], hsvAvg[3];
				BGR2HSV(bgrPtr, hsvPtr, UCHARSRC_FLOATDST);
				BGR2HSV(avgPtr, hsvAvg, FLOATSRC_FLOATDST);
				float darkFactor = hsvPtr[2]/hsvAvg[2];
				float satDiff = abs(hsvPtr[1] - hsvAvg[1]);
				float hueDiff = abs(hsvPtr[0] - hsvAvg[0]);
				hueDiff = MIN(hueDiff, 360 - hueDiff);
				if (darkFactor > minDarkening && darkFactor < maxDarkening && satDiff < ts && hueDiff < th)
					*forPtr = 0;
				else if (raw_color_mod > minColorDistortion) {
					*forPtr = 255;
				} else {
					*forPtr = 0;
				}
			}
		}
	} else {
		for (int row = 0; row < height; row++) {
			uchar *forPtr = (uchar *) (foreground->imageData + row*foreground->widthStep);
			const float *rawPtr = (const float *) (rawDistortion->data.ptr + row*rawDistortion->step);
			const float *adjPtr = (const float *) (adjDistortion->data.ptr + row*adjDistortion->step);
			for (int col = 0; col < width; col++, forPtr++, rawPtr+=2, adjPtr+=2) {
				float raw_brightness_mod = (rawPtr[0] - 1)/adjPtr[0];
				float raw_color_mod = rawPtr[1]/adjPtr[1];
				if (raw_color_mod > minColorDistortion || (raw_brightness_mod < lowRGBMaxBrightnessDistortion && raw_brightness_mod > -100000.f))	
					*forPtr = 255;
				else
					*forPtr = 0;
			}
		}
	}
}

/**** BackgroundModel Implementation ****/

BackgroundModeling::BackgroundModeling(int height, int width) {
	this->height = height;
	this->width = width;
	this->edgeIntensityModel = new EdgeIntensityModel(height, width);
	this->activityModel = new ActivityModel(height, width);
	this->colorModel = new ColorModel(height, width);
	this->foreSegmentation = new ForegroundSegmentation(height, width);
	this->fcc = new FindConnectedComponents(height, width);
}

BackgroundModeling::~BackgroundModeling() {
	delete edgeIntensityModel;
	delete activityModel;
	delete colorModel;
	delete foreSegmentation;
	delete fcc;
}

CvSeq *BackgroundModeling::UpdateForeground(const IplImage *bgrImage, const IplImage *vEdge, const IplImage *hEdge,
											bool initPhase) {
	edgeIntensityModel->UpdateEdgeIntensityModel(vEdge, hEdge);
	activityModel->UpdateActivityModel(edgeIntensityModel->GetVertActivityVector(),
		edgeIntensityModel->GetHorzActivityVector());
	colorModel->UpdateColorModel(bgrImage, activityModel->GetRunningAverage(), initPhase);
	foreSegmentation->UpdateForeground(colorModel, bgrImage);
	return fcc->UpdateConnectedComponents(foreSegmentation->GetCurrentForegroundPtr());
}