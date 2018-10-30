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

#include "RecordPlayerDialog.h"
#include "Shared.h"
#include "ElaborationCore.h"
#include "PlanViewMap.h"
#include "MLDialog.h"

#include <LeoWindowsGUI.h>

static char selectedDirectory[_MAX_PATH] = "";
static CvArrStorage cas;

static PLaTHEARecorderMode prm = PLaTHEA_NO_PLAYER;
static HWND recordPlayerDialogHwnd = NULL;

static enum InternalStatusEnum { RECORDER_STANDBY, RECORDER_EXECUTION };
InternalStatusEnum status = RECORDER_STANDBY;

extern SystemInfo *si;

static bool CheckSelectedDirectoryValidity(char *directory) {
	WIN32_FIND_DATAA wfd;
	HANDLE hFile = FindFirstFileA(directory, &wfd);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;
	FindClose(hFile);
	const char * filenames[] = {"left.avi", "right.avi"};
	char buf[_MAX_PATH];
	for (int i = 0; i < 2; i++) {
		sprintf_s(buf, _MAX_PATH, "%s\\%s", directory, filenames[i]);
		hFile = FindFirstFileA(buf, &wfd);
		if (hFile == INVALID_HANDLE_VALUE)
			return false;
		FindClose(hFile);
	}
	return true;
}


void ShowRecordPlayerDialog(HWND parentWindow, PLaTHEARecorderMode mode) {
	
	if (prm != PLaTHEA_NO_PLAYER)
		return;
	prm = mode;
	//recordPlayerDialogHwnd = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_RECORDERDIALOG), parentWindow, RecordPlayerDialogProc);
	//ShowWindow(recordPlayerDialogHwnd, SW_SHOW);
	cas.clear();
	//printf("Debug\n");
}


void StartPlayer(const char dir[260]) {
	for (int i = 0; i < 260; i++) {
		selectedDirectory[i] = dir[i];
	}
	if (CheckSelectedDirectoryValidity(selectedDirectory)) {
		//printf("player");
		cas.clear();
		cas = CvArrStorage(selectedDirectory);
		//printf("elaboration core:  %d\n",si->GetElaborationCore());
		si->GetElaborationCore()->LoadSavedState(cas);

		//if (si->GetElaborationCore()->DuringInitializationPhase())
			//si->GetElaborationCore()->EndInitializationPhase();

		si->GetStereoRig()->StartPlaybackMode(selectedDirectory, VideoPlaybackStarted());
		status = RECORDER_EXECUTION;
	}
}

void StopPlayer() {
	si->GetStereoRig()->StopPlaybackMode();
	VideoPlaybackStop();
	status = RECORDER_STANDBY;
}

void CloseRecordPlayerDialog() {
	DestroyWindow(recordPlayerDialogHwnd);
	recordPlayerDialogHwnd = NULL;
	prm = PLaTHEA_NO_PLAYER;
}