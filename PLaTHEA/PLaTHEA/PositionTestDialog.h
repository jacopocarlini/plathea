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

#ifndef POSITION_TEST_DIALOG_H
#define POSITION_TEST_DIALOG_H

#include "CommonHeader.h"
#include "PositionTest.h"

bool ShowPositionTestWindow(HWND parentWindow);
void RegisterNewSoftwareMeasurements(std::vector<TrackedObject *> &data, DWORD measurementTimestamp);

PositionTest *GetCurrentlySelectedTest();

#endif