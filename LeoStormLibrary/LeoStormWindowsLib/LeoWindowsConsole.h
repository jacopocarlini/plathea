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

#ifndef LEOSTORM_WINDOWS_CONSOLE_H
#define LEOSTORM_WINDOWS_CONSOLE_H

#include "LeoWindowsCommonHeader.h"
#include "LeoLog4CPP.h"

class WindowsConsole: public leostorm::logging::Logger {
private:
	static BOOL WINAPI consoleHandlerRoutine(DWORD dwCtrlType);
	static WindowsConsole *instance;
	WindowsConsole();
public:
	static WindowsConsole *GetWindowsConsole(wchar_t *consoleTitle, HICON hIcon = NULL);
	~WindowsConsole();
	bool virtualWriteToLOG(wchar_t *frmt);
};

#endif //LEOSTORM_WINDOWS_CONSOLE_H