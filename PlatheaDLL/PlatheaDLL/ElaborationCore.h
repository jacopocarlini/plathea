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

#ifndef ELABORATION_CORE
#define ELABORATION_CORE

#include "CommonHeader.h"
#include "StereoExplorer.h"
#include "ConnectedComponents.h"
#include "FaceDatabase.h"
#include "NetworkCamera.h"
#include "LeottaBackgroundModel.h"

#include <LeoWindowsThread.h>
#include <LeoSettingsPersistence.h>

struct TrackedFaceCorrespondence {
	int ID;
	char name[256];
	int personID;
	CvRect boundingRect;
	float xPosition;
	float yPosition;
	bool founded;
};

struct HaarClassifierThreadParam {
	IplImage *bigImagePointer;
	HANDLE hEventBigImageReady, hEventFacesReady, hStopRunningEvent;
	CvSeq *faces;
	CvHaarClassifierCascade *cascade;
	CvPoint *imagePixelMapping;
	std::vector<TrackedFaceCorrespondence> tfc;
};

class PlanViewMap;

class ElaborationCore: public Thread, public PrerequisitesChecker {
public:
	enum ElaborationCoreMode{FULL_FEATURE_ELABORATION_CORE_MODE, TRACKING_FREE_ELABORATION_CORE_MODE};
private:
	HANDLE hThread, hStereoEvent, hStopRunningEvent;
	bool initPhase;
	BackgroundModeling *bm;
	PlanViewMap *pvm;
	CvStereoBMState *&BMState;
	//cv::StereoBM *BMState;
	cv::StereoSGBM *SGBMState;
	RWLock modelLock;
	ElaborationCoreMode currentElaborationCoreMode;
	bool saveToFile; char saveToFilename[MAX_PATH];
public:
	ElaborationCore();
	~ElaborationCore();
	BackgroundModeling * GetBackgroundModeling() {
		return bm;
	}
	PlanViewMap * GetPlanViewMap() {
		return pvm;
	}
	void ShowSettingsWindow();
	void EndInitializationPhase() {		
		initPhase = 0;
		printf("%d initPhase  false=%d\n", &initPhase, initPhase);
	}
	bool DuringInitializationPhase() {
		return initPhase;
	}
	void SetElaborationCoreMode(ElaborationCoreMode ecm) {
		currentElaborationCoreMode = ecm;
	}
	ElaborationCoreMode GetElaborationCoreMode() {
		return currentElaborationCoreMode;
	}
	int &noCorrespondenceDisparityValue;
	bool PrerequisitesCheck(wchar_t *errMsg = 0, int bufferSize = 0, bool *warning = 0);
	void SaveCurrentState(CvArrStorage &stateContainer);
	void LoadSavedState(CvArrStorage &stateContainer);
	bool ActivateSaveToFileMode(const char *fileName);
protected:
	virtual void Run(void *param = NULL);
	virtual bool StopPreprocedure();
};

#endif /* ELABORATION_CORE */