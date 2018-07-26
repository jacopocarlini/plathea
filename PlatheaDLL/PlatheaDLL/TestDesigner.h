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

#ifndef TEST_DESIGNER_H
#define TEST_DESIGNER_H

#include "CommonHeader.h"

#include <LeoSettingsPersistence.h>

bool ShowTestDesignerWindow(HWND parentWindow);

class TestPath: public std::vector<POINT> {
public:
	COLORREF pathColor;
	char pathName[256];
	TestPath() : pathColor(RGB(0, 255, 0)) {}
	void UpdateImage(IplImage *image);
	void SaveToPS(char *destString, int width, int height);
	void DrawLines(IplImage *image, float normalizationFactor);
	void DrawLinesPS(char *destString, float normalizationFactor);
	void DrawPoints(IplImage *image, float normalizationFactor);
	void DrawPointsPS(char *destString, float normalizationFactor);
	void DrawNumbers(IplImage *image, CvScalar textColor, float normalizationFactor);
	void DrawNumbersPS(char *destString, float normalizationFactor);
	void GetExtremeValues(long &maxX, long &maxY);
};

class TestDesigner: public std::unordered_map<int, TestPath>, public leostorm::settingspersistence::ParameterSetter {
private:
	int currentID;
public:
	TestDesigner(): leostorm::settingspersistence::ParameterSetter("TestDesigner") {currentID = 0;}
	//Special Management functions
	int FindPathIndexByName(char *name);
	int AddPathToDBase(TestPath &t);
	bool RemovePathFromDBase(char *name);
	
	bool LoadFromXMLElement(TiXmlElement *xmlElement);
	bool SaveToXMLElement(TiXmlElement *xmlElement) const;
};

extern TestDesigner mainTestDesigner;

#endif //TEST_DESIGNER_H