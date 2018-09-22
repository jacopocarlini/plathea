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

#ifndef SYNC_TEST
#define SYNC_TEST

#include "CommonHeader.h"
#include <LeoWindowsThread.h>

void OpenSynchronizationTest(EventRaiser *er, HWND staticHandle[2]);

struct SynchronizationTestResults {
	double stereoFrameRate;
	struct {
		double averageDifference, varianceDifference;
		int maxDifference, minDifference;
	} absoluteDifference;
	struct {
		double averageDifferece;
		int maxDifference, minDifference, numberOfDirectTransitions, numberOfHalfTransitions;
	} synchronizationStats;
};

#endif //SYNC_TEST