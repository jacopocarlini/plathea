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

#ifndef LEOSTORM_WINDOWS_OPENCV_H
#define LEOSTORM_WINDOWS_OPENCV_H

#include <LeoWindowsCommonHeader.h>
#include <opencv2/core.hpp>

HBITMAP IplImage2HBITMAP(HWND destWindow, HBITMAP templ, CvArr *img);
bool ConditionalShowImage(char *windowName, CvArr *data);

#endif //LEOSTORM_WINDOWS_OPENCV_H