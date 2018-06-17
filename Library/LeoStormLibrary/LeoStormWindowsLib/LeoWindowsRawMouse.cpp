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

#include "LeoWindowsRawMouse.h"

RawMouseMessageDispatcher standardRawMouseMessageDispatcher;

RawMouseMessageDispatcher::RawMouseMessageDispatcher() {
	rawMessageStarted = false;
	rawMouseEventListener = NULL;
}

RawMouseMessageDispatcher::~RawMouseMessageDispatcher() {
	if (rawMessageStarted)
		StopRawMessageAnalysis();
}

void RawMouseMessageDispatcher::UpdateMouseList(WPARAM evento, LPARAM mouseHandle) {
	int i = 0; bool founded = false;
	for (; i < int(rdiVector.size()) && !founded; i++) {
		if (rdiVector[i].hMouse == (HANDLE) mouseHandle)
			founded = true;
	}

	if (evento == GIDC_ARRIVAL && !founded) {
		InstalledMouse im;
		im.hMouse = (HANDLE) mouseHandle;

		wchar_t symbolicDeviceName[256];
		UINT bufferSize = sizeof(symbolicDeviceName);
		GetRawInputDeviceInfo(im.hMouse, RIDI_DEVICENAME, symbolicDeviceName, &bufferSize);

		wchar_t *deviceFamily = wcsrchr(symbolicDeviceName, L'\\'); *deviceFamily = L'\0'; deviceFamily++;
		wchar_t *deviceID = wcsstr(deviceFamily, L"#");
		*deviceID = L'\0'; deviceID++;
		wchar_t *deviceNumber = wcsstr(deviceID, L"#");
		*deviceNumber = L'\0'; deviceNumber++;
		wchar_t *deviceGUID = wcsstr(deviceNumber, L"#"); *deviceGUID = L'\0'; deviceGUID++;
		
		wchar_t regKeyString[256]; swprintf_s(regKeyString, 256, L"SYSTEM\\CurrentControlSet\\Enum\\%s\\%s\\%s",
			deviceFamily, deviceID, deviceNumber);
		DWORD deviceNameBufferSize = sizeof(im.mouseName); DWORD regKeyType;
		if (RegGetValue(HKEY_LOCAL_MACHINE, regKeyString, L"DeviceDesc", RRF_RT_REG_SZ, &regKeyType, im.mouseName, &deviceNameBufferSize)==ERROR_SUCCESS) {
			bufferSize = sizeof(RID_DEVICE_INFO);
			GetRawInputDeviceInfo(im.hMouse, RIDI_DEVICEINFO, &im.rdi, &bufferSize);

			wcscpy_s(im.deviceFamily, 64, deviceFamily);
			wcscpy_s(im.deviceID, 64, deviceID);

			im.suppressSystemEvents = false;
			im.leftButtonState = im.rightButtonState = false;

			im.associatedUser = 0; //Zero means no user associated
			im.associatedSubject = -1; //-1 means no subject associated

			if (wcscmp(deviceID, L"RDP_MOU")) { //We ignore Remote Desktop Protocol mouse
				rdiVector.push_back(im);
				if (rawMouseEventListener)
					rawMouseEventListener(GetTickCount(), int(rdiVector.size()) - 1, NEW_MOUSE_CONNECTED);
			}
		}
	} else if (evento == GIDC_REMOVAL && founded) {
		rdiVector.erase(rdiVector.begin() + (i - 1));
		if (rawMouseEventListener)
			rawMouseEventListener(GetTickCount(), i - 1, MOUSE_REMOVED);
	}
}

bool RawMouseMessageDispatcher::StartRawMessageAnalysis(HWND referenceWindow) {
	if (rawMessageStarted)
		return true;

	this->referenceWindow = referenceWindow;

	rdiVector.clear();

	UINT rawInputSize; GetRawInputDeviceList(NULL, &rawInputSize, sizeof(RAWINPUTDEVICELIST));
	RAWINPUTDEVICELIST *deviceList = new RAWINPUTDEVICELIST[rawInputSize];
	GetRawInputDeviceList(deviceList, &rawInputSize, sizeof(RAWINPUTDEVICELIST));
	for (UINT i = 0; i < rawInputSize; i++) {
		if (deviceList[i].dwType == RIM_TYPEMOUSE)
			UpdateMouseList(GIDC_ARRIVAL, (LPARAM) deviceList[i].hDevice);
	}
	delete[] deviceList;

	RAWINPUTDEVICE ridWait[1];
	
	ridWait[0].usUsagePage = 0x01;
	ridWait[0].usUsage = 0x02;
	ridWait[0].dwFlags = RIDEV_DEVNOTIFY;
	ridWait[0].hwndTarget = referenceWindow;

	rawMessageStarted = (RegisterRawInputDevices(ridWait, 1, sizeof(ridWait[0])) == TRUE);
	if (rawMessageStarted && rawMouseEventListener)
		rawMouseEventListener(GetTickCount(), -1, RAW_INPUT_MANAGEMENT_STARTED);
	return rawMessageStarted;
}

bool RawMouseMessageDispatcher::StopRawMessageAnalysis() {
	if (!rawMessageStarted)
		return true;
	RAWINPUTDEVICE ridWait[1];
	
	ridWait[0].usUsagePage = 0x01;
	ridWait[0].usUsage = 0x02;
	ridWait[0].dwFlags = RIDEV_REMOVE;
	ridWait[0].hwndTarget =NULL;

	rawMessageStarted = !RegisterRawInputDevices(ridWait, 1, sizeof(ridWait[0]));
	if (!rawMessageStarted && rawMouseEventListener)
		rawMouseEventListener(GetTickCount(), -1, RAW_INPUT_MANAGEMENT_ENDED);
	return !rawMessageStarted;
}