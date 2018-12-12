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

#include "LeoSettingsPersistence.h"
#include <cstdio>

#include <LeoLog4CPP.h>

using namespace leostorm::logging;
using namespace leostorm::settingspersistence;

ParameterSetter::ParameterSetter(const char *moduleName) {
	this->moduleName = _strdup(moduleName);
	(*SettingsPersistence::GetInstance())[moduleName] = this;
}

ParameterSetter::~ParameterSetter() {
	free(moduleName);
	(*SettingsPersistence::GetInstance()).erase(moduleName);
}

SettingsPersistence *SettingsPersistence::instance = NULL;

SettingsPersistence *SettingsPersistence::GetInstance() {
	if (!instance)
		instance = new SettingsPersistence();
	return instance;
}

SettingsPersistence::SettingsPersistence(): LoggedModule(L"SETTINGSPERSISTENCE") {
	loadedFile=NULL;
}

SettingsPersistence::~SettingsPersistence() {
	instance = NULL;
}

void SettingsPersistence::LoadFromFile(char *fileName) {
	printf("SettingsPersistence: %s\n", fileName);
	TiXmlDocument doc(fileName);
	bool loadOkay = doc.LoadFile();
	if (!loadOkay)
		return;
	
	TiXmlHandle mainElement(doc.RootElement()); 

	for (ParameterSetterMap::const_iterator module_iterator = begin(); module_iterator != end(); module_iterator++) {
		TiXmlElement *configuration_element = mainElement.FirstChildElement(module_iterator->first.c_str()).Element();
		if (configuration_element == NULL) {
			Logger::writeToLOG(L"WARNING: No configuration for module %S.\r\n", module_iterator->first.c_str());
		} else {
			if (configuration_element->NextSiblingElement(module_iterator->first.c_str())) {
				Logger::writeToLOG(L"WARNING: More than a configuration tag for module %S. The first one will be used.\r\n", module_iterator->first.c_str());
			}

			module_iterator->second->LoadFromXMLElement(configuration_element); //If the class does not have any self acquired config the body of the function is empty

			ParameterMap &module_parameter_map = module_iterator->second->GetParameterMap();
			
			for (ParameterMap::const_iterator parameter_iterator = module_parameter_map.begin(); parameter_iterator != module_parameter_map.end(); parameter_iterator++) {
				TiXmlElement *parameter_element = configuration_element->FirstChildElement(parameter_iterator->first.c_str());
				if (parameter_element) {
					const char *type_attribute = parameter_element->Attribute("type");
					if (type_attribute) {
						printf("SettingsPersistence: %s\n",parameter_element->GetText());
						Logger::writeToLOG(L"%S:%S = (%S) %S\r\n", module_iterator->first.c_str(), parameter_iterator->first.c_str(), type_attribute, parameter_element->GetText());
						if (strcmp(type_attribute, "string") == 0) {
							if (parameter_iterator->second.param_type == parameter_iterator->second.STRING_PARAM) {
								sprintf_s((char *) parameter_iterator->second.param_ptr, 260, "%s", parameter_element->GetText());
							} else {
								Logger::writeToLOG(L"WARNING: Parameter type for %S:%S should be a string instead of a %S.\r\n", module_iterator->first.c_str(), parameter_iterator->first.c_str(), type_attribute);
							}
						} else if (strcmp(type_attribute, "float") == 0) {
							if (parameter_iterator->second.param_type == parameter_iterator->second.FLOAT_PARAM) {
								float *surrogatePointer = (float *) parameter_iterator->second.param_ptr;
								*surrogatePointer = (float) atof(parameter_element->GetText());
							} else {
								Logger::writeToLOG(L"WARNING: Parameter type for %S:%S should be a float instead of a %S.\r\n", module_iterator->first.c_str(), parameter_iterator->first.c_str(), type_attribute);
							}
						} else if (strcmp(type_attribute, "int") == 0) {
							if (parameter_iterator->second.param_type == parameter_iterator->second.INT_PARAM) {
								int *surrogatePointer = (int *) parameter_iterator->second.param_ptr;
								*surrogatePointer = atoi(parameter_element->GetText());
							} else {
								Logger::writeToLOG(L"WARNING: Parameter type for %S:%S should be an int instead of a %S.\r\n", module_iterator->first.c_str(), parameter_iterator->first.c_str(), type_attribute);
							}
						} else if (strcmp(type_attribute, "bool") == 0) {
							if (parameter_iterator->second.param_type == parameter_iterator->second.BOOL_PARAM) {
								bool *surrogatePointer = (bool *) parameter_iterator->second.param_ptr;
								*surrogatePointer = (strcmp("true", parameter_element->GetText()) == 0 ? true : false);
							} else {
								Logger::writeToLOG(L"WARNING: Parameter type for %S:%S should be a bool instead of a %S.\r\n", module_iterator->first.c_str(), parameter_iterator->first.c_str(), type_attribute);
							}
						} else {
							Logger::writeToLOG(L"Unrecognized data type %s for parameter %s:%s.\r\n", type_attribute, module_iterator->first.c_str(), parameter_iterator->first.c_str());
						}
					} else {
						Logger::writeToLOG(L"WARNING: No parameter type for parameter %s:%s. The parameter will be skipped.\r\n", module_iterator->first.c_str(), parameter_iterator->first.c_str());
					}
				} else {
					Logger::writeToLOG(L"WARNING: No configuration details for parameter %s:%s.\r\n", module_iterator->first.c_str(), parameter_iterator->first.c_str());
				}
			}
		}
	}

	loadedFile = _strdup(fileName);
}

void SettingsPersistence::SaveToFile(const char *rootLabel, const char *fileName) {
	wchar_t wide_buffer[256];
	char buffer[256];

	TiXmlDocument doc;
	TiXmlElement *root = new TiXmlElement(rootLabel);
	doc.LinkEndChild(root);

	for (ParameterSetterMap::const_iterator it = begin(); it!=end(); it++) {
		TiXmlElement *currentElement = new TiXmlElement(it->first.c_str());

		it->second->SaveToXMLElement(currentElement);

		for (ParameterMap::const_iterator map_it = it->second->GetParameterMap().begin(); map_it != it->second->GetParameterMap().end(); map_it++) {
			TiXmlElement *currentParam = new TiXmlElement(map_it->first.c_str());

			switch(map_it->second.param_type) {
			case Parameter::STRING_PARAM:
				currentParam->SetAttribute("type", "string");
				break;
			case Parameter::FLOAT_PARAM:
				currentParam->SetAttribute("type", "float");
				break;
			case Parameter::INT_PARAM:
				currentParam->SetAttribute("type", "int");
				break;
			case Parameter::BOOL_PARAM:
				currentParam->SetAttribute("type", "bool");
				break;
			}
			map_it->second.ToString(wide_buffer, 256);
			sprintf_s(buffer, 256, "%S", wide_buffer);
			currentParam->LinkEndChild(new TiXmlText(buffer));

			if (map_it->second.xmlComment.compare(L"")) {
				sprintf_s(buffer, 256, "%S", map_it->second.xmlComment.c_str());
				TiXmlComment *currentComment = new TiXmlComment(buffer);
				currentParam->LinkEndChild(currentComment);
			}

			currentElement->LinkEndChild(currentParam);
		}

		root->LinkEndChild(currentElement);
	}
	sprintf_s(buffer, 256, "%s", (fileName == NULL ? loadedFile : fileName));
	doc.SaveFile(buffer);
	doc.Clear();

	if (fileName)
		loadedFile = _strdup(fileName);
}
