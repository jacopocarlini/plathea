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

#ifndef POSITION_TEST_H
#define POSITION_TEST_H

#include "CommonHeader.h"
#include "TestDesigner.h"
#include "PlanViewMap.h"
#include "ElaborationCore.h"

typedef std::vector<DWORD> GroundTruthMeasurementsVector;
typedef std::vector<GroundTruthMeasurementsVector> GroundTruthSubjectsVector;

struct PositionTestMeasuredEntry {
	DWORD timestamp;
	float x_position, y_position;
	float x_velocity, y_velocity;
	float max_height;
	float x_base, y_base;
	TrackedObjectType type;
	int nameID;
	int trackingID;
	PositionTestMeasuredEntry() {}
	PositionTestMeasuredEntry(void *xmlElem);
	void toXMLElement(void *xmlElem) const;
};

typedef std::vector<PositionTestMeasuredEntry> SoftwareMeasurementsVector;
typedef std::vector<SoftwareMeasurementsVector> MeasuredSubjectsMap;
typedef std::unordered_map<int, SoftwareMeasurementsVector> TrackedMeasurementMap;

class PositionTest: public std::vector<int> {
friend class PositionTestVector;
public:
	enum VisualizationStyle {SHOW_PATH = 1, SHOW_CURRENT_POINT = 2, SHOW_GRID = 4, SHOW_SOFTWARE_MEASUREMENTS = 8, SHOW_REAL_SW_CORRESPONDENCE = 16};
	enum PositionTestType {ONLINE_POSITION_TEST = 1, OFFLINE_POSITION_TEST = 2, COMPLETION_POSITION_TEST = 4, NOT_INITIALIZED_TEST = 8};
private:
	MeasuredSubjectsMap softwareMeasurements;
	TrackedMeasurementMap allSoftwareMeasurements;
	GroundTruthSubjectsVector groundTruthMeasurements;
	std::vector<std::vector<int>> groundTruthToSoftwareMeasurements;
	std::vector<int> numberOfTrackedObjectsAssociated;

	std::vector<int> usersToSubjects;

	CvArrStorage cas;

	char completionDate[64];
	char completionTime[64];

	char additionalDataDirectory[_MAX_PATH];

	PositionTestType ptt;

	DWORD startMillisecond;

	bool testStarted;

	bool AssociateSoftwareMeasurementsToUsers(TestDesigner &td, float texelSide);
public:
	PositionTest & operator=(const std::vector<int> &other);
	bool UpdateImage(IplImage *image, TestDesigner &td, float texelSide = 0.f, int visualizationMask = SHOW_PATH, int gridStep = 1000, std::vector<int> *pathToShow = NULL);
	bool SaveToPS(char * destString, int width, int height, TestDesigner &td, float texelSide = 0.f, int visualizationMask = SHOW_PATH, int gridStep = 1000, std::vector<int> *pathToShow = NULL);
	PositionTest(): testStarted(false), ptt(NOT_INITIALIZED_TEST) {}
	~PositionTest() {
		clear();
	}
	void clear();
	PositionTest(void *xmlElem);
	const GroundTruthMeasurementsVector &GetGroundTruthMeasurements(int userID) const {
		return groundTruthMeasurements[userID - 1];
	}
	const SoftwareMeasurementsVector &GetSoftwareMeasurements(int userId) const {
		return softwareMeasurements[userId];
	}
	GroundTruthMeasurementsVector &GetGroundTruthMeasurements(int userID) {
		return groundTruthMeasurements[userID - 1];
	}
	SoftwareMeasurementsVector &GetSoftwareMeasurements(int userId) {
		return softwareMeasurements[userId];
	}
	int &GetUserToSubjectCorrespondence(int userId) {
		return usersToSubjects[userId - 1];
	}
	const char * GetCompletionTime() const {
		return completionTime;
	}
	const char * GetCompletionDate() const {
		return completionDate;
	}
	void SetAdditionalDataDirectory(const char * directory) {
		strcpy_s(additionalDataDirectory, directory);
	}
	const char * GetAdditionalDataDirectory() const {
		return additionalDataDirectory;
	}
	void SetPositionTestType(PositionTestType ptt) {
		this->ptt = ptt;
	}
	PositionTestType GetPositionTestType() const {
		return ptt;
	}
	DWORD GetStartTime() const {
		return startMillisecond;
	}
	CvArrStorage & GetCvArrStorage() {
		return cas;
	}
	void toXMLElement(void *xmlElem) const;
	void AddLastSoftwareMeasurements(TestDesigner &td, float texelSide, SoftwareMeasurementsVector &nyad, RWLock &nyadLock);
	void StartNewTest(std::vector<int> *usersToSubjects);
	void StopTest(TestDesigner &td, float texelSide);
	bool IsTestStarted() {
		return testStarted;
	}
};

void SelectExperiment(HWND pathList, PositionTest *testToShow, std::unordered_map<int, int> *pathIDToImageID, bool createColumn, bool selectAll);

#endif //POSITION_TEST_H