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

#include "Shared.h"
#include "video_output.h"
#include "ElaborationCore.h"

//****** Implementation of SystemInfo ******//
SystemInfo::SystemInfo(HWND statusBar, HWND hwnd, HWND leftPic, HWND rightPic) {
	this->statusBar = statusBar;
	this->mainWnd = hwnd;
	this->vo = new VideoOutput(leftPic, rightPic, NULL, NULL);
	this->asr = NULL;
	this->ec = NULL;
}

SystemInfo::~SystemInfo() {
	delete vo;
}

HWND SystemInfo::GetStatusBar() {
	return statusBar;
}

VideoOutput * SystemInfo::GetVideoOutput() {
	return vo;
}

StereoRig * SystemInfo::GetStereoRig() {
	return asr;
}

void SystemInfo::SetStereoRig(StereoRig *asr) {
	this->asr = asr;
}

void SystemInfo::SetElaborationCore(ElaborationCore *ec) {
	if (this->ec) {
		this->ec->Stop();
		delete this->ec;
	}
	this->ec = ec;
}

ElaborationCore * SystemInfo::GetElaborationCore() {
	return ec;
}

HWND SystemInfo::GetMainWindow() {
	return this->mainWnd;
}

void GetPLaTHEATempPath(wchar_t *pathBuf, int bufSize) {
	GetEnvironmentVariable(L"APPDATA", pathBuf, bufSize);
	swprintf_s(pathBuf, bufSize, L"%s\\PLaTHEA", pathBuf);
}