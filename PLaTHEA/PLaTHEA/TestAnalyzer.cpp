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

#include "CommonHeader.h"

#include "TestAnalyzer.h"
#include "TestDesigner.h"
#include <LeoWindowsGDI.h>
#include "FaceDatabase.h"

#include <LeoLog4CPP.h>
using namespace leostorm::logging;

const int PositionTestAnalysisResult::numberOfSupportedStatistics = 7;
const std::string PositionTestAnalysisResult::StatisticsDescription[7] = {"Pivot Position Error", "Pivot to Pivot Path Distance Error", "Height measurement error",
	"Bounding box x side", "Bounding box y side", "Associated objects per user", "Lacking measurements per user"};

std::string StatisticalMeasure::toString() {
	std::ostringstream buffer;
	buffer << "Average: " << average << std::endl;
	buffer << "Standard Deviation: " << stdDev << std::endl;
	buffer << "Minimum: " << min << std::endl;
	buffer << "Maximum: " << max << std::endl;
	buffer << "90th Percentile : " << perc90 << std::endl;
	return buffer.str();
}

void StatisticalMeasure::addSample(double value) {
	average += value;
	stdDev += (value * value);
	if (numberOfSamples == 0 || max < value)
		max = value;
	if (numberOfSamples == 0 || min > value)
		min = value;
	int newPos = 0;
	for (; newPos < samples.size() && value > samples[newPos]; newPos++) {}
	samples.insert(samples.begin() + newPos, value);
	numberOfSamples++;
}

void StatisticalMeasure::finalize() {
	average = average / numberOfSamples;
	stdDev = sqrt((stdDev / numberOfSamples) - (average * average));
	int index90Perc = (int) floor((numberOfSamples * 0.9) + 0.5);
	perc90 = samples[index90Perc - 1];
}

void FillStoredExperimentList(HWND storedTestList, PositionTestVector *ptv, bool createColumns, int flagMask, TestDesigner &testDesigner) {
	ListView_DeleteAllItems(storedTestList);

	if (createColumns) {
		while (ListView_DeleteColumn(storedTestList, 0)) {}
		LVCOLUMN lc;
		lc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		lc.fmt = LVCFMT_LEFT;

		lc.cx = 120;
		lc.pszText = L"Test Name";
		lc.iSubItem = 0;
		ListView_InsertColumn(storedTestList, 0, &lc);

		lc.cx = 75;
		lc.pszText = L"Date";
		lc.iSubItem = 1;
		ListView_InsertColumn(storedTestList, 1, &lc);

		lc.cx = 80;
		lc.pszText = L"Time";
		lc.iSubItem = 2;
		ListView_InsertColumn(storedTestList, 2, &lc);

		lc.cx = 40;
		lc.pszText = L"NOK";
		lc.iSubItem = 3;
		ListView_InsertColumn(storedTestList, 3, &lc);
	}

	if (ptv) {
		int j = 0;
		for (PositionTestVector::const_iterator it = ptv->begin(); it != ptv->end(); it++, j++) {
			wchar_t wideBuffer[256];

			if (flagMask & it->second.GetPositionTestType()) {

				LVITEM li;
				li.mask = LVIF_TEXT;
				li.state = 0; li.stateMask = 0;
				li.pszText = wideBuffer;
				li.cchTextMax = 256;
				li.iSubItem = 0;

				li.iItem = j;

				swprintf_s(wideBuffer, 256, L"%S", it->first.c_str());
				ListView_InsertItem(storedTestList, &li);

				swprintf_s(wideBuffer, 256, L"%S", it->second.GetCompletionDate());
				li.iSubItem = 1;
				ListView_SetItem(storedTestList, &li);

				swprintf_s(wideBuffer, 256, L"%S", it->second.GetCompletionTime());
				li.iSubItem = 2;
				ListView_SetItem(storedTestList, &li);

				bool nok = false;
				if (it->second.GetPositionTestType() & PositionTest::COMPLETION_POSITION_TEST || it->second.GetPositionTestType() & PositionTest::ONLINE_POSITION_TEST) {
					for (int i = 0; i < it->second.size() && !nok; i++) {
						if (it->second.GetGroundTruthMeasurements(i).size() != testDesigner[it->second[i]].size())
							nok = true;
					}
				}
				swprintf_s(wideBuffer, 256, L"%S", (nok ? "X" : ""));
				li.iSubItem = 3;
				ListView_SetItem(storedTestList, &li);
			}
		}
	}
}

PositionTestAnalysisResult PositionTestVector::CalculateStatisticalResults(std::vector<std::string> &testVector, TestDesigner &testDesigner, float texelSide) {
	PositionTestAnalysisResult result;
	memset(result.statistics, 0, result.numberOfSupportedStatistics * sizeof(StatisticalMeasure));
	for (std::vector<std::string>::const_iterator it = testVector.begin(); it != testVector.end(); it++) {
		PositionTest &positionTest = (*this)[*it];
		std::vector<std::vector<int>> &groundTruthToSoftwareMeasurements = positionTest.groundTruthToSoftwareMeasurements;
		size_t numberOfUsers = positionTest.size();
		for (size_t i = 0; i < numberOfUsers; i++) {
			int numberOfLackingMeasurements = 0;
			int correspondentDatabaseID = positionTest.GetUserToSubjectCorrespondence(int(i) + 1);
			float dbaseIDHeight = float(mainFaceDatabase[correspondentDatabaseID].height) / texelSide;
			float dbaseSubjectWidth = float(mainFaceDatabase[correspondentDatabaseID].shoulders) / texelSide;
			float dbaseSubjectFat = float(mainFaceDatabase[correspondentDatabaseID].chest) / texelSide;

			std::vector<int> &userGroundTruthToSoftwareMeasurements = groundTruthToSoftwareMeasurements[i];
			TestPath &testPath = testDesigner[positionTest[i]];
			const SoftwareMeasurementsVector &userSoftwareMeasurements = positionTest.GetSoftwareMeasurements(int(i));
			size_t numberOfGroundTruthMeasurements = userGroundTruthToSoftwareMeasurements.size();
			bool previousValid = false;
			for (size_t j = 0; j < numberOfGroundTruthMeasurements; j++) {
				int correspondentSoftwareMeasurement = userGroundTruthToSoftwareMeasurements[j];
				if (correspondentSoftwareMeasurement == -1) {
					//TODO: When this appens replace with a SoftwareMeasurementVector fatto a tavolino
					numberOfLackingMeasurements++;
					previousValid = false;
				} else {
					double vectorialDifference = CalculateVectorialDistance(float(testPath[j].x) / texelSide , float(testPath[j].y) / texelSide,
						userSoftwareMeasurements[correspondentSoftwareMeasurement].x_position, userSoftwareMeasurements[correspondentSoftwareMeasurement].y_position);
					result.statistics[PositionTestAnalysisResult::PLATHEA_PIVOT_POSITION_ERROR].addSample(vectorialDifference);
					if (previousValid) {
						double alphaAngle = atan(double(testPath[j].y - testPath[j-1].y) / double(testPath[j].x - testPath[j - 1].x));
						if (alphaAngle > M_PI_2)
							alphaAngle -= M_PI - alphaAngle;
						int previousCorrespondentSoftwareMeasurement = userGroundTruthToSoftwareMeasurements[j - 1];
						double groundTruthDifference = CalculateVectorialDistance(float(testPath[j].x), float(testPath[j].y),
							float(testPath[j - 1].x), float(testPath[j - 1].y)) / texelSide;
						//Logger::writeToLOG(L"GT: (%f, %f) (%f, %f) - ", float(testPath[j].x), float(testPath[j].y),
						//	float(testPath[j - 1].x), float(testPath[j - 1].y));
						double pivotToPivotPath = CalculateVectorialDistance(float(testPath[j-1].x)/texelSide, float(testPath[j-1].y)/texelSide,
							userSoftwareMeasurements[previousCorrespondentSoftwareMeasurement].x_position,
							userSoftwareMeasurements[previousCorrespondentSoftwareMeasurement].y_position);
						for (int k = previousCorrespondentSoftwareMeasurement; k < correspondentSoftwareMeasurement; k++) {
							double heightDifference = abs(userSoftwareMeasurements[k].max_height - dbaseIDHeight);
							result.statistics[PositionTestAnalysisResult::PLATHEA_CONTINUOUS_HEIGHT_ERROR].addSample(heightDifference);

							double predictedXBase = abs(dbaseSubjectWidth*cos(M_PI_2 - alphaAngle))
								+ abs(dbaseSubjectFat*sin(M_PI_2 - alphaAngle));
							double predictedYBase = abs(dbaseSubjectWidth*sin(M_PI_2 - alphaAngle))
								+ abs(dbaseSubjectFat*cos(M_PI_2 - alphaAngle));
							double xBaseDifference = abs(userSoftwareMeasurements[k].x_base - predictedXBase);
							double yBaseDifference = abs(userSoftwareMeasurements[k].y_base - predictedYBase);
							//Logger::writeToLOG(L"%f - %f\r\n", xBaseDifference, yBaseDifference);
							result.statistics[PositionTestAnalysisResult::PLATHEA_CONTINUOUS_BASEX_ERROR].addSample(xBaseDifference);
							result.statistics[PositionTestAnalysisResult::PLATHEA_CONTINUOUS_BASEY_ERROR].addSample(yBaseDifference);

							pivotToPivotPath += CalculateVectorialDistance(userSoftwareMeasurements[k].x_position,
								userSoftwareMeasurements[k].y_position, userSoftwareMeasurements[k + 1].x_position,
								userSoftwareMeasurements[k + 1].y_position);
						}
						pivotToPivotPath += CalculateVectorialDistance(float(testPath[j].x)/texelSide, float(testPath[j].y/texelSide),
							userSoftwareMeasurements[correspondentSoftwareMeasurement].x_position,
							userSoftwareMeasurements[correspondentSoftwareMeasurement].y_position);
						//Logger::writeToLOG(L"%f/%f", pivotToPivotPath, groundTruthDifference);
						pivotToPivotPath /= groundTruthDifference;
						//Logger::writeToLOG(L"=%f\r\n", pivotToPivotPath);

						result.statistics[PositionTestAnalysisResult::PLATHEA_PIVOT_TO_PIVOT_ERROR].addSample(pivotToPivotPath);
					}
					previousValid = true;
				}
			}
			result.statistics[PositionTestAnalysisResult::PLATHEA_LACKING_MEASUREMENTS].addSample(numberOfLackingMeasurements);
			result.statistics[PositionTestAnalysisResult::PLATHEA_ASSOCIATION_ERROR].addSample(positionTest.numberOfTrackedObjectsAssociated[i]);
		}
	}
	for (int i = 0; i < PositionTestAnalysisResult::numberOfSupportedStatistics; i++) {
		result.statistics[i].finalize();
	}
	result.errorInAcquiredData = false;
	return result;
}

bool PositionTestVector::LoadFromXMLElement(TiXmlElement *sourceElem) {
	char buffer[256];
	clear();
	for (TiXmlElement *childElem = sourceElem->FirstChildElement("PositionExperiment"); childElem != NULL; childElem = childElem->NextSiblingElement("PositionExperiment")) {
		sprintf_s(buffer, 256, "%s", childElem->Attribute("symbolicName"));
		PositionTest pt(childElem);
		(*this)[buffer] = pt;
	}
	return true;
}

bool PositionTestVector::SaveToXMLElement(TiXmlElement *destElem) const{
	char wideBuf[256];
	for (PositionTestVector::const_iterator it = begin(); it != end(); it++) {
		TiXmlElement *childElem = new TiXmlElement("PositionExperiment");
		sprintf_s(wideBuf, 256, "%s", it->first.c_str());
		childElem->SetAttribute("symbolicName", wideBuf);
		it->second.toXMLElement(childElem);
		destElem->LinkEndChild(childElem);
	}
	return true;
}