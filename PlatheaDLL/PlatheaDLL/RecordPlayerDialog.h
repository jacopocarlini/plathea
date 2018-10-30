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

#ifndef RECORD_PLAYER_DIALOG_H
#define RECORD_PLAYER_DIALOG_H

#include "CommonHeader.h"
#include <string>  

enum PLaTHEARecorderMode {PLaTHEA_RECORDER_MODE, PLaTHEA_PLAYER_MODE, PLaTHEA_NO_PLAYER};

void ShowRecordPlayerDialog(HWND parentWindow, PLaTHEARecorderMode mode);
void CloseRecordPlayerDialog();
void StartPlayer(const char dir[260]);
void StopPlayer();

#endif //RECORD_PLAYER_DIALOG_H