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

#include "AcquisitionStatistics.h"
#include "NetworkCamera.h"
#include "Shared.h"

extern SystemInfo *si;

static HWND acquisitionStatHwnd = NULL;

float _leftRate=0;
float _rightRate=0;
float _stereoRate=0;

static void LeftRateUpdate(float leftRate) {
	//wchar_t c_buffer[64];
	//swprintf_s(c_buffer, 64, L"%.2f fps", (double) leftRate);
	//SetDlgItemText(acquisitionStatHwnd, IDC_LEFTFRAMESTATIC, c_buffer);
	_leftRate = leftRate;
}

static void RightRateUpdate(float rightRate) {
	wchar_t c_buffer[64];
	swprintf_s(c_buffer, 64, L"%.2f fps", (double) rightRate);
	//SetDlgItemText(acquisitionStatHwnd, IDC_RIGHTFRAMESTATIC, c_buffer);
	_rightRate = rightRate;
}

static void StereoRateUpdate(float stereoRate) {
	//wchar_t c_buffer[64];
	//swprintf_s(c_buffer, 64, L"%.2f fps", (double) stereoRate);
	//SetDlgItemText(acquisitionStatHwnd, IDC_STEREOFRAMESTATIC, c_buffer);
	_stereoRate = stereoRate;
}

/*
static INT_PTR CALLBACK AcquisitionStatisticProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			if (si->GetStereoRig()) {
				si->GetStereoRig()->notifyFunction = StereoRateUpdate;
				si->GetStereoRig()->GetAcquisitionCamera(LEFT_SIDE_CAMERA)->notifyFunction = LeftRateUpdate;
				si->GetStereoRig()->GetAcquisitionCamera(RIGHT_SIDE_CAMERA)->notifyFunction = RightRateUpdate;
			}
		}
		return TRUE;
	case WM_CLOSE:
		{
			if (si->GetStereoRig()) {
				si->GetStereoRig()->notifyFunction = NULL;
				si->GetStereoRig()->GetAcquisitionCamera(LEFT_SIDE_CAMERA)->notifyFunction = NULL;
				si->GetStereoRig()->GetAcquisitionCamera(RIGHT_SIDE_CAMERA)->notifyFunction = NULL;
			}
			DestroyWindow(hwndDlg);
			acquisitionStatHwnd = NULL;
		}
		return TRUE;
	}
	return FALSE;
}

void OpenAcquisitionStatisticWindow(HWND parentHwnd) {
	if (acquisitionStatHwnd == NULL) {
		//acquisitionStatHwnd = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ACQUISITIONSTATISTIC), parentHwnd, AcquisitionStatisticProc);
		ShowWindow(acquisitionStatHwnd, SW_SHOW);
	} else {
		SetForegroundWindow(acquisitionStatHwnd);
	}
}

*/


void StartAcquisitionStatisticProc() {
	if (si->GetStereoRig()) {
		si->GetStereoRig()->notifyFunction = StereoRateUpdate;
		si->GetStereoRig()->GetAcquisitionCamera(LEFT_SIDE_CAMERA)->notifyFunction = LeftRateUpdate;
		si->GetStereoRig()->GetAcquisitionCamera(RIGHT_SIDE_CAMERA)->notifyFunction = RightRateUpdate;
	}
}

void StopAcquisitionStatisticProc() {
	if (si->GetStereoRig()) {
		si->GetStereoRig()->notifyFunction = NULL;
		si->GetStereoRig()->GetAcquisitionCamera(LEFT_SIDE_CAMERA)->notifyFunction = NULL;
		si->GetStereoRig()->GetAcquisitionCamera(RIGHT_SIDE_CAMERA)->notifyFunction = NULL;
	}
}

// return float array: { _leftRate , _rightRate, _stereoRate};
float* GetAcquisitionStatisticProc() {
	float ret[3] = { _leftRate , _rightRate, _stereoRate};
	return ret;
}