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

#include "PositionTest.h"
#include "Shared.h"

#include <LeoLog4CPP.h>
using namespace leostorm::logging;

bool PositionTest::UpdateImage(IplImage *image, TestDesigner &td, float texelSide, int visualizationMask, int gridStep, std::vector<int> *pathToShow) {
	//cvSetZero(image);
	cvRectangle(image, cvPoint(0, 0), cvPoint(image->width - 1, image->height - 1), cvScalar(255, 255, 255), CV_FILLED);
	if (size()) {
		int width = image->width - 30, height = image->height - 30;
		long maxX = 0, maxY = 0;
		for (std::vector<int>::const_iterator it = begin(); it != end(); it++)
			td[*it].GetExtremeValues(maxX, maxY);
		float normalizationFactor = MIN(float(width)/float(maxX), float(height)/float(maxY));

		if (visualizationMask & SHOW_SOFTWARE_MEASUREMENTS) {
			//SOME BULLSHIT CODE...IF WE HAVE SOFTWARE MEASUREMENTS WE DO NEED A ZOOM OUT
			normalizationFactor *= 0.9f;
		}

		int normalizedMaxX = int(maxX * normalizationFactor);
		int normalizedMaxY = int(maxY * normalizationFactor);

		if (visualizationMask & SHOW_GRID) {
			float normalizedGridStep = gridStep * normalizationFactor;
			if (int(normalizedGridStep) < 15)
				return false;
			long numberOfHorzGridLines = long(normalizedMaxY / normalizedGridStep);
			long numberOfVertGridLines = long(normalizedMaxX / normalizedGridStep);
			for (int i = 1; i <= numberOfVertGridLines; i++) {
				cvLine(image, cvPoint(15 + int(normalizedGridStep * i), 15), cvPoint(15 + int(normalizedGridStep * i), normalizedMaxY + 15),
					cvScalar(0, 0, 255), 1);
			}
			for (int i = 1; i <=numberOfHorzGridLines; i++) {
				cvLine(image, cvPoint(15, 15 + int(normalizedGridStep * i)), cvPoint(normalizedMaxX + 15, 15 + int(normalizedGridStep * i)),
					cvScalar(0, 0, 255), 1);
			}
		}

		cvLine(image, cvPoint(15, 15), cvPoint(normalizedMaxX + 15, 15), cvScalar(0, 0, 0));
		cvLine(image, cvPoint(15, 15), cvPoint(15, normalizedMaxY + 15), cvScalar(0, 0, 0));
		cvLine(image, cvPoint(15, normalizedMaxY + 15),
			cvPoint(normalizedMaxX + 15, normalizedMaxY + 15), cvScalar(0, 0, 0));
		cvLine(image, cvPoint(normalizedMaxX + 15, 15),
			cvPoint(normalizedMaxX + 15, normalizedMaxY + 15), cvScalar(0, 0, 0));

		/*std::vector<std::vector<int>> groundTruthToSoftwareMeasurements;
		if (visualizationMask & SHOW_SOFTWARE_MEASUREMENTS)
			DoAcquiredDataAnalysis(groundTruthToSoftwareMeasurements);*/

		std::vector<int> &pathVector = (pathToShow ? (*pathToShow) : (*this));
		for (std::vector<int>::const_iterator it = pathVector.begin(); it != pathVector.end(); it++) {
			int correspondentUserIndex = (int) (std::find(begin(), end(), (*it)) - begin());
			COLORREF pathColor = td[*it].pathColor;
			CvScalar scalarPathColor = cvScalar(GetBValue(pathColor), GetGValue(pathColor), GetRValue(pathColor));

			if (visualizationMask & SHOW_PATH) {
				td[*it].DrawLines(image, normalizationFactor);
				td[*it].DrawPoints(image, normalizationFactor);
			}

			if (visualizationMask & SHOW_SOFTWARE_MEASUREMENTS || visualizationMask & SHOW_REAL_SW_CORRESPONDENCE) {
				SoftwareMeasurementsVector &correspondentUserSWMeasure = softwareMeasurements[correspondentUserIndex];

				std::vector<int> &correspondentTruthToSoftwareMeasurements = groundTruthToSoftwareMeasurements[correspondentUserIndex];
				std::vector<int>::iterator researchBegin = correspondentTruthToSoftwareMeasurements.begin();

				int softwarePointIndex = 0;
				for (SoftwareMeasurementsVector::const_iterator swIt = correspondentUserSWMeasure.begin(); swIt != correspondentUserSWMeasure.end(); swIt++, softwarePointIndex++) {
					int pointThickness = 1;
					std::vector<int>::iterator foundMeasurement = std::find(researchBegin, correspondentTruthToSoftwareMeasurements.end(), softwarePointIndex);
					if (foundMeasurement != correspondentTruthToSoftwareMeasurements.end()) {
						researchBegin = foundMeasurement + 1;
						if (visualizationMask & SHOW_REAL_SW_CORRESPONDENCE) {
							int groundTruthIndex = int(foundMeasurement - correspondentTruthToSoftwareMeasurements.begin());
							cvLine(image, cvPoint(int(swIt->x_position * normalizationFactor * texelSide) + 15,
								int(swIt->y_position * normalizationFactor * texelSide) + 15),
								cvPoint(int(td[*it][groundTruthIndex].x * normalizationFactor) + 15,
								int(td[*it][groundTruthIndex].y * normalizationFactor) + 15), cvScalar(0, 0, 0));
						}
						if (!(visualizationMask & SHOW_SOFTWARE_MEASUREMENTS)) {
							cvCircle(image, cvPoint(int(swIt->x_position * normalizationFactor * texelSide) + 15,
							int(swIt->y_position * normalizationFactor * texelSide) + 15), pointThickness, scalarPathColor);
						}
						pointThickness = 3;
					}
					if (visualizationMask & SHOW_SOFTWARE_MEASUREMENTS) {
						cvCircle(image, cvPoint(int(swIt->x_position * normalizationFactor * texelSide) + 15,
							int(swIt->y_position * normalizationFactor * texelSide) + 15), pointThickness, scalarPathColor);
					}
				}
			}
		}
		
		if (visualizationMask & SHOW_CURRENT_POINT) {
			int i = 0;
			for (std::vector<int>::const_iterator it = begin(); it != end(); it++, i++) {
				int userMeasurementsNumber = MIN((int) groundTruthMeasurements[i].size(), (int) td[*it].size());
				if (userMeasurementsNumber) {
					int posX = int(td[*it][userMeasurementsNumber - 1].x * normalizationFactor) + 15;
					int posY = int(td[*it][userMeasurementsNumber - 1].y * normalizationFactor) + 15;
					cvCircle(image, cvPoint(posX, posY),
						10, cvScalar(GetBValue(td[*it].pathColor), GetGValue(td[*it].pathColor), GetRValue(td[*it].pathColor)), 3, CV_AA);
				}
			}
		}
		return true;
	}
	return false;
}

bool PositionTest::SaveToPS(char * destString, int width, int height, TestDesigner &td, float texelSide, int visualizationMask, int gridStep, std::vector<int> *pathToShow) {
	if (size()) {
		width = width - 30; height = height - 30;
		long maxX = 0, maxY = 0;
		for (std::vector<int>::const_iterator it = begin(); it != end(); it++)
			td[*it].GetExtremeValues(maxX, maxY);
		float normalizationFactor = MIN(float(width)/float(maxX), float(height)/float(maxY));

		if (visualizationMask & SHOW_SOFTWARE_MEASUREMENTS) {
			//SOME BULLSHIT CODE...IF WE HAVE SOFTWARE MEASUREMENTS WE DO NEED A ZOOM OUT
			normalizationFactor *= 0.9f;
		}

		int normalizedMaxX = int(maxX * normalizationFactor);
		int normalizedMaxY = int(maxY * normalizationFactor);

		strcat(destString, "0.0 0.0 0.0 setrgbcolor\n");
		strcat(destString, "1 setlinewidth\n");
		//BEGIN SURROUNDING BOX
		strcat(destString, "newpath\n");
		strcat(destString, "15 15 moveto\n"); sprintf(destString, "%s%d 15 lineto\n", destString, normalizedMaxX + 15);
		sprintf(destString, "%s%d %d lineto\n", destString, normalizedMaxX + 15, normalizedMaxY + 15);
		sprintf(destString, "%s%d %d lineto\n", destString, 15, normalizedMaxY + 15);
		sprintf(destString, "%s%d %d lineto\n", destString, 15, 15);
		strcat(destString, "closepath\n");
		strcat(destString, "stroke\n");
		//END SURROUNDING BOX

		std::vector<int> &pathVector = (pathToShow ? (*pathToShow) : (*this));
		for (std::vector<int>::const_iterator it = pathVector.begin(); it != pathVector.end(); it++) {
			int correspondentUserIndex = (int) (std::find(begin(), end(), (*it)) - begin());
			COLORREF pathColor = td[*it].pathColor;

			if (visualizationMask & SHOW_PATH) {
				td[*it].DrawLinesPS(destString, normalizationFactor);
				td[*it].DrawPointsPS(destString, normalizationFactor);
			}

			if (visualizationMask & SHOW_SOFTWARE_MEASUREMENTS || visualizationMask & SHOW_REAL_SW_CORRESPONDENCE) {
				SoftwareMeasurementsVector &correspondentUserSWMeasure = softwareMeasurements[correspondentUserIndex];

				std::vector<int> &correspondentTruthToSoftwareMeasurements = groundTruthToSoftwareMeasurements[correspondentUserIndex];
				std::vector<int>::iterator researchBegin = correspondentTruthToSoftwareMeasurements.begin();

				int softwarePointIndex = 0;
				for (SoftwareMeasurementsVector::const_iterator swIt = correspondentUserSWMeasure.begin(); swIt != correspondentUserSWMeasure.end(); swIt++, softwarePointIndex++) {
					int pointThickness = 1;
					std::vector<int>::iterator foundMeasurement = std::find(researchBegin, correspondentTruthToSoftwareMeasurements.end(), softwarePointIndex);
					if (foundMeasurement != correspondentTruthToSoftwareMeasurements.end()) {
						researchBegin = foundMeasurement + 1;
						if (visualizationMask & SHOW_REAL_SW_CORRESPONDENCE) {
							int groundTruthIndex = int(foundMeasurement - correspondentTruthToSoftwareMeasurements.begin());
							strcat(destString, "0.0 0.0 0.0 setrgbcolor\n");
							strcat(destString, "1 setlinewidth\n");
							strcat(destString, "newpath\n");
							sprintf(destString, "%s%d %d moveto\n", destString,
								int(swIt->x_position * normalizationFactor * texelSide) + 15, int(swIt->y_position * normalizationFactor * texelSide) + 15);
							sprintf(destString, "%s%d %d lineto\n", destString,
								int(td[*it][groundTruthIndex].x * normalizationFactor) + 15, int(td[*it][groundTruthIndex].y * normalizationFactor) + 15);
							strcat(destString, "stroke\n");
						}
						if (!(visualizationMask & SHOW_SOFTWARE_MEASUREMENTS)) {
							sprintf(destString, "%s%f %f %f setrgbcolor\n", destString, float(GetRValue(pathColor))/255.f, float(GetGValue(pathColor))/255.f, float(GetBValue(pathColor))/255.f);
							strcat(destString, "1 setlinewidth\n");
							sprintf(destString, "%s%d %d %d 0 360 arc stroke\n", destString,
								int(swIt->x_position * normalizationFactor * texelSide) + 15,
								int(swIt->y_position * normalizationFactor * texelSide) + 15,
								pointThickness);
						}
						pointThickness = 2;
					}
					if (visualizationMask & SHOW_SOFTWARE_MEASUREMENTS) {
						sprintf(destString, "%s%f %f %f setrgbcolor\n", destString, float(GetRValue(pathColor))/255.f, float(GetGValue(pathColor))/255.f, float(GetBValue(pathColor))/255.f);
						strcat(destString, "1 setlinewidth\n");
						sprintf(destString, "%s%d %d %d 0 360 arc stroke\n", destString,
								int(swIt->x_position * normalizationFactor * texelSide) + 15,
								int(swIt->y_position * normalizationFactor * texelSide) + 15,
								pointThickness);
					}
				}
			}
		}
		return true;
	}
	return false;
}

void PositionTest::AddLastSoftwareMeasurements(TestDesigner &td, float texelSide, SoftwareMeasurementsVector &nyad, RWLock &nyadLock) {
	nyadLock.AcquireWriteLock();
	for (SoftwareMeasurementsVector::const_iterator it = nyad.begin(); it != nyad.end(); it++) {
		const PositionTestMeasuredEntry &ptme = (*it);
		if (ptme.type == TRACKED || ptme.type == NEW_OBJECT)
			allSoftwareMeasurements[ptme.trackingID].push_back(ptme);
	}
	nyad.clear();
	nyadLock.ReleaseWriteLock();
}

bool PositionTest::AssociateSoftwareMeasurementsToUsers(TestDesigner &td, float texelSide) {
	std::vector<std::map<int, std::vector<int>>> allTrackedSoftwareToGround;
	allTrackedSoftwareToGround.resize(size());
	std::vector<std::map<int, double>> pathQuality; //Use to select the best path to associate...fill dividing the error detected for the single points
	pathQuality.resize(size());
	std::vector<std::map<int, int>> numberOfSuitableMeasurements;
	numberOfSuitableMeasurements.resize(size());
	for (unsigned int i = 0; i < size(); i++) {
		for (TrackedMeasurementMap::const_iterator kIt = allSoftwareMeasurements.begin(); kIt != allSoftwareMeasurements.end(); kIt++) {
			Logger::writeToLOG(L"User: %d, Track: %d", i, kIt->first);
			unsigned int z = 0;
			allTrackedSoftwareToGround[i][kIt->first].clear();
			pathQuality[i][kIt->first] = 0.0;
			numberOfSuitableMeasurements[i][kIt->first] = 0;
			for (unsigned int j = 0; j < groundTruthMeasurements[i].size(); j++) {
				if (groundTruthMeasurements[i][j] < kIt->second[0].timestamp) {
					allTrackedSoftwareToGround[i][kIt->first].push_back(-1);
					Logger::writeToLOG(L" -1");
				} else {
					bool found = false;
					for (; z < kIt->second.size() - 1 && !found; z++) {
						DWORD timeDifference1 = (kIt->second[z].timestamp > groundTruthMeasurements[i][j] ? kIt->second[z].timestamp - groundTruthMeasurements[i][j] : groundTruthMeasurements[i][j] - kIt->second[z].timestamp);
						DWORD timeDifference2 = (kIt->second[z+1].timestamp > groundTruthMeasurements[i][j] ? kIt->second[z+1].timestamp - groundTruthMeasurements[i][j] : groundTruthMeasurements[i][j] - kIt->second[z+1].timestamp);
						if (kIt->second[z+1].timestamp > groundTruthMeasurements[i][j] && kIt->second[z].timestamp <= groundTruthMeasurements[i][j]) {
							float candidateXPosition = float(td[(*this)[i]][j].x) / texelSide;
							float candidateYPosition = float(td[(*this)[i]][j].y) / texelSide;
							float candidateXDifference = candidateXPosition - kIt->second[z].x_position;
							float candidateYDifference = candidateYPosition - kIt->second[z].y_position;
							float candidateDistance = (candidateXDifference * candidateXDifference) + (candidateYDifference * candidateYDifference);
							allTrackedSoftwareToGround[i][kIt->first].push_back(z);
							Logger::writeToLOG(L" %d", allTrackedSoftwareToGround[i][kIt->first][j]);
							pathQuality[i][kIt->first]+=candidateDistance;
							numberOfSuitableMeasurements[i][kIt->first]++;
							found = true;
						}
					}
					if (!found) {
						allTrackedSoftwareToGround[i][kIt->first].push_back(-1);
						Logger::writeToLOG(L" -1");
					}
				}
			}
			if (numberOfSuitableMeasurements[i][kIt->first]) {
				pathQuality[i][kIt->first]/=pow(double(numberOfSuitableMeasurements[i][kIt->first]),2.0);
			} else {
				pathQuality[i][kIt->first] = -1.0;
			}
			Logger::writeToLOG(L" Quality: %f, Measurements: %d\r\n", pathQuality[i][kIt->first], numberOfSuitableMeasurements[i][kIt->first]);
		}
	}
	//Associate tracked objects following the quality of the path
	std::map<int, int> trackedObjectToUser;
	for (TrackedMeasurementMap::const_iterator kIt = allSoftwareMeasurements.begin(); kIt != allSoftwareMeasurements.end(); kIt++) {
		int currentlyAssociatedUser =  -1;
		double bestResult = 0.0;
		for (int i = 0; i < size(); i++) {
			if (numberOfSuitableMeasurements[i][kIt->first] > 0 && (currentlyAssociatedUser == -1 || pathQuality[i][kIt->first] < bestResult)) {
				currentlyAssociatedUser = i;
				bestResult = pathQuality[i][kIt->first];
			}
		}
		trackedObjectToUser[kIt->first] = currentlyAssociatedUser;
		Logger::writeToLOG(L"Tracked Object: %d, Best User: %d\r\n", kIt->first, currentlyAssociatedUser);
	}
	//Insert measurement to user ordering for number of measurements
	groundTruthToSoftwareMeasurements.clear();
	groundTruthToSoftwareMeasurements.resize(size());
	numberOfTrackedObjectsAssociated.resize(size());
	for (int i = 0; i < size(); i++) {
		softwareMeasurements[i].clear();
		groundTruthToSoftwareMeasurements[i].clear();
		numberOfTrackedObjectsAssociated[i] = 0;
		int currentOffset = 0;
		DWORD lastTimestampAdded = groundTruthMeasurements[i][0];
		for (unsigned int j = 0; j < groundTruthMeasurements[i].size();) {
			int designedIndex = -1;
			int designedIndexSuitableMeasurements = 0;
			for (TrackedMeasurementMap::const_iterator kIt = allSoftwareMeasurements.begin(); kIt != allSoftwareMeasurements.end(); kIt++) {
				if (trackedObjectToUser[kIt->first] == i && allTrackedSoftwareToGround[i][kIt->first][j] != -1 && (designedIndex == -1 || numberOfSuitableMeasurements[i][kIt->first] > designedIndexSuitableMeasurements)) {
					designedIndex = kIt->first;
					designedIndexSuitableMeasurements = numberOfSuitableMeasurements[i][kIt->first];
				}
			}
			Logger::writeToLOG(L"User %d, Measurement %d, Track %d\r\n", i, j, designedIndex);
			if (designedIndex != -1) {
				trackedObjectToUser[designedIndex] = -1;
				int initialJ = j;
				int currentTrackInitialOffset = allTrackedSoftwareToGround[i][designedIndex][initialJ];
				for (SoftwareMeasurementsVector::const_iterator it = allSoftwareMeasurements[designedIndex].begin() + allTrackedSoftwareToGround[i][designedIndex][initialJ]; currentTrackInitialOffset > 0 && it->timestamp > lastTimestampAdded; it--) {
					currentTrackInitialOffset--;
				}
				for (; j < allTrackedSoftwareToGround[i][designedIndex].size() && allTrackedSoftwareToGround[i][designedIndex][j] != -1; j++) {
					groundTruthToSoftwareMeasurements[i].push_back(currentOffset + (allTrackedSoftwareToGround[i][designedIndex][j] - currentTrackInitialOffset));
				}
				softwareMeasurements[i].insert(softwareMeasurements[i].end(),
					allSoftwareMeasurements[designedIndex].begin() + currentTrackInitialOffset,
					allSoftwareMeasurements[designedIndex].end());
				currentOffset += int(allSoftwareMeasurements[designedIndex].size() - size_t(currentTrackInitialOffset));
				lastTimestampAdded = (softwareMeasurements[i].end() - 1)->timestamp;
				//Every time we have a new association...in this way we can estimate the number of association errors
				numberOfTrackedObjectsAssociated[i]++;
			} else {
				groundTruthToSoftwareMeasurements[i].push_back(-1);
				j++;
			}
		}
		Logger::writeToLOG(L"User: %d, ", i);
		for (int k = 0; k < groundTruthToSoftwareMeasurements[i].size(); k++) {
			Logger::writeToLOG(L"%d ", groundTruthToSoftwareMeasurements[i][k]);
		}
		Logger::writeToLOG(L": %d\r\n", numberOfTrackedObjectsAssociated[i]);
		/*for (int k = 0; k < softwareMeasurements[i].size(); k++) {
			Logger::writeToLOG(L"%d: (%f, %f) ", k, softwareMeasurements[i][k].x_position, softwareMeasurements[i][k].y_position);
		}
		Logger::writeToLOG(L"\r\n");*/
	}
	return true;
}

PositionTest & PositionTest::operator=(const std::vector<int> &other) {
	clear();
	std::vector<int>::operator=(other);
	return *this;
}

void PositionTest::clear() {
	std::vector<int>::clear();
	groundTruthMeasurements.clear();
	softwareMeasurements.clear();
	allSoftwareMeasurements.clear();
	cas.clear();
	testStarted = false;
	strcpy_s(additionalDataDirectory, "");
	ptt = NOT_INITIALIZED_TEST;
}

void PositionTest::toXMLElement(void *xmlElem) const {
	char wideBuffer[256];

	TiXmlElement *elem = (TiXmlElement *) xmlElem;

	if (ptt == OFFLINE_POSITION_TEST) {
		sprintf_s(wideBuffer, "%s", additionalDataDirectory);
		elem->SetAttribute("additionalDataDirectory", wideBuffer);
		elem->SetAttribute("type", "offline");
	} else {
		elem->SetAttribute("type", "online");
	}

	sprintf_s(wideBuffer, 256, "%s", completionDate);
	elem->SetAttribute("completionDate", wideBuffer);

	sprintf_s(wideBuffer, 256, "%s", completionTime);
	elem->SetAttribute("completionTime", wideBuffer);

	for (int i = 0; i < size(); i++) {
		TiXmlElement *userElem = new TiXmlElement("User");

		sprintf_s(wideBuffer, 256, "%d", (*this)[i]);
		userElem->SetAttribute("pathID", wideBuffer);
		sprintf_s(wideBuffer, 256, "%d", usersToSubjects[i]);
		userElem->SetAttribute("personDBaseID", wideBuffer);

		if (ptt == ONLINE_POSITION_TEST || ptt == COMPLETION_POSITION_TEST) {
			TiXmlElement *softwareMeasurementsElem = new TiXmlElement("SoftwareMeasurements");
			for (SoftwareMeasurementsVector::const_iterator it = softwareMeasurements[i].begin(); it != softwareMeasurements[i].end(); it++) {
				TiXmlElement *softwareMeasurementElem = new TiXmlElement("SoftwareMeasurement");
				it->toXMLElement(softwareMeasurementElem);
				softwareMeasurementsElem->LinkEndChild(softwareMeasurementElem);
			}
			userElem->LinkEndChild(softwareMeasurementsElem);
		}

		int z = 0;
		TiXmlElement *groundTruthMeasurementsElem = new TiXmlElement("GroundTruthMeasurements");
		for (GroundTruthMeasurementsVector::const_iterator it = groundTruthMeasurements[i].begin(); it != groundTruthMeasurements[i].end(); it++) {
			TiXmlElement *groundTruthMeasurementElement = new TiXmlElement("GroundTruthMeasure");
			sprintf_s(wideBuffer, 256, "%d", z++);
			groundTruthMeasurementElement->SetAttribute("point", wideBuffer);
			sprintf_s(wideBuffer, 256, "%lu", *it);
			groundTruthMeasurementElement->SetAttribute("timestamp", wideBuffer);
			groundTruthMeasurementsElem->LinkEndChild(groundTruthMeasurementElement);
		}
		userElem->LinkEndChild(groundTruthMeasurementsElem);

		elem->LinkEndChild(userElem);
	}
}

PositionTest::PositionTest(void *xmlElem): testStarted(false) {
	TiXmlElement *testElem = (TiXmlElement *) xmlElem;
	sprintf_s(completionDate, 64, "%s", testElem->Attribute("completionDate"));
	sprintf_s(completionTime, 64, "%s", testElem->Attribute("completionTime"));
	if (_stricmp("offline", testElem->Attribute("type")) == 0) {
		ptt = OFFLINE_POSITION_TEST;
		sprintf_s(additionalDataDirectory, "%s", testElem->Attribute("additionalDataDirectory"));
		cas = CvArrStorage(additionalDataDirectory);
	} else
		ptt = ONLINE_POSITION_TEST;

	resize(0);
	softwareMeasurements.resize(0);
	groundTruthMeasurements.resize(0);
	usersToSubjects.resize(0);
	int actualIndex = 0;
	for (TiXmlElement *userElem = testElem->FirstChildElement("User"); userElem != NULL; userElem = userElem->NextSiblingElement("User"), actualIndex++) {
		resize(actualIndex+1);
		softwareMeasurements.resize(actualIndex+1);
		groundTruthMeasurements.resize(actualIndex+1);
		usersToSubjects.resize(actualIndex+1);
		int pathId; sscanf_s(userElem->Attribute("pathID"), "%d", &pathId);
		(*this)[actualIndex] = pathId;
		int personDBaseId; sscanf_s(userElem->Attribute("personDBaseID"), "%d", &personDBaseId);
		usersToSubjects[actualIndex] = personDBaseId;
		if (ptt == ONLINE_POSITION_TEST) {
			TiXmlElement *softwareMeasurementsListElem = userElem->FirstChildElement("SoftwareMeasurements");
			for (TiXmlElement *softwareMeasurementElement = softwareMeasurementsListElem->FirstChildElement("SoftwareMeasurement"); softwareMeasurementElement != NULL; softwareMeasurementElement = softwareMeasurementElement->NextSiblingElement("SoftwareMeasurement")) {
				softwareMeasurements[actualIndex].push_back(PositionTestMeasuredEntry(softwareMeasurementElement));
			}
		}
		TiXmlElement *groundTruthMeasurementsListElem = userElem->FirstChildElement("GroundTruthMeasurements");
		for (TiXmlElement *groundTruthMeasurementElem = groundTruthMeasurementsListElem->FirstChildElement("GroundTruthMeasure"); groundTruthMeasurementElem != NULL; groundTruthMeasurementElem = groundTruthMeasurementElem->NextSiblingElement("GroundTruthMeasure")) {
			DWORD groundTruthTimeStamp = 0;
			sscanf_s(groundTruthMeasurementElem->Attribute("timestamp"), "%lu", &groundTruthTimeStamp);
			groundTruthMeasurements[actualIndex].push_back(groundTruthTimeStamp);
		}
	}
}

void PositionTestMeasuredEntry::toXMLElement(void *xmlElem) const {
	char wideBuffer[256];
	TiXmlElement *returnElem = (TiXmlElement *) xmlElem;
	sprintf_s(wideBuffer, 256, "%lu", timestamp);
	returnElem->SetAttribute("timestamp", wideBuffer);
	sprintf_s(wideBuffer, 256, "%d", (int) type);
	returnElem->SetAttribute("type", wideBuffer);
	sprintf_s(wideBuffer, 256, "%d", nameID);
	returnElem->SetAttribute("nameID", wideBuffer);
	sprintf_s(wideBuffer, 256, "%d", trackingID);
	returnElem->SetAttribute("trackingID", wideBuffer);

	TiXmlElement *positionElement = new TiXmlElement("Position");
	sprintf_s(wideBuffer, 256, "%f", x_position);
	positionElement->SetAttribute("x", wideBuffer);
	sprintf_s(wideBuffer, 256, "%f", y_position);
	positionElement->SetAttribute("y", wideBuffer);
	returnElem->LinkEndChild(positionElement);

	TiXmlElement *velocityElement = new TiXmlElement("Velocity");
	sprintf_s(wideBuffer, 256, "%f", x_velocity);
	velocityElement->SetAttribute("x", wideBuffer);
	sprintf_s(wideBuffer, 256, "%f", y_velocity);
	velocityElement->SetAttribute("y", wideBuffer);
	returnElem->LinkEndChild(velocityElement);

	TiXmlElement *heightElement = new TiXmlElement("Height");
	sprintf_s(wideBuffer, 256, "%f", max_height);
	heightElement->SetAttribute("max", wideBuffer);
	returnElem->LinkEndChild(heightElement);
}

PositionTestMeasuredEntry::PositionTestMeasuredEntry(void *xmlElem) {
	TiXmlElement *elem = (TiXmlElement *) xmlElem;
	sscanf_s(elem->Attribute("timestamp"), "%lu", &timestamp);
	sscanf_s(elem->Attribute("type"), "%d", &type);
	sscanf_s(elem->Attribute("nameID"), "%d", &nameID);
	sscanf_s(elem->Attribute("trackingID"), "%d", &trackingID);
	
	TiXmlElement * positionElement = elem->FirstChildElement("Position");
	sscanf_s(positionElement->Attribute("x"), "%f", &x_position);
	sscanf_s(positionElement->Attribute("y"), "%f", &y_position);

	TiXmlElement * velocityElement = elem->FirstChildElement("Velocity");
	sscanf_s(velocityElement->Attribute("x"), "%f", &x_velocity);
	sscanf_s(velocityElement->Attribute("y"), "%f", &y_velocity);

	TiXmlElement * heightElement = elem->FirstChildElement("Height");
	sscanf_s(heightElement->Attribute("max"), "%f", &max_height);
}

void PositionTest::StartNewTest(std::vector<int> *usersToSubjects) {
	if (testStarted || ptt == NOT_INITIALIZED_TEST)
		return;
	if (ptt != COMPLETION_POSITION_TEST) {
		groundTruthMeasurements.clear();
		groundTruthMeasurements.resize(size());
		this->usersToSubjects = *usersToSubjects;
	}
	softwareMeasurements.clear();
	softwareMeasurements.resize(size());
	allSoftwareMeasurements.clear();

	for (int i = 0; i < int(size()); i++) {
		if (ptt != COMPLETION_POSITION_TEST)
			groundTruthMeasurements[i].reserve(100);
		softwareMeasurements[i].reserve(10000);
		//writeToConsole(L"User %d gtm %d sm %d\r\n", i, (int) groundTruthMeasurements[i].capacity(), (int) softwareMeasurements[i].capacity());
	}
	startMillisecond = GetTickCount();
	testStarted = true;
}

void PositionTest::StopTest(TestDesigner &td, float texelSide) {
	if (!testStarted)
		return;
	testStarted = false;

	SYSTEMTIME experimentTime;
	GetLocalTime(&experimentTime);
	sprintf_s(completionDate, 64, "%u-%u-%u", experimentTime.wDay, experimentTime.wMonth, experimentTime.wYear);
	sprintf_s(completionTime, 64, "%u-%u-%u-%u", experimentTime.wHour, experimentTime.wMinute,
		experimentTime.wSecond, experimentTime.wMilliseconds);

	if (ptt == ONLINE_POSITION_TEST || ptt == COMPLETION_POSITION_TEST) {
		AssociateSoftwareMeasurementsToUsers(td, texelSide);
	}
}
/*
void SelectExperiment(HWND pathList, PositionTest *testToShow, std::unordered_map<int, int> *pathIDToImageID, bool createColumn, bool selectAll) {
	wchar_t wideBuffer[256];
	ListView_DeleteAllItems(pathList);

	if (createColumn) {
		while (ListView_DeleteColumn(pathList, 0)) {}
		LVCOLUMN lc;
		lc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lc.fmt = LVCFMT_LEFT;

		lc.cx = 40;
		lc.pszText = L"User";
		lc.iSubItem = 0;
		ListView_InsertColumn(pathList, 0, &lc);

		lc.cx = 80;
		lc.pszText = L"Subject";
		lc.iSubItem = 1;
		ListView_InsertColumn(pathList, 1, &lc);

		lc.cx = 80;
		lc.pszText = L"Path";
		lc.iSubItem = 2;
		ListView_InsertColumn(pathList, 2, &lc);
	}

	if (testToShow) {
		int userIndex = 0;
		for (PositionTest::const_iterator it = testToShow->begin(); it != testToShow->end(); it++, userIndex++) {
			LVITEM li;
			if (pathIDToImageID) {
				li.mask = LVIF_TEXT | LVIF_IMAGE;
				li.iImage = (*pathIDToImageID)[*it];
			} else {
				li.mask = LVIF_TEXT;
			}
			li.state = 0; li.stateMask = 0;
			li.pszText = wideBuffer;
			li.cchTextMax = 256;
			li.iSubItem = 0;

			li.iItem = userIndex;

			swprintf_s(wideBuffer, 20, L"%d", userIndex + 1);
			ListView_InsertItem(pathList, &li);

			li.mask = LVIF_TEXT;

			swprintf_s(wideBuffer, 256, L"%S", mainFaceDatabase[testToShow->GetUserToSubjectCorrespondence(userIndex +1)].name);
			li.iSubItem = 1;
			ListView_SetItem(pathList, &li);

			swprintf_s(wideBuffer, 256, L"%S", mainTestDesigner[*it].pathName);
			li.iSubItem = 2;
			ListView_SetItem(pathList, &li);

			if (selectAll)
				ListView_SetItemState(pathList, userIndex, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
		}
	}
}
*/