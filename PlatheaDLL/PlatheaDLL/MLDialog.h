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

#ifndef ML_DIALOG_H
#define ML_DIALOG_H

#include "CommonHeader.h"

void ShowMLDialog (HWND parentWindow, bool accuracyController);
bool VideoPlaybackStarted();
bool VideoPlaybackStop();
void CloseMLDialog();
bool TrainingDataFileToMemory(char *fileName, CvMat **trainingDataMatrix, CvMat **trainingClassificationMatrix);

struct TrainingDataFileRecord {
	float colorDifference;
	float positionDifference;
	float angleDifference;
	int classification;
};

#endif //ML_DIALOG_H