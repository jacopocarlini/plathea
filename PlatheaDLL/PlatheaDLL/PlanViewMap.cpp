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

#include "PlanViewMap.h"
#include "ElaborationCore.h"

#include <LeoLog4CPP.h>

int TrackedObject::staticID = 0;

char PlanViewMap::svmClassifierFileName[MAX_PATH];

extern SystemInfo *si;

using namespace leostorm::logging;
using namespace leostorm::settingspersistence;

extern bool svmTracker;

TrackedObject::TrackedObject(ObjectTemplate *ot) {
	ID = staticID++;
	strcpy_s(name, 256, "");
	nameID = suspectID = -1;
	occupancyTemplate = cvCloneMat(ot->occupancyTemplate);
	heightTemplate = cvCloneMat(ot->heightTemplate);
	lastMeasurement = cvCloneMat(ot->currentMeasurement);
	colorTemplate = new int[4096];
	memcpy(colorTemplate, ot->colorTemplate, 4096*sizeof(int));
	colorTemplateSum = ot->colorTemplateSum;
	type = NEW_OBJECT;
	color = cvScalar(rand()%256, rand()%256, rand()%256);
	consecutiveEvents=1;
	justIdentified = false;
	numberOfProbation=0;

	creationTimestamp = GetTickCount();
	noUpdateDelay = 0;

	prepareTrackingFilter(ot->currentMeasurement->data.fl[0], ot->currentMeasurement->data.fl[1],
		ot->currentMeasurement->data.fl[2]);

	UpdateRepresentation(ot);
}

TrackedObject::~TrackedObject() {
	Clear();
	delete[] colorTemplate;
	cvReleaseKalman(&kalmanFilter);
	cvReleaseKalman(&movementKalmanFilter);
	cvReleaseMat(&lastMeasurement);
}

void TrackedObject::Clear() {
	cvReleaseMat(&occupancyTemplate);
	cvReleaseMat(&heightTemplate);
}

void TrackedObject::prepareTrackingFilter(float startXPosition, float startYPosition, float startHeight) {
	kalmanFilter = cvCreateKalman(5, 3, 0);
	const float F[] = {1.f, 0.f, 0.1f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.1f, 0.f,
		0.f, 0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 0.f, 1.f};
	memcpy(kalmanFilter->transition_matrix->data.fl, F, sizeof(F));
	const float H[] = {1.f, 0.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 0.f, 0.f, 1.f};
	memcpy(kalmanFilter->measurement_matrix->data.fl, H, sizeof(H));
	cvSetIdentity(kalmanFilter->error_cov_post, cvRealScalar(1));
	cvSetIdentity(kalmanFilter->process_noise_cov, cvRealScalar(2));
	cvSetIdentity(kalmanFilter->measurement_noise_cov, cvRealScalar(5));
	kalmanFilter->state_post->data.fl[0] = startXPosition;
	kalmanFilter->state_post->data.fl[1] = startYPosition;
	kalmanFilter->state_post->data.fl[2] = kalmanFilter->state_post->data.fl[3] = 0.f;
	kalmanFilter->state_post->data.fl[4] = startHeight;

	//The second kalman filter is used to cope with the possibility that when measurements are lacking the subject
	//is moving following the previously acquired velocity
	movementKalmanFilter = cvCreateKalman(5, 3, 0);
	memcpy(movementKalmanFilter->transition_matrix->data.fl, F, sizeof(F));
	memcpy(movementKalmanFilter->measurement_matrix->data.fl, H, sizeof(H));
	cvSetIdentity(movementKalmanFilter->error_cov_post, cvRealScalar(1));
	cvSetIdentity(movementKalmanFilter->process_noise_cov, cvRealScalar(2));
	cvSetIdentity(movementKalmanFilter->measurement_noise_cov, cvRealScalar(5));
	movementKalmanFilter->state_post->data.fl[0] = startXPosition;
	movementKalmanFilter->state_post->data.fl[1] = startYPosition;
	movementKalmanFilter->state_post->data.fl[2] = movementKalmanFilter->state_post->data.fl[3] = 0.f;
	movementKalmanFilter->state_post->data.fl[4] = startHeight;
}

class PVMParametersSetter: public ParameterSetter {
public:
	float chi, occ_threshold;
	PVMParametersSetter();
	void ModifyParameter(char *var, int delta); //For perimScale
};

PVMParametersSetter::PVMParametersSetter(): ParameterSetter("planViewMap") {
	RoomSettingsStruct* rss = &RoomSettings::GetInstance()->data;

	AddParameter("chi", &chi);
	chi = 0.01f;
	AddParameter("person_occupation_threshold", &occ_threshold);
	occ_threshold = 0.05f;
}

void PVMParametersSetter::ModifyParameter(char *var, int delta) {
	RoomSettingsStruct* rss = &RoomSettings::GetInstance()->data;

	if (strcmp(var, "chi")==0) {
		chi+=(delta*0.01f);
		chi = ApplyThreshold(chi, LEO_APPLYTHRESHOLD_MIN | LEO_APPLYTHRESHOLD_MAX, 0.f, 1.f);
	} else if (strcmp(var, "person_occupation_threshold")==0) {
		occ_threshold+=(delta*0.05f);
		occ_threshold = ApplyThreshold(occ_threshold, LEO_APPLYTHRESHOLD_MIN, 0.f, 0.f);
	}
}

static PVMParametersSetter pvmParameters;

PlanViewMap::PlanViewMap(): chi(pvmParameters.chi), occ_threshold(pvmParameters.occ_threshold) {
	
	rss = &RoomSettings::GetInstance()->data;
	
	ytexels = int(ceil((rss->WYmax - rss->WYmin)/rss->texelSide)) +1;
	xtexels = int(ceil((rss->WXmax - rss->WXmin)/rss->texelSide)) +1;

	totTexels = ytexels*xtexels;
	occupancyImage = cvCreateMat(ytexels, xtexels, CV_8UC3);
	heightImage = cvCreateMat(ytexels, xtexels, CV_8UC3);
	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1.0, 1.0, 0.0, 2);
	tempPvm = cvCreateMat(ytexels, xtexels, CV_8UC1);
	bytes_occupation = sizeof(float) * totTexels;

	/*OPTICAL FLOW INITIALIZATION*/
	AOFImage = cvCreateImage(cvSize(xtexels, ytexels), IPL_DEPTH_8U, 1);
	BOFImage = cvCreateImage(cvSize(xtexels, ytexels), IPL_DEPTH_8U, 1);
	eigOFTemp = cvCreateImage(cvSize(xtexels, ytexels), IPL_DEPTH_32F, 1);
	tempOFTemp = cvCreateImage(cvSize(xtexels, ytexels), IPL_DEPTH_32F, 1);
	CvSize pyr_size = cvSize(xtexels + 8, ytexels/3);
	AOFPyr = cvCreateImage(pyr_size, IPL_DEPTH_32F, 1);
	BOFPyr = cvCreateImage(pyr_size, IPL_DEPTH_32F, 1);
	numberOfCornersOF = 500;
	AOFCorners = new CvPoint2D32f[numberOfCornersOF];
	BOFCorners = new CvPoint2D32f[numberOfCornersOF];
	currentlyAOF = true;
	/*END OPTICAL FLOW INITIALIZATION*/

	workOccupancy = new float[totTexels];
	workOccupancyImage = cvMat(ytexels, xtexels, CV_32FC1, workOccupancy);
	workHeight = new float[totTexels];
	workHeightImage = cvMat(ytexels, xtexels, CV_32FC1, workHeight);

	foregroundPixelsForTexels = new int[totTexels];
	colorsForTexels = new BGRTriple[totTexels*100];

	memStorage = cvCreateMemStorage();
	contourScratch = cvCreateMat(ytexels, xtexels, CV_8UC1);
	
	CvMat *leftIntrinsics = StereoCalibration::GetInstance()->M_LEFT;
	z_cam_normalization = float(cvGetReal2D(leftIntrinsics, 0, 0)*cvGetReal2D(leftIntrinsics, 1, 1));
	z_cam_normalization = 1/z_cam_normalization;

	Reset();

	pvmc = NULL;

	WIN32_FIND_DATAA wfd; HANDLE hFile = FindFirstFileA(PlanViewMap::GetSVMClassifierFileName(), &wfd);
	if (hFile != INVALID_HANDLE_VALUE) {
		currentSVM = cv::ml::SVM::load(svmClassifierFileName);
		FindClose(hFile);
	}
}

PlanViewMap::~PlanViewMap() {
	delete[] workOccupancy;
	delete[] workHeight;
	delete[] colorsForTexels;
	delete[] foregroundPixelsForTexels;
	currentSVM.release();

	/*OPTICAL FLOW RELEASE*/
	cvReleaseImage(&AOFImage);
	cvReleaseImage(&BOFImage);
	cvReleaseImage(&eigOFTemp);
	cvReleaseImage(&tempOFTemp);
	cvReleaseImage(&AOFPyr);
	cvReleaseImage(&BOFPyr);
	delete[] AOFCorners;
	delete[] BOFCorners;
	/*END OPTICAL FLOW RELEASE*/

	Reset();

	cvReleaseMat(&occupancyImage);
	cvReleaseMat(&heightImage);
	cvReleaseMat(&tempPvm);
	cvReleaseMemStorage(&memStorage);
	cvReleaseMat(&contourScratch);
}

void PlanViewMap::UpdatePlanViewMap(CvMat *projection3D, IplImage *foreground, CvMat *disparity, IplImage *leftBGR, CvPoint *imagePixelMapping) {
	static double wc[3];
	static CvMat aux = cvMat(3, 1, CV_64FC1, wc);
	ZeroMemory(workOccupancy, bytes_occupation);
	ZeroMemory(workHeight, bytes_occupation);
	ZeroMemory(foregroundPixelsForTexels, totTexels*sizeof(int));
	
	CvMat *Traslation_Vector = ExternalCalibration::GetInstance()->Traslation_Vector;
	CvMat *Rotation_Matrix = ExternalCalibration::GetInstance()->Rotation_Matrix;
	int width = foreground->width, heights = foreground->height;
	int minDisp = si->GetElaborationCore()->noCorrespondenceDisparityValue;
	CvPoint *pixMapPtr = imagePixelMapping;

	for (int row = 0; row < heights; row++) {
		const float *ptr3D = (const float *) (projection3D->data.ptr + row*projection3D->step);
		const uchar *ptrFor = (const uchar *) (foreground->imageData + row*foreground->widthStep);
		const short *dispPtr = (const short *) (disparity->data.ptr + row*disparity->step);
		const BGRTriple *bgrPtr = (const BGRTriple *) (leftBGR->imageData + row*leftBGR->widthStep);
		for (int col = 0; col < width; col++, ptr3D+=3, ptrFor++, dispPtr++, bgrPtr++, pixMapPtr++) {
			int x_plan = -1, y_plan = -1;
			if (*ptrFor || imagePixelMapping) {
				if (*dispPtr > minDisp) {
					wc[0] = (double) ptr3D[0]; wc[1] = (double) ptr3D[1]; wc[2] = (double) ptr3D[2];
					cvSub(&aux, Traslation_Vector, &aux);
					cvGEMM(Rotation_Matrix, &aux, 1, NULL, 0, &aux);
					if (wc[0] >= rss->WXmin && wc[0] <= rss->WXmax && wc[1] >= rss->WYmin && wc[1] <= rss->WYmax && wc[2] >= rss->WZmin && wc[2] <= rss->WZmax) {
						x_plan = int(floor((float) (((wc[0] - rss->WXmin)/rss->texelSide) + 0.5)));
						y_plan = int(floor((float) (((wc[1] - rss->WYmin)/rss->texelSide) + 0.5)));
					}
				}
				if (imagePixelMapping) {
					pixMapPtr->x = x_plan;
					pixMapPtr->y = y_plan;
				}
			}
			if (*ptrFor && x_plan >= 0 && y_plan >=0) {
				int pos = y_plan*xtexels + x_plan;
				if (y_plan < ytexels && x_plan < xtexels) {
					workOccupancy[pos]+=(ptr3D[2]*ptr3D[2])*z_cam_normalization;
					if (wc[2] > workHeight[pos] && wc[2] <= rss->personMaximumHeight)
						workHeight[pos] = (float) wc[2];
					int color_pos = (pos*100) + foregroundPixelsForTexels[pos];
					colorsForTexels[color_pos] = *bgrPtr;
					foregroundPixelsForTexels[pos]++;
				}
			}
		}
	}

	cvSmooth(&workOccupancyImage, &workOccupancyImage, CV_GAUSSIAN, 3);
	cvSmooth(&workHeightImage, &workHeightImage, CV_GAUSSIAN, 3);

	float theta_occ = chi * rss->texelSide * rss->personMaximumHeight;

	for (int i = 0; i<totTexels; i++) {
		if (workOccupancy[i] < theta_occ) {
			workHeight[i] = 0.f;
			workOccupancy[i] = 0.f;
		}
	}

	cvNormalize(&workOccupancyImage, tempPvm, 0, 255, CV_MINMAX);
	cvConvertImage(tempPvm, occupancyImage);
	cvNormalize(&workHeightImage, tempPvm, 0, 255, CV_MINMAX);
	cvConvertImage(tempPvm, heightImage);

	cvClearMemStorage(memStorage);
	cvConvertImage(occupancyImage, contourScratch);
	cvConvertImage(occupancyImage, (currentlyAOF ? AOFImage : BOFImage)); //OPTICAL FLOW Image
	cvFindContours(contourScratch, memStorage, &currentStereoContours, sizeof(CvContour),
		CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
	/*cvZero(contourScratch);
	if (currentStereoContours) {
		cvDrawContours(contourScratch, currentStereoContours, cvScalar(255), cvScalar(0), 1);
	}
	cvShowImage("Detected Contours", contourScratch);*/
}

void PlanViewMap::IdentifyVisibleSubjects() {
	for (ObjectTemplate *ot : candidateObject) {
		delete ot;
	}
	candidateObject.resize(0);

	const float theta_new_occ = occ_threshold * rss->personAverageHeight * rss->personAverageWidth;

	for (CvSeq *aux = currentStereoContours; aux!=NULL; aux = aux->h_next) {
		CvRect boundingBox = cvBoundingRect(aux);
		ObjectTemplate *ot = new ObjectTemplate(boundingBox.width, boundingBox.height);

		CvPoint upperLeft, bottomRight;
		upperLeft.x = boundingBox.x; bottomRight.x = boundingBox.x + boundingBox.width - 1;
		upperLeft.y = boundingBox.y; bottomRight.y = boundingBox.y + boundingBox.height - 1;

		float *occPtr = ot->occupancyTemplate->data.fl;
		float *heiPtr = ot->heightTemplate->data.fl;

		float totOccupancy = 0.f;
		float maxHeight = 0.f;
		for (int row = upperLeft.y, aux_Row = 0; row<=bottomRight.y; row++, aux_Row++) {
			int pos = row*xtexels + upperLeft.x;
			for (int col = upperLeft.x; col <= bottomRight.x; col++, pos++, occPtr++, heiPtr++) {
				if (cvPointPolygonTest(aux, cvPoint2D32f(col, row), 0) >= 0) {
					*heiPtr = workHeight[pos];
					if (*heiPtr > maxHeight)
						maxHeight = *heiPtr;
					*occPtr = workOccupancy[pos];
					totOccupancy+= *occPtr;
					int pixelsForCurrentTexels = foregroundPixelsForTexels[pos];
					BGRTriple *colorsForCurrentTexels = colorsForTexels + (pos*100);
					for (int i = 0; i < pixelsForCurrentTexels; i++, colorsForCurrentTexels++) {
						//Cromaticity values taken from Scozzafava
						int rg = (colorsForCurrentTexels->r - colorsForCurrentTexels->g) + 255; //We ensure a positive result
						rg = (rg * 63) / 510; //Quantization on 64 levels
						int by = ((2*colorsForCurrentTexels->b - colorsForCurrentTexels->r - colorsForCurrentTexels->g)/2) + 255;
						by = (by * 63) / 510; //Quantization on 64 levels
						//colorTemplate is a 64x64 matrix
						int color_pos = rg*64 + by;
						ot->colorTemplate[color_pos]++;
						ot->colorTemplateSum++;
					}
				} else {
					*heiPtr = *occPtr = 0.f;
				}
			}
		}
		if (maxHeight > (rss->personMimimumHeight/2) && totOccupancy > theta_new_occ) {
			//writeToConsole(L"%d - %d %d %d %d\r\n", bau++, boundingBox.x, boundingBox.y, boundingBox.width, boundingBox.height);
			ot->currentMeasurement->data.fl[0] = (boundingBox.x + (boundingBox.width / 2.f));
			ot->currentMeasurement->data.fl[1] = (boundingBox.y + (boundingBox.height / 2.f));
			ot->currentMeasurement->data.fl[2] = maxHeight / rss->texelSide;
			candidateObject.push_back(ot);
		} else {
			delete ot;
		}
	}
}

inline int NumberOfOccurrences (int *arr, int numberOfElements, int val) {
	int res = 0;
	for (int i = 0; i < numberOfElements; i++) {
		if (arr[i]==val)
			res++;
	}
	return res;
}

void PlanViewMap::Track_em(std::vector<TrackedFaceCorrespondence> *tfc, DWORD dt) {
	char stringBuffer[256];
	int numTrackedPersons = (int) trackedPersons.size();
	int numCandidateObject = (int) candidateObject.size();
	DWORD currentTimestamp = GetTickCount();

	if (numCandidateObject == 0 || numTrackedPersons == 0) {
		if (pvmc)
			pvmc->WaitForController(&candidateObject, &trackedPersons, NULL);
	}

	if (numCandidateObject > 0) {
		bool *isANewTrackedPerson = new bool[numCandidateObject];
		int *numberOfTrackedCandidate = new int[numCandidateObject];
		int *numberOfObjectCandidate = new int[numTrackedPersons];
		int *olderTrackedCandidate = new int[numCandidateObject];
		for (int i = 0; i<numCandidateObject; i++) {
			isANewTrackedPerson[i] = true;
			numberOfTrackedCandidate[i] = 0;
			olderTrackedCandidate[i] = -1;
		}
		memset(numberOfObjectCandidate, 0, sizeof(int)*numTrackedPersons);
		if (numTrackedPersons > 0) {
			CvMat *componentValues = cvCreateMat(numTrackedPersons, numCandidateObject, CV_32FC3);
			Matrix<int> associationResult(numTrackedPersons, numCandidateObject); //For each tracked person is 1 if association was successfull

			char  features_found[500];
			float feature_errors[500];
			if (!svmTracker) { /*OPTICAL FLOW CALCULATION*/
				cvCalcOpticalFlowPyrLK((currentlyAOF ? BOFImage : AOFImage), (currentlyAOF ? AOFImage : BOFImage),
					(currentlyAOF ? BOFPyr : AOFPyr), (currentlyAOF ? AOFPyr : BOFPyr),
					(currentlyAOF ? BOFCorners : AOFCorners), (currentlyAOF ? AOFCorners : BOFCorners),
					numberOfCornersOF, cvSize(20, 20), 5, features_found, feature_errors,
					cvTermCriteria( CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, .3 ), 0);
			} /*OPTICAL FLOW CALCULATION*/

			for (int row = 0; row < numTrackedPersons; row++) {
				//float lostElapsedTime = ((trackedPersons[row]->noUpdateDelay+dt)/1000.f);
				float elapsedTime = dt/1000.f;
				trackedPersons[row]->kalmanFilter->transition_matrix->data.fl[2] =
					trackedPersons[row]->kalmanFilter->transition_matrix->data.fl[8] = elapsedTime;
				trackedPersons[row]->movementKalmanFilter->transition_matrix->data.fl[2] =
					trackedPersons[row]->movementKalmanFilter->transition_matrix->data.fl[8] = elapsedTime;
				//Comment the IF statement if you want to follow Kalman filter even when an object is lost
				//if (trackedPersons[row]->type != LOST) {
					cvKalmanPredict(trackedPersons[row]->kalmanFilter);
					cvKalmanPredict(trackedPersons[row]->movementKalmanFilter);
					trackedPersons[row]->UpdateRepresentation(NULL, true);
				//}
				float *rowPtr = (float *) (componentValues->data.ptr + row*componentValues->step);
				for (int col = 0; col < numCandidateObject; col++, rowPtr += 3) {
					associationResult(row, col) = 0;

					float colorDifference = 0.f;
					if ((currentSVM && svmTracker) || pvmc) { //Tracker based on SVM
						int *candidateColorTemplate = candidateObject[col]->colorTemplate,
							*trackedColorTemplate = trackedPersons[row]->colorTemplate;
						for (int i = 0; i < 4096; i++, candidateColorTemplate++, trackedColorTemplate++) {
							colorDifference+=abs(float(*candidateColorTemplate) - float(*trackedColorTemplate));
						}
						colorDifference/=float(trackedPersons[row]->colorTemplateSum);

						//float averageHeightRatio = abs(candidateObject[col]->currentMeasurement->data.fl[2] - trackedPersons[row]->kalmanFilter->state_pre->data.fl[4]);
						//Maximum value of averageHeightRation is the height of the ceiling in mm divided by the texel side (4m e texel side 2mm --> 2000)

						float diffX = candidateObject[col]->currentMeasurement->data.fl[0] - trackedPersons[row]->kalmanFilter->state_pre->data.fl[0];
						float diffY = candidateObject[col]->currentMeasurement->data.fl[1] - trackedPersons[row]->kalmanFilter->state_pre->data.fl[1];
						float positionDifference = sqrt(diffX*diffX + diffY*diffY);
						positionDifference /= (500.f/rss->texelSide);

						float candidateXDiff = candidateObject[col]->currentMeasurement->data.fl[0] - trackedPersons[row]->kalmanFilter->state_post->data.fl[0];
						float candidateYDiff = candidateObject[col]->currentMeasurement->data.fl[1] - trackedPersons[row]->kalmanFilter->state_post->data.fl[1];
						float angoloCompreso = (candidateXDiff*trackedPersons[row]->kalmanFilter->state_post->data.fl[2] + candidateYDiff*trackedPersons[row]->kalmanFilter->state_post->data.fl[3]);
						if (angoloCompreso != 0.f) {
							angoloCompreso /= (sqrt(candidateXDiff*candidateXDiff + candidateYDiff*candidateYDiff)
								*sqrt(trackedPersons[row]->kalmanFilter->state_post->data.fl[2]*trackedPersons[row]->kalmanFilter->state_post->data.fl[2] + trackedPersons[row]->kalmanFilter->state_post->data.fl[3]*trackedPersons[row]->kalmanFilter->state_post->data.fl[3]));
							angoloCompreso = acos(angoloCompreso);
						}

						//leostorm::logging::Logger::writeToLOG(L"dt: %lu CD: %f AVH: %f PD: %f", dt, colorDifference, positionDifference, velocityDifference);

						rowPtr[0] = colorDifference;
						rowPtr[1] = positionDifference;
						rowPtr[2] = angoloCompreso;
					}
					if (!pvmc) {
						if (!currentSVM.empty() && svmTracker) {
							cv::Mat sampleMat(1, 3, CV_32FC1, rowPtr);
							associationResult(row, col) = int(currentSVM->predict(sampleMat));
							if (!associationResult(row, col)) {
								float diffX = candidateObject[col]->currentMeasurement->data.fl[0] - trackedPersons[row]->movementKalmanFilter->state_pre->data.fl[0];
								float diffY = candidateObject[col]->currentMeasurement->data.fl[1] - trackedPersons[row]->movementKalmanFilter->state_pre->data.fl[1];
								float positionDifference = sqrt(diffX*diffX + diffY*diffY);
								positionDifference /= (500.f/rss->texelSide);

								float candidateXDiff = candidateObject[col]->currentMeasurement->data.fl[0] - trackedPersons[row]->movementKalmanFilter->state_post->data.fl[0];
								float candidateYDiff = candidateObject[col]->currentMeasurement->data.fl[1] - trackedPersons[row]->movementKalmanFilter->state_post->data.fl[1];
								float angoloCompreso = (candidateXDiff*trackedPersons[row]->movementKalmanFilter->state_post->data.fl[2] + candidateYDiff*trackedPersons[row]->movementKalmanFilter->state_post->data.fl[3]);
								if (angoloCompreso != 0.f) {
									angoloCompreso /= (sqrt(candidateXDiff*candidateXDiff + candidateYDiff*candidateYDiff)
										*sqrt(trackedPersons[row]->movementKalmanFilter->state_post->data.fl[2]*trackedPersons[row]->movementKalmanFilter->state_post->data.fl[2] + trackedPersons[row]->movementKalmanFilter->state_post->data.fl[3]*trackedPersons[row]->movementKalmanFilter->state_post->data.fl[3]));
									angoloCompreso = acos(angoloCompreso);
								}

								rowPtr[0] = colorDifference;
								rowPtr[1] = positionDifference;
								rowPtr[2] = angoloCompreso;

								cv::Mat sampleMat(1, 3, CV_32FC1, rowPtr);
								associationResult(row, col) = -int(currentSVM->predict(sampleMat));
								//if (associationResult(row, col))
								//	Logger::writeToLOG(L"c");
							}
						} else if (!svmTracker) { //OPTICAL FLOW TRACKING START...no SVM has been selected
							CvPoint candUL, candBR;
							candUL.x = int(candidateObject[col]->currentMeasurement->data.fl[0] - (candidateObject[col]->xSideBox / 2.f));
							candUL.y = int(candidateObject[col]->currentMeasurement->data.fl[1] - (candidateObject[col]->ySideBox / 2.f));
							candBR.x = candUL.x + candidateObject[col]->xSideBox;
							candBR.y = candUL.y + candidateObject[col]->ySideBox;
							for (int cI = 0; cI < numberOfCornersOF; cI++) {
								if (features_found[cI] != 0 && feature_errors[cI] <= 550) {
									CvPoint2D32f* prev = (currentlyAOF ? BOFCorners : AOFCorners) + cI;
									CvPoint2D32f* curr = (currentlyAOF ? AOFCorners : BOFCorners) + cI;
									if (prev->x <= trackedPersons[row]->bottomRight.x && prev->x >= trackedPersons[row]->upperLeft.x
										&& prev->y <= trackedPersons[row]->bottomRight.y && prev->y >= trackedPersons[row]->upperLeft.y
										&& curr->x <= candBR.x && curr->x >= candUL.x && curr->y <= candBR.y && curr->y >= candUL.y) {
										associationResult(row, col) = associationResult(row, col) + 1;
									}
								}
							}
						}//OPTICAL FLOW TRACKING END
						if (associationResult(row, col)) {
							if (numberOfTrackedCandidate[col] == 0 || trackedPersons[olderTrackedCandidate[col]]->creationTimestamp > trackedPersons[row]->creationTimestamp) {
								olderTrackedCandidate[col] = row;
							}
							numberOfTrackedCandidate[col]++;
							isANewTrackedPerson[col] = false;
							numberOfObjectCandidate[row]++;
						}
					}
				}
			}

			int accuracyCorrect = 0, accuracyTotal = 0, accuracyAppliedCorrect = 0;
			if (pvmc) {
				pvmc->WaitForController(&candidateObject, &trackedPersons, componentValues);
				if (pvmc) {
					accuracyTotal = numTrackedPersons * numCandidateObject;
					for (int i = 0; i < numTrackedPersons; i++) {
						if (pvmc->accuracyController) {
							if (currentSVM) {
								for (int j = 0; j < numCandidateObject; j++) {
									if ((pvmc->currentAssociation[i] == j && associationResult(i, j)) || (pvmc->currentAssociation[i] != j && !associationResult(i, j))) {
										accuracyCorrect++;
									}
								}
							}
						} else {
							int associatedObject = pvmc->currentAssociation[i];
							if (associatedObject >= 0) {
								trackedPersons[i]->SetTemplate(candidateObject[associatedObject], 1);
								isANewTrackedPerson[associatedObject] = false;
								numberOfTrackedCandidate[associatedObject]++;
								numberOfObjectCandidate[i]++;
							} else {
								trackedPersons[i]->NoCorrespondence(dt);
							}
						}
					}
					if (pvmc->accuracyController)
						pvmc->AccuracyCallback(accuracyCorrect, accuracyTotal, accuracyAppliedCorrect, numTrackedPersons);
				}
			} else if ((svmTracker && currentSVM) || !svmTracker) {
				bool *haveBeenAssigned = new bool[numTrackedPersons];
				memset(haveBeenAssigned, 0, numTrackedPersons * sizeof(bool));
				for (int j = 0; j < numCandidateObject; j++) {
					/*int i = olderTrackedCandidate[j];
					if (i != -1 && numberOfObjectCandidate[i] == 1) {
						trackedPersons[i]->SetTemplate(candidateObject[j], associationResult(i, j));
						haveBeenAssigned[i] = true;
					}*/
					int assignedTracked = -1;
					for (int i = 0; i < numTrackedPersons; i++) {
						if (associationResult(i, j) && numberOfObjectCandidate[i] == 1 && numberOfTrackedCandidate[j] == 1) {
							assignedTracked = i;
						}
					}
					if (assignedTracked != -1) {
						trackedPersons[assignedTracked]->SetTemplate(candidateObject[j], associationResult(assignedTracked, j));
						haveBeenAssigned[assignedTracked] = true;
					}
				}
				for (int i = 0; i < numTrackedPersons; i++) {
					if (!haveBeenAssigned[i]) {
						trackedPersons[i]->NoCorrespondence(dt);
					}
				}
				delete[] haveBeenAssigned;
			}
		}

		for (int i = 0; i<numCandidateObject; i++) {
			if (isANewTrackedPerson[i]) {
				TrackedObject *to = new TrackedObject(candidateObject[i]);
				trackedPersons.push_back(to);
			}
		}
		delete[] isANewTrackedPerson;
		delete[] numberOfTrackedCandidate;
		delete[] numberOfObjectCandidate;
		delete[] olderTrackedCandidate;
	} else {
		for (int row = 0; row < numTrackedPersons; row++) {
			trackedPersons[row]->NoCorrespondence(dt);
		}
	}

	std::vector<TrackedFaceCorrespondence> &tfc_ref = *tfc;
	if (tfc) {
		tfc_ref.resize(0);
		tfc_ref.resize(trackedPersons.size());
	}

	/*OPTICAL FLOW FEATURES CALCULATION*/
	CvPoint2D32f *currentCorners = (currentlyAOF ? AOFCorners : BOFCorners);
	int numberOfCorners = 500;
	cvGoodFeaturesToTrack((currentlyAOF ? AOFImage : BOFImage), eigOFTemp, tempOFTemp,
		currentCorners, &numberOfCorners,
		0.01, 5.0, 0, 3, 0, 0.04);
	cvFindCornerSubPix((currentlyAOF ? AOFImage : BOFImage), currentCorners, numberOfCorners,
		cvSize(10, 10), cvSize(-1,-1), cvTermCriteria(CV_TERMCRIT_ITER|CV_TERMCRIT_EPS,20,0.03));
	/*for (int i = 0; i < numberOfCorners; i++) {
		CvPoint punto = cvPoint(cvRound(currentCorners[i].x), cvRound(currentCorners[i].y));
		cvDrawCircle(occupancyImage, punto, 3, cvScalar(255.0));
	}*/
	numberOfCornersOF = numberOfCorners;
	currentlyAOF = !currentlyAOF;
	/*END OPTICAL FLOW FEATURES CALCULATION*/

	int j = 0;
	for (unsigned int i = 0; i < trackedPersons.size();) {
		TrackedObject *to = trackedPersons[i];
		if (trackedPersons[i]->justIdentified)
			trackedPersons[i]->justIdentified = false;
		if (to->type == STALE) {
			trackedPersons.erase(trackedPersons.begin() + i);
		} else {
			if (to->type != NEW_OBJECT || pvmc) {
				cvDrawRect(occupancyImage, to->upperLeft, to->bottomRight, to->color);
				cvDrawRect(heightImage, to->upperLeft, to->bottomRight, to->color);
				if (to->nameID != -1) {
					strcpy_s(stringBuffer, 256, to->name);
				} else {
					sprintf_s(stringBuffer, 256, "%d", trackedPersons[i]->ID);
				}
				cvPutText(occupancyImage, stringBuffer, cvPoint(int(to->kalmanFilter->state_post->data.fl[0]),
					int(to->kalmanFilter->state_post->data.fl[1])), &font, cvScalar(255, 255, 255));
			}
			if (tfc && to->type != LOST) {
				tfc_ref[j].ID = to->ID;
				tfc_ref[j].boundingRect.height = to->bottomRight.y - to->upperLeft.y;
				tfc_ref[j].boundingRect.width = to->bottomRight.x - to->upperLeft.x;
				tfc_ref[j].boundingRect.x = to->upperLeft.x;
				tfc_ref[j].boundingRect.y = to->upperLeft.y;
				tfc_ref[j].founded = false;
				tfc_ref[j].xPosition = to->kalmanFilter->state_post->data.fl[0];
				tfc_ref[j].yPosition = to->kalmanFilter->state_post->data.fl[1];
				j++;
			}
			i++;
		}
	}
	if (tfc)
		tfc_ref.resize(j);
}

void PlanViewMap::InterpolateIdentity(std::vector<TrackedFaceCorrespondence> &tfc) {
	for (unsigned int i = 0; i < tfc.size(); i++) {
		if (tfc[i].founded) {
			for (unsigned int j = 0; j < trackedPersons.size(); j++) {
				if (trackedPersons[j]->nameID == -1 && tfc[i].ID == trackedPersons[j]->ID) {
					if (tfc[i].personID != trackedPersons[j]->nameID) {
						if (tfc[i].personID == trackedPersons[j]->suspectID) {
							trackedPersons[j]->numberOfProbation++;
							if (trackedPersons[j]->numberOfProbation == 2) {
								strcpy_s(trackedPersons[j]->name, tfc[i].name);
								trackedPersons[j]->nameID = tfc[i].personID;
								trackedPersons[j]->justIdentified = true;
								trackedPersons[j]->suspectID=-1;
								trackedPersons[j]->numberOfProbation=0;
							}
						} else {
							trackedPersons[j]->suspectID = tfc[i].personID;
							trackedPersons[j]->numberOfProbation=1;
						}
						Logger::writeToLOG(L"Tracked ID %d is %S with %d probes\r\n", trackedPersons[j]->ID,
							tfc[i].name, trackedPersons[j]->numberOfProbation);
					}
					break;
				}
			}
		}
	}
}

char *PlanViewMap::GetSVMClassifierFileName() {
	return svmClassifierFileName;
}

void PlanViewMap::SetSVMClassifierFileName(const char *fileName) {
	strcpy_s(svmClassifierFileName, _MAX_PATH, fileName);
}

void PlanViewMap::Reset() {
	for (ObjectTemplate *ot : candidateObject) {
		delete ot;
	}
	candidateObject.resize(0);
	for (TrackedObject *to : trackedPersons) {
		delete to;
	}
	trackedPersons.resize(0);
	TrackedObject::staticID = 0;
}