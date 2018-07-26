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

#ifndef LEOSTORM_SETTINGS_PERSISTENCE_H
#define LEOSTORM_SETTINGS_PERSISTENCE_H

#include <unordered_map>
#include <string>

#include <tinyxml.h>

#include <LeoLog4CPP.h>

//Classes for runtime parameter setters

namespace leostorm {
	namespace settingspersistence {

		struct Parameter {
			enum param_type_enum {STRING_PARAM, FLOAT_PARAM, INT_PARAM, BOOL_PARAM, UNDEFINED_PARAM} param_type;
			void *param_ptr;
			bool runtimeChangeable;
			std::wstring xmlComment;
			Parameter() {
				param_type = STRING_PARAM;
				param_ptr = 0;
				runtimeChangeable = true;
			}
			void ToString(wchar_t *buffer, int bufferLenght) const {
				if (param_type == FLOAT_PARAM)
					swprintf_s(buffer, bufferLenght, L"%.3f", *((float *) param_ptr));
				else if (param_type == Parameter::INT_PARAM)
					swprintf_s(buffer, bufferLenght, L"%d", *((int *) param_ptr));
				else if (param_type == Parameter::BOOL_PARAM)
					swprintf_s(buffer, bufferLenght, *((bool *) param_ptr) ? L"true" : L"false");
				else if (param_type == Parameter::STRING_PARAM)
					swprintf_s(buffer, bufferLenght, L"%S", (char *) param_ptr);
			}
		};

		class ParameterMap: public std::unordered_map<std::string, Parameter> {}; //A simple typedef would generate decorated name truncation

		class XMLSaver {
			virtual bool LoadFromXMLElement(TiXmlElement *xmlElement) = 0;
			virtual bool SaveToXMLElement(TiXmlElement *xmlElement) const = 0;
		};

		class ParameterSetter: public XMLSaver {
		private:
			ParameterMap pm;
			char *moduleName;
		protected:
			template <class T> void AddParameter(const char* paramName, T* paramPtr, std::wstring xmlComment = L"", bool runtimeChangeable = true) {
				ParameterMap::const_iterator it = pm.find(paramName);
				if (it != pm.end())
					throw "Already existing parameter";
				Parameter p;
				if (typeid(T) == typeid(float)) {
					p.param_type = Parameter::FLOAT_PARAM;
				} else if (typeid(T) == typeid(int)) {
					p.param_type = Parameter::INT_PARAM;
				} else if (typeid(T) == typeid(char)) {
					p.param_type = Parameter::STRING_PARAM;
				} else if (typeid(T) == typeid(bool)) {
					p.param_type = Parameter::BOOL_PARAM;
				} else {
					p.param_type = Parameter::UNDEFINED_PARAM;
				}
				p.param_ptr = paramPtr;
				p.runtimeChangeable = runtimeChangeable;
				p.xmlComment = xmlComment;
				pm[paramName] = p;
			}
		public:
			ParameterSetter(const char* moduleName);
			~ParameterSetter();
			ParameterMap &GetParameterMap() {
				return pm;
			}
			virtual void ModifyParameter(char *var, int offset = 1) {};
			template <class T> T GetParameterValue(const char *var) {
				ParameterMap::const_iterator it = pm.find(var);
				if (it==pm.end())
					throw "Parameter doesn't exist";
				return *((T *) it->second.param_ptr);
			}
			template <class T> void SetParameterValue(const char *var, T new_value) {
				ParameterMap::const_iterator it = pm.find(var);
				if (it==pm.end())
					throw "Parameter doesn't exist";
				*((T *) it->second.param_ptr) = new_value;
			}
			virtual bool LoadFromXMLElement(TiXmlElement *xmlElement) {return false;};
			virtual bool SaveToXMLElement(TiXmlElement *xmlElement) const {return false;};
		};

		//The storage manager

		typedef std::unordered_map<std::string, ParameterSetter *> ParameterSetterMap;

		class SettingsPersistence: public ParameterSetterMap, public leostorm::logging::LoggedModule  {
		private:
			static SettingsPersistence *instance;
			SettingsPersistence();
			char *loadedFile;
		public:
			static SettingsPersistence *GetInstance();
			~SettingsPersistence();
			void LoadFromFile(const char *fileName);
			void SaveToFile(char *rootLabel, char *fileName = NULL);
			char *GetCurrentlyLoadedFile() {
				return loadedFile;
			}
		};
	}
}

#endif //LEOSTORM_SETTINGS_PERSISTENCE_H