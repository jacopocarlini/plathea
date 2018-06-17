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

#ifndef LEOSTORM_WINDOWS_GDI_H
#define LEOSTORM_WINDOWS_GDI_H

#include "LeoWindowsCommonHeader.h"

inline HBITMAP CreateCompatibleHBITMAP(HWND destWindow, int width, int height, int bpp, int origin);
HBITMAP RawBits2HBITMAP(HWND destWindow, HBITMAP templ, int width, int height, int size, void* pBits);

void DrawRectangle(RECT &ret, HDC dc);
BOOL AdaptiveBitBlt(HDC destDC, int destX, int destY, int destWidth, int destHeight, HDC srcDC, int srcImageWidth, int srcImageHeight);

#endif //LEOSTORM_WINDOWS_GDI_H