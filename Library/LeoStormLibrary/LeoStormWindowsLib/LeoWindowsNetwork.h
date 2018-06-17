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

#ifndef LEOSTORM_WINDOWS_NETWORK_H
#define LEOSTORM_WINDOWS_NETWORK_H

#include "LeoWindowsCommonHeader.h"

#include <WinSock2.h>
#include <ws2tcpip.h>

#include <string>
#include <unordered_map>

#define LEO_NETWORK_SOCKET_ERROR 1001
#define LEO_NETWORK_CONNECT_ERROR 1002

void TCPRequestAndReply (sockaddr_in* address, char *request, char *reply, int maxcc);

int encodeBase64(char * data, int datalength, char** out);
int decodeBase64(char * data, int dataLength, char** out);

std::string readLine(SOCKET ConnectSocket);

char* createHttpSimpleAuthentication(const char* username, const char*password);

class HttpStreamSplitter {
public:
	HttpStreamSplitter(SOCKET ConnectSocket);
	~HttpStreamSplitter();
	DWORD ReadHeaders(std::unordered_map<std::string, std::string> &ht);
	bool IsAtStreamEnd();
private:
	SOCKET ConnectSocket;
	bool stream_end;
	void AddPropertyValue(std::string propline, std::unordered_map<std::string, std::string>& ht);
};

#endif //LEOSTORM_WINDOWS_NETWORK_H