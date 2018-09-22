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

#ifndef FACE_DATABASE
#define FACE_DATABASE

#include "CommonHeader.h"
#include "SIFT\sift.h"
#include "SIFT\imgfeatures.h"
#include "SIFT\kdtree.h"
#include "SIFT\utils.h"
#include "Shared.h"

#include <LeoSettingsPersistence.h>

struct TrainingFace {
private:
	void CreateSIFTStructures();
	void DeleteSIFTStructures();
	bool externalImage;
	char filename[MAX_PATH];
public:
	friend struct Person;
	int total;
	IplImage *image;
	feature *features;
	kd_node *kdRoot;
	TrainingFace(): image(NULL), features(NULL), kdRoot(NULL), total(0), externalImage(false) {}
	TrainingFace(IplImage *sourceImage): image(NULL), features(NULL), kdRoot(NULL), total(0), externalImage(false) {
		//writeToConsole(L"Standard Constructor\r\n");
		image = cvCloneImage(sourceImage);
		CreateSIFTStructures();
		strcpy_s(filename, MAX_PATH, "");
	}
	TrainingFace(const char *faceFile, const char *faceURL, bool externalImage = false): image(NULL), features(NULL), kdRoot(NULL), total(0) {
		image = cvLoadImage(faceFile);
		CreateSIFTStructures();
		this->externalImage = externalImage;
		strcpy_s(filename, MAX_PATH, faceURL);
	}
	TrainingFace(const TrainingFace &other): image(NULL), features(NULL), kdRoot(NULL), total(0) {
		//writeToConsole(L"Copy Constructor\r\n");
		MakeACopy(other);
	}
	TrainingFace & operator= (const TrainingFace & other) {
		//writeToConsole(L"Assigment Operator\r\n");
		MakeACopy(other);
		return (*this);
	}
	~TrainingFace() {
		//writeToConsole(L"Destructor\r\n");
		if (image)
			cvReleaseImage(&image);
		DeleteSIFTStructures();
	}
	char *GetFileName() {
		return (externalImage ? filename : NULL);
	}
private:
	void MakeACopy(const TrainingFace &other) {
		if (image)
			cvReleaseImage(&image);
		image = cvCloneImage(other.image);
		CreateSIFTStructures();
		externalImage = other.externalImage;
		strcpy_s(filename, MAX_PATH, other.filename);
	}
};

struct Person: public std::vector<TrainingFace> {
	char name[256];
	int height, shoulders, chest;
	int AddTrainingFace(TrainingFace &tf);
	void DeleteData();
	Person() {}
	Person(void *xmlElem);
	void toXMLElement(void *xmlElem) const;
};

INT_PTR CALLBACK FaceDBaseDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

class FaceDatabase: public PrerequisitesChecker, public std::unordered_map<int, Person>, public leostorm::settingspersistence::ParameterSetter {
private:
	char haarFileName[MAX_PATH];
	char faceDatabaseDirectory[MAX_PATH];

	int currentPersonID;
	void ClearMemory();
public:
	FaceDatabase();
	~FaceDatabase();
	//Special Management functions
	int FindPersonIndexByName(char *name);
	int AddPersonToDBase(Person &p);
	bool RemovePersonFromDBase(char *name);
	//Matching function
	int FaceMatch(IplImage *face, int *score);

	//External parameter setter
	char *GetHaarClassifierFilename();
	void SetHaarClassifierFileName(const char *fileName);
	char *GetFaceDatabaseDirectory() {
		return faceDatabaseDirectory;
	}
	void SetFaceDatabaseDirectory(char *fileName) {
		strcpy_s(faceDatabaseDirectory, MAX_PATH, fileName);
	}

	//Prerequisite checker
	bool PrerequisitesCheck(wchar_t *errMsg = 0, int bufferSize = 0, bool *warning = 0);

	//CallbackParameterSetter
	bool LoadFromXMLElement(TiXmlElement *xmlElement);
	bool SaveToXMLElement(TiXmlElement *xmlElement) const;
};

extern FaceDatabase mainFaceDatabase;

#endif //FACE_DATABASE