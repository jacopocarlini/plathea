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

#ifndef LEOSTORM_WINDOWS_GUI_H
#define LEOSTORM_WINDOWS_GUI_H

#include "LeoWindowsCommonHeader.h"

#include <shlobj.h>
#include <CommDlg.h>

#include <vector>

bool UseCommonItemDialog(char *buffer, int bufferSize, HWND hwndParent, CLSID type, DWORD options,
						 COMDLG_FILTERSPEC *filtro = NULL, int filterItems = 0);

bool UseCommonColorDialog(HWND hwndParent, COLORREF &color);

void GetListViewSelectedIndexes(HWND listView, std::vector<int> &vec);

void PerformClipCursor(HWND windowToClip, bool clientArea);

BOOL RedrawStaticControl(HWND* control, int numberOfControls, bool erase = true, bool updateNow = true);

#endif //LEOSTORM_WINDOWS_GUI_H