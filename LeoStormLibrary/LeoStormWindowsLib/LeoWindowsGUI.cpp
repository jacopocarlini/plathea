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

#include "LeoWindowsGUI.h"

bool UseCommonItemDialog(char *buffer, int bufferSize, HWND hwndParent, CLSID type, DWORD options,
						 COMDLG_FILTERSPEC *filtro, int filterItems) {
	IFileDialog *ifd;
	HRESULT hr = CoCreateInstance(type, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&ifd));
	if (SUCCEEDED(hr)) {
		DWORD currOptions; ifd->GetOptions(&currOptions);
		ifd->SetOptions(currOptions | options | FOS_FORCEFILESYSTEM);
		if (filtro) {
			ifd->SetFileTypes(filterItems, filtro);
			ifd->SetDefaultExtension(filtro[0].pszSpec);
		}
		hr = ifd->Show(hwndParent);
		if (SUCCEEDED(hr)) {
			// Obtain the result of the user's interaction with the dialog.
			IShellItem *psiResult;
			hr = ifd->GetResult(&psiResult);
			if (SUCCEEDED(hr))
			{
				wchar_t *buf;
				hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &buf);
				if (SUCCEEDED(hr)) {
					size_t numberOfConverted;
					wcstombs_s(&numberOfConverted, buffer, bufferSize, buf, _TRUNCATE);
					CoTaskMemFree(buf);
				}
				psiResult->Release();
			}
		}
		ifd->Release();
	}
	return SUCCEEDED(hr);
}

bool UseCommonColorDialog(HWND window, COLORREF &color) {
	CHOOSECOLOR cc;                 // common dialog box structure 
	static COLORREF acrCustClr[16]; // array of custom colors 

	// Initialize CHOOSECOLOR 
	ZeroMemory(&cc, sizeof(cc));
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = window;
	cc.lpCustColors = (LPDWORD) acrCustClr;
	cc.rgbResult = color;
	cc.Flags = CC_FULLOPEN | CC_RGBINIT;

	if (ChooseColor(&cc)==TRUE) {
		color = cc.rgbResult;
		return true;
	} else {
		return false;
	}
}

void GetListViewSelectedIndexes(HWND listView, std::vector<int> &vec) {
	vec.clear();
	int nextSelectedIndex = ListView_GetNextItem(listView, -1, LVNI_SELECTED);
	while (nextSelectedIndex != -1) {
		vec.push_back(nextSelectedIndex);
		nextSelectedIndex = ListView_GetNextItem(listView, nextSelectedIndex, LVNI_SELECTED);
	}
}

void PerformClipCursor(HWND windowToClip, bool clientArea) {
	RECT windowRect;
	if (clientArea) {
		GetWindowRect(windowToClip, &windowRect);
		ClientToScreen(windowToClip, (LPPOINT) &windowRect.left);
		ClientToScreen(windowToClip, (LPPOINT) &windowRect.right);
	} else {
		GetWindowRect(windowToClip, &windowRect);
	}
	ClipCursor(&windowRect);
}

BOOL RedrawStaticControl(HWND* control, int numberOfControls, bool erase, bool updateNow) {
	if (numberOfControls < 1)
		return FALSE;
	HWND parentWindow = GetParent(control[0]);
	HRGN baseRegion = CreateRectRgn(0, 0, 0, 0);
	for (int i = 0; i < numberOfControls; i++) {
		RECT result; GetClientRect(control[i], &result);
		MapWindowPoints(control[i], parentWindow, (POINT *) &result, 2);
		HRGN staticControlRegion = CreateRectRgnIndirect(&result);
		CombineRgn(baseRegion, baseRegion, staticControlRegion, RGN_OR);
		DeleteObject(staticControlRegion);
	}
	return RedrawWindow(parentWindow, NULL, baseRegion, RDW_INVALIDATE | (erase ? RDW_ERASE : 0) | (updateNow ? RDW_UPDATENOW : 0));
}
