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

#include "LeoOpenCVCvArrStorage.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <Windows.h>

CvArrStorage::CvArrStorage(const char *dirName) {
	wchar_t searchPath[MAX_PATH];
	char fileToLoad[MAX_PATH];
	swprintf_s(searchPath, L"%S\\*.xml", dirName);
	WIN32_FIND_DATA wfd;
	HANDLE hFile = FindFirstFile(searchPath, &wfd);
	if (hFile != INVALID_HANDLE_VALUE) {
		do {
			sprintf_s(fileToLoad, "%s\\%S", dirName, wfd.cFileName);
			CvArr *loadedArr = (CvArr *) cvLoad(fileToLoad);
			sprintf_s(fileToLoad, "%S", wfd.cFileName);
			*strstr(fileToLoad, ".xml") = '\0';
			(*this)[fileToLoad] = loadedArr;
		} while (FindNextFile(hFile, &wfd));
	}
}

void CvArrStorage::clear() {
	for (CvArrStorage::const_iterator it = begin(); it != end(); it++) {
		CvArr *aux = it->second;
		cvRelease(&aux);
	}
	std::map<std::string, CvArr *>::clear();
}

void CvArrStorage::saveToDir(char *dirName) {
	char fileName[_MAX_PATH];
	for (CvArrStorage::const_iterator it = begin(); it != end(); it++) {
		sprintf_s(fileName, "%s\\%s.xml", dirName, it->first.c_str());
		cvSave(fileName, it->second);
	}
}

void CvArrStorage::copy(const CvArrStorage &other) {
	clear();
	for (CvArrStorage::const_iterator it = other.begin(); it != other.end(); it++) {
		(*this)[it->first.c_str()] = cvClone(it->second);
	}
}

CvArrStorage & CvArrStorage::operator=(const CvArrStorage &other) {
	copy(other);
	return *this;
}

CvArrStorage::CvArrStorage(const CvArrStorage &other) {
	copy(other);
}