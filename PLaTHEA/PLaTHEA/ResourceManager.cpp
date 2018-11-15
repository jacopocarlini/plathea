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

#include "ResourceManager.h"

std::unordered_map<std::string, ResourceManager *> ResourceManager::resourceManagersList;

std::string ResourceManager::GetLocalURL(std::string url) {
	ResourceManager *rm = NULL;
	for (std::unordered_map<std::string, ResourceManager *>::const_iterator it = resourceManagersList.begin(); rm == NULL && it != resourceManagersList.end(); it++) {
		if (url.length() >= it->first.length() && _stricmp(url.substr(0, it->first.length()).c_str(), it->first.c_str()) == 0)
			rm = it->second;
	}
	if (rm)
		return rm->URLToLocal(url);
	return url;
}

static class FileSystemResourceManager: public ResourceManager {
private:
	std::string URLToLocal(std::string url) const {
		return url.substr(7, url.length() - 7);
	}
public:
	FileSystemResourceManager(): ResourceManager("file://") {}
} mainFileSystemResourceManager;