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

#include "LeoStormSTLExtension.h"

//**** STL C++ Library doesn't implement the well known trim function for std::string, we do *****//
void trim(std::string& str) {
	std::string::size_type pos = str.find_last_not_of(' ');
	if(pos != std::string::npos) {
		str.erase(pos + 1);
		pos = str.find_first_not_of(' ');
		if(pos != std::string::npos) str.erase(0, pos);
	} else {
		str.erase(str.begin(), str.end());
	}
}

void trim(char *str) {
	int lunghezza = (int) strlen(str);
	if (lunghezza == 0)
		return;
	int firstNonSpace = -1, lastNonSpace = -1;
	for (int i = 0; i < lunghezza; i++) {
		if (str[i] != ' ' && str[i] != '\t') {
			firstNonSpace = i;
			break;
		}
	}
	if (firstNonSpace == -1) {
		strcpy_s(str, 1, "");
		return;
	}
	for (int i = lunghezza - 1; i >= 0; i--) {
		if (str[i] != ' ' && str[i] != '\t') {
			lastNonSpace = i;
			break;
		}
	}
	int i, j;
	for (i = 0, j = firstNonSpace; j <= lastNonSpace; i++, j++) {
		str[i] = str[j];
	}
	str[i] = '\0';
}

bool ToRelativePath(wchar_t *configurationFile, wchar_t *toConvertFile, wchar_t *outputFile, int bufSize) {
	wchar_t relativePath[1024];
	wchar_t *slashChar = wcsrchr(configurationFile, L'\\');
	wcscpy_s(relativePath, configurationFile);
	relativePath[slashChar-configurationFile] = '\0';
	wchar_t *found = wcsstr(toConvertFile, relativePath);
	if (found == 0) {
		wcscpy_s(outputFile, bufSize, toConvertFile);
		return false;
	}
	swprintf_s(outputFile, bufSize, L".%s", toConvertFile + wcslen(relativePath));
	return true;
}
