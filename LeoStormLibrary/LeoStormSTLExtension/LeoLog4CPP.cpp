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

#include "LeoLog4CPP.h"

using namespace leostorm::logging;

#include <cwchar>
#include <cstdarg>
#include <cstdlib>

Logger *Logger::selectedLogger = NULL;

Logger * Logger::GetCurrentLogger() {
	return selectedLogger;
}

bool Logger::SetCurrentLogger(Logger *logger) {
	if (logger == NULL || selectedLogger != logger) {
		delete selectedLogger;
	}
	selectedLogger = logger;
	return true;
}

Logger::~Logger() {
	selectedLogger = NULL;
}

Logger::Logger() {}

bool Logger::writeToLOG(const wchar_t *frmt, ...) {
	if (!selectedLogger)
		return false;
	wchar_t buf[4096];
	va_list vaArgs;
	va_start(vaArgs, frmt);
	vswprintf(buf, 4096, frmt, vaArgs);
	va_end(vaArgs);
	selectedLogger->virtualWriteToLOG(buf);
	return true;
}

bool LoggedModule::writeToLOG(wchar_t *frmt, ...) {
		wchar_t buf[4096], newFrmt[4096];
		swprintf_s(newFrmt, 4096, L"[%s] %s", moduleName, frmt);
		va_list vaArgs;
		va_start(vaArgs, frmt);
		vswprintf(buf, 4096, newFrmt, vaArgs);
		va_end(vaArgs);
		return Logger::writeToLOG(L"%s", buf);
}

LoggedModule::LoggedModule(wchar_t *moduleName) {
	this->moduleName = _wcsdup(moduleName);
}

LoggedModule::~LoggedModule() {
	free(moduleName);
}