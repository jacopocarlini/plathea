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

#ifndef LEOSTORM_WINDOWS_RAWMOUSE_H
#define LEOSTORM_WINDOWS_RAWMOUSE_H

#include "LeoWindowsCommonHeader.h"
#include <vector>

enum RawMouseEvents {NEW_MOUSE_CONNECTED, MOUSE_REMOVED, RAW_INPUT_MANAGEMENT_STARTED, RAW_INPUT_MANAGEMENT_ENDED,
	RAW_MOUSE_LEFT_CLICK, RAW_MOUSE_RIGHT_CLICK, RAW_MOUSE_LEFT_BUTTON_DOWN, RAW_MOUSE_LEFT_BUTTON_UP,
	RAW_MOUSE_RIGHT_BUTTON_DOWN, RAW_MOUSE_RIGHT_BUTTON_UP};

typedef void (*RawMouseEventCallback) (DWORD, int, UINT);

struct InstalledMouse {
	HANDLE hMouse;
	wchar_t mouseName[256];
	wchar_t deviceID[64];
	wchar_t deviceFamily[64];
	RID_DEVICE_INFO rdi;
	bool suppressSystemEvents;
	bool leftButtonState;
	bool rightButtonState;
	int associatedUser;
	int associatedSubject;
};

class RawMouseMessageDispatcher {
private:
	std::vector<InstalledMouse> rdiVector;
	bool rawMessageStarted;
	HWND referenceWindow;
public:
	RawMouseMessageDispatcher();
	~RawMouseMessageDispatcher();
	RawMouseEventCallback rawMouseEventListener;
	void UpdateMouseList(WPARAM evento, LPARAM mouseHandle);
	bool StartRawMessageAnalysis(HWND referenceWindow);
	bool StopRawMessageAnalysis();
	bool IsRawMessageAnalysisStarted() {
		return rawMessageStarted;
	}
	std::vector<InstalledMouse> &GetInstalledMice() {
		return rdiVector;
	}
	int GetMouseIndexByUser(int userId) {
		int i = 0;
		for (std::vector<InstalledMouse>::const_iterator it = rdiVector.begin(); it != rdiVector.end(); it++, i++) {
			if (it->associatedUser == userId)
				return i;
		}
		return -1;
	}
	int GetMouseIndexBySubject(int subjectId) {
		int i = 0;
		for (std::vector<InstalledMouse>::const_iterator it = rdiVector.begin(); it != rdiVector.end(); it++, i++) {
			if (it->associatedSubject == subjectId)
				return i;
		}
		return -1;
	}
	int GetSuppressedMouseCount() {
		int result = 0;
		for (std::vector<InstalledMouse>::const_iterator it = rdiVector.begin(); it != rdiVector.end(); it++) {
			if (it->suppressSystemEvents)
				result++;
		}
		return result;
	}
	bool ReplacedWindowsInnerLoop() {
		MSG msg;

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_INPUT) {
				UINT dwSize;

				GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));

				LPBYTE lpb = new BYTE[dwSize]; 

				GetRawInputData((HRAWINPUT) msg.lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

				RAWINPUT *raw = (RAWINPUT *) lpb;

				if (raw->header.dwType == RIM_TYPEMOUSE) {
					std::vector<InstalledMouse>::const_iterator it = rdiVector.begin();
					int mouseIndex = -1;
					for (int i = 0; it != rdiVector.end(); i++, it++) {
						if (it->hMouse == raw->header.hDevice) {
							mouseIndex = i;
							break;
						}
					}
					if (mouseIndex != -1) {
						int numberOfEvents = -1; UINT *eventsVector = NULL;
						if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) {
							rdiVector[mouseIndex].leftButtonState = true;
							numberOfEvents = 4; eventsVector = new UINT[4];
							eventsVector[0] = WM_LBUTTONDOWN; eventsVector[1] = WM_NCLBUTTONDOWN;
							eventsVector[2] = WM_LBUTTONDBLCLK; eventsVector[3] = WM_NCLBUTTONDBLCLK;
							if (rawMouseEventListener)
								rawMouseEventListener(GetTickCount(), mouseIndex, RAW_MOUSE_LEFT_BUTTON_DOWN);
						} else if (raw->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) {
							if (rdiVector[mouseIndex].leftButtonState && rawMouseEventListener)
								rawMouseEventListener(GetTickCount(), mouseIndex, RAW_MOUSE_LEFT_CLICK);
							rdiVector[mouseIndex].leftButtonState = false;
							numberOfEvents = 2; eventsVector = new UINT[2];
							eventsVector[0] = WM_LBUTTONUP; eventsVector[1] = WM_NCLBUTTONUP;
							if (rawMouseEventListener)
								rawMouseEventListener(GetTickCount(), mouseIndex, RAW_MOUSE_LEFT_BUTTON_UP);
						}
						if (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN) {
							rdiVector[mouseIndex].rightButtonState = true;
							numberOfEvents = 4; eventsVector = new UINT[4];
							eventsVector[0] = WM_RBUTTONDOWN; eventsVector[1] = WM_NCRBUTTONDOWN;
							eventsVector[2] = WM_RBUTTONDBLCLK; eventsVector[3] = WM_NCRBUTTONDBLCLK;
							if (rawMouseEventListener)
								rawMouseEventListener(GetTickCount(), mouseIndex, RAW_MOUSE_RIGHT_BUTTON_DOWN);
						} else if (raw->data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP) {
							if (rdiVector[mouseIndex].rightButtonState && rawMouseEventListener)
								rawMouseEventListener(GetTickCount(), mouseIndex, RAW_MOUSE_RIGHT_CLICK);
							rdiVector[mouseIndex].rightButtonState = false;
							numberOfEvents = 2; eventsVector = new UINT[2];
							eventsVector[0] = WM_RBUTTONUP; eventsVector[1] = WM_NCRBUTTONUP;
							if (rawMouseEventListener)
								rawMouseEventListener(GetTickCount(), mouseIndex, RAW_MOUSE_RIGHT_BUTTON_UP);
						}
						if (rdiVector[mouseIndex].suppressSystemEvents && eventsVector) {
							bool stopLooping = false;
							while (!stopLooping) {
								for (int i = 0; i < numberOfEvents; i++) {
									stopLooping = (stopLooping || PeekMessage(&msg, NULL, eventsVector[i], eventsVector[i], PM_QS_INPUT | PM_REMOVE));
								}
							}
							delete[] eventsVector;
						}
					}
				}

				delete[] lpb;
			} else if (msg.message == WM_QUIT) {
				return false;
			} else if (msg.message == WM_INPUT_DEVICE_CHANGE) {
				UpdateMouseList(msg.wParam, msg.lParam);
			} else {
				/* Here we suppose to have a sole window in our thread to control...in order to extend it we should
					manage a modeless dialog window list and if necessary call IsDialogMessage */
				if (!referenceWindow || !IsDialogMessage(referenceWindow, &msg)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}
		return true;
	}
};

extern RawMouseMessageDispatcher standardRawMouseMessageDispatcher;

#endif //LEOSTORM_WINDOWS_RAWMOUSE_H