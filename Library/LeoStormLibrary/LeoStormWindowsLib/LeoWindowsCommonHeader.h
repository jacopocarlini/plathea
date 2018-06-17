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

#ifndef LEOSTORM_WINDOWS_COMMON_HEADER_H
#define LEOSTORM_WINDOWS_COMMON_HEADER_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <WindowsX.h>
#include <CommCtrl.h>
#include <process.h>

//Some common useful macros
#define chDIMOF(Array) (sizeof(Array) / sizeof(Array[0]))

typedef unsigned (__stdcall *PTHREAD_START) (void *);

#define chBEGINTHREADEX(lpsa, cbStack, lpStartAddr, \
   lpvThreadParm, fdwCreate, lpIDThread)            \
      ((HANDLE)_beginthreadex(                      \
         (void *) (lpsa),                           \
         (unsigned) (cbStack),                      \
         (PTHREAD_START) (lpStartAddr),             \
         (void *) (lpvThreadParm),                  \
         (unsigned) (fdwCreate),                    \
         (unsigned *) (lpIDThread)))


#define chHANDLE_DLGMSG(hwnd, message, fn)                           \
   case (message): return (SetDlgMsgResult(hwnd, uMsg,               \
      HANDLE_##message((hwnd), (wParam), (lParam), (fn))))

#endif //LEOSTORM_WINDOWS_COMMON_HEADER_H