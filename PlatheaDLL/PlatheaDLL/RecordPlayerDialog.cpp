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

static enum InternalStatusEnum {RECORDER_STANDBY, RECORDER_EXECUTION} status = RECORDER_STANDBY;

extern SystemInfo *si;

static bool CheckSelectedDirectoryValidity(char *directory) {
	WIN32_FIND_DATAA wfd;
	HANDLE hFile = FindFirstFileA(directory, &wfd);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;
	FindClose(hFile);
	char * filenames[] = {"left.avi", "right.avi"};
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

static INT_PTR CALLBACK RecordPlayerDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			cas.clear();
			//recordPlayerDialogHwnd = hwndDlg;
			if (prm == PLaTHEA_RECORDER_MODE) {
				SetWindowText(hwndDlg, L"PLaTHEA Record and Play [RECORDER]");
				EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON3), FALSE);
				SetDlgItemText(hwndDlg, IDC_EDIT1, L"Ready to Record");
			} else {
				SetWindowText(hwndDlg, L"PLaTHEA Record and Play [PLAYBACK]");
				SetDlgItemText(hwndDlg, IDC_BUTTON3, L"Load");
				if (CheckSelectedDirectoryValidity(selectedDirectory)) {
					SetDlgItemTextA(hwndDlg, IDC_EDIT1, selectedDirectory);
				} else {
					SetDlgItemText(hwndDlg, IDC_EDIT1, L"No selected directory");
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON1), FALSE);
				}
				if (si->GetElaborationCore() && si->GetElaborationCore()->IsRunning())
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON4), TRUE);
			}
		}
		return TRUE;
	case WM_COMMAND:
		{
			WORD messageId = HIWORD(wParam);
			WORD controlId = LOWORD(wParam);
			if (controlId == IDC_BUTTON1) {
				if (!si->GetElaborationCore() || !si->GetElaborationCore()->IsRunning())
					MessageBox(hwndDlg, L"Elaboration is not yet started.", L"Warning!!!", MB_OK| MB_ICONWARNING);
				else {
					cas.clear();
					if (prm == PLaTHEA_RECORDER_MODE) {
						cas.clear();
						si->GetElaborationCore()->SaveCurrentState(cas);
					} else if (prm == PLaTHEA_PLAYER_MODE) {
						cas = CvArrStorage(selectedDirectory);
						si->GetElaborationCore()->LoadSavedState(cas);
					}
					if (si->GetElaborationCore()->DuringInitializationPhase())
						si->GetElaborationCore()->EndInitializationPhase();
				}
				if (prm == PLaTHEA_RECORDER_MODE) {
					si->GetStereoRig()->StartRecorderMode();
					SetDlgItemTextA(hwndDlg, IDC_EDIT1, "Recording Started");
				} else if (prm == PLaTHEA_PLAYER_MODE) {
					si->GetStereoRig()->StartPlaybackMode(selectedDirectory, VideoPlaybackStarted());
					SetDlgItemTextA(hwndDlg, IDC_EDIT1, "Playback Started");
				}
				EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON1), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON2), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON3), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON4), FALSE);
				status = RECORDER_EXECUTION;
			} else if (controlId == IDC_BUTTON2) {
				HCURSOR defaultCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
				if (prm == PLaTHEA_RECORDER_MODE)
					si->GetStereoRig()->StopRecorderMode();
				else if (prm == PLaTHEA_PLAYER_MODE) {
					si->GetStereoRig()->StopPlaybackMode();
					VideoPlaybackStop();
				}
				SetDlgItemTextA(hwndDlg, IDC_EDIT1, selectedDirectory);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON1), TRUE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON2), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON3), TRUE);
				if (si->GetElaborationCore() && si->GetElaborationCore()->IsRunning())
					EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON4), TRUE);
				status = RECORDER_STANDBY;
				SetCursor(defaultCursor);
			} else if (controlId == IDC_BUTTON3) {
				if (UseCommonItemDialog(selectedDirectory, sizeof(selectedDirectory), hwndDlg, CLSID_FileOpenDialog, FOS_PICKFOLDERS)) {
					if (prm == PLaTHEA_RECORDER_MODE) {
						si->GetStereoRig()->SaveRecordingToDirectory(selectedDirectory);
						cas.saveToDir(selectedDirectory);
					} else if (prm == PLaTHEA_PLAYER_MODE) {
						EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON1), TRUE);
					}
					SetDlgItemTextA(hwndDlg, IDC_EDIT1, selectedDirectory);
				}
			} else if (controlId == IDC_BUTTON4) {
				bool learningController = (MessageBox(hwndDlg, L"Do you want to start a learning controller instead of an accuracy controller?", L"Question", MB_YESNO | MB_ICONQUESTION) == IDYES);
				ShowMLDialog(hwndDlg, !learningController);
			}
		}
		return TRUE;
	case WM_CLOSE:
		if (status == RECORDER_STANDBY) {
			CloseMLDialog();
			CloseRecordPlayerDialog();
		}
		return TRUE;
	}
	return FALSE;
}

void ShowRecordPlayerDialog(HWND parentWindow, PLaTHEARecorderMode mode) {
	if (prm != PLaTHEA_NO_PLAYER)
		return;
	prm = mode;
	recordPlayerDialogHwnd = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_RECORDERDIALOG), parentWindow, RecordPlayerDialogProc);
	ShowWindow(recordPlayerDialogHwnd, SW_SHOW);
}

void CloseRecordPlayerDialog() {
	DestroyWindow(recordPlayerDialogHwnd);
	recordPlayerDialogHwnd = NULL;
	prm = PLaTHEA_NO_PLAYER;
}