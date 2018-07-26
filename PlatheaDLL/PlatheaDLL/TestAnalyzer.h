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

#ifndef TEST_ANALYZER_H
#define TEST_ANALYZER_H

#include "CommonHeader.h"
#include "PositionTest.h"

#include <LeoSettingsPersistence.h>

struct StatisticalMeasure {
	std::vector<double> samples;
	double average;
	double stdDev;
	double perc90;
	double min;
	double max;
	int numberOfSamples;
	void addSample(double value);
	void finalize();
	std::string toString();
};

struct PositionTestAnalysisResult {
	enum StatisticsType{PLATHEA_PIVOT_POSITION_ERROR, PLATHEA_PIVOT_TO_PIVOT_ERROR, PLATHEA_CONTINUOUS_HEIGHT_ERROR, PLATHEA_CONTINUOUS_BASEX_ERROR, PLATHEA_CONTINUOUS_BASEY_ERROR, PLATHEA_ASSOCIATION_ERROR, PLATHEA_LACKING_MEASUREMENTS};
	static const std::string StatisticsDescription[7];
	static const int numberOfSupportedStatistics;
	bool errorInAcquiredData;
	StatisticalMeasure statistics[7];
};

class PositionTestVector: public std::unordered_map<std::string, PositionTest>, public leostorm::settingspersistence::ParameterSetter {
public:
	PositionTestVector(): leostorm::settingspersistence::ParameterSetter("PositionExperiments") {}
	PositionTestAnalysisResult CalculateStatisticalResults(std::vector<std::string> &testVector, TestDesigner &testDesigner, float texelSide);
	bool LoadFromXMLElement(TiXmlElement *xmlElement);
	bool SaveToXMLElement(TiXmlElement *xmlElement) const;
};

void FillStoredExperimentList(HWND storedTestList, PositionTestVector *ptv, bool createColumns, int flagMask, TestDesigner &testDesigner);

#endif //TEST_ANALYZER_H