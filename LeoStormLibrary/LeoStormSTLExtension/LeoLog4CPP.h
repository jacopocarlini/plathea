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

#ifndef LEOSTORM_LOG4CPP_H
#define LEOSTORM_LOG4CPP_H

namespace leostorm {
	namespace logging {
		class LoggedModule {
		private:
			wchar_t *moduleName;
		public:
			bool writeToLOG(wchar_t *frmt, ...);
			LoggedModule(wchar_t *moduleName);
			~LoggedModule();
		};

		class Logger {
			friend class LoggedModule;
		private:
			static Logger *selectedLogger;
		protected:
			virtual bool virtualWriteToLOG(wchar_t *message) = 0;
		public:
			static bool SetCurrentLogger(Logger *selectedLogger);
			static Logger *GetCurrentLogger();
			static bool writeToLOG(const wchar_t *frmt, ...);
			Logger();
			~Logger();
		};
	}
}

#endif //LEOSTORM_LOG4CPP_H