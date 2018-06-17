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

#include "LeoWindowsConsole.h"

WindowsConsole *WindowsConsole::instance = NULL;

WindowsConsole::WindowsConsole() {
	HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	AllocConsole();
	SetConsoleCtrlHandler(consoleHandlerRoutine, TRUE);
}

WindowsConsole::~WindowsConsole() {
	SetConsoleCtrlHandler(consoleHandlerRoutine, FALSE);
	FreeConsole();
}

bool WindowsConsole::virtualWriteToLOG(wchar_t *frmt) {
	HANDLE stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD written;
	WriteConsole(stdHandle, frmt, (DWORD) wcslen(frmt), &written, NULL);
	return true;
}

BOOL WINAPI WindowsConsole::consoleHandlerRoutine(DWORD dwCtrlType) {
	delete instance;
	instance = NULL;
	return TRUE;
}

WindowsConsole *WindowsConsole::GetWindowsConsole(wchar_t *consoleTitle, HICON hIcon) {
	if (instance == NULL) {
		instance = new WindowsConsole();
	}
	SetConsoleTitle(consoleTitle);
	if (hIcon != NULL) {
		SendMessage(GetConsoleWindow(), WM_SETICON, ICON_SMALL, (LPARAM) hIcon);
	}
	return instance;
}


