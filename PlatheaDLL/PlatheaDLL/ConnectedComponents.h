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

#ifndef CONNECTED_COMPONENTS
#define CONNECTED_COMPONENTS

#include "CommonHeader.h"
#include <LeoSettingsPersistence.h>

class FindConnectedComponents {
private:
	float &perimScale;
	bool &useContourScanner;
	IplImage *connectedComponents;
	CvMemStorage *mem_storage;
public:
	FindConnectedComponents(int height, int width);
	~FindConnectedComponents();
	IplImage *GetConnectedComponents() {
		return connectedComponents;
	}
	CvSeq *UpdateConnectedComponents(const IplImage *rawForeground);
};

#endif //CONNECTED_COMPONENTS