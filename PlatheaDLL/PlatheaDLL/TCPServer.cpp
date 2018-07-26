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

#include "TCPServer.h"
#include <Mswsock.h>
#include <LeoLog4CPP.h>
#include "RoomSettings.h"

using namespace leostorm::logging;

TCPServer::TCPServer(const int port) {
	hStopRunningEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	this->port = port;
}

TCPServer::~TCPServer() {
	CloseHandle(hStopRunningEvent);
}

void TCPServer::Run(void *param) {
	SOCKET ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN inAddress;
	inAddress.sin_family = AF_INET;
	inAddress.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	inAddress.sin_port = htons((u_short) port);
	bind(ListenSocket, (sockaddr *) &inAddress, sizeof(inAddress));
	listen(ListenSocket, 5);
	while (true) {
		SOCKET incomingCall = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		CHAR AcceptBuffer[2 * (sizeof(SOCKADDR_IN) + 16)];
		WSAOVERLAPPED overStruct; SecureZeroMemory(&overStruct, sizeof(overStruct));
		overStruct.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		DWORD bytesReceived;
		
		GUID GuidAcceptEx = WSAID_ACCEPTEX;
		LPFN_ACCEPTEX fncAcceptEx;
		WSAIoctl(ListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx),
			&fncAcceptEx, sizeof(fncAcceptEx), &bytesReceived, NULL, NULL);
		
		fncAcceptEx(ListenSocket, incomingCall, (PVOID) AcceptBuffer, 0, sizeof(SOCKADDR_IN) + 16,
			sizeof(SOCKADDR_IN) + 16, &bytesReceived, &overStruct);
		
		HANDLE toWait[2] = {overStruct.hEvent, hStopRunningEvent};
		DWORD waitResult = WaitForMultipleObjects(2, toWait, FALSE, INFINITE);
		if (waitResult - WAIT_OBJECT_0 == 1) {
			closesocket(incomingCall);
			break;
		}

		DWORD flags;
		WSAGetOverlappedResult(ListenSocket, &overStruct, &bytesReceived, FALSE, &flags);
		
		GUID GuidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
		LPFN_GETACCEPTEXSOCKADDRS fncGetAcceptExSockaddrs;
		WSAIoctl(ListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidGetAcceptExSockaddrs, sizeof(GuidGetAcceptExSockaddrs),
			&fncGetAcceptExSockaddrs, sizeof(fncGetAcceptExSockaddrs), &bytesReceived, NULL, NULL);

		sockaddr *localAddress = NULL, *remoteAddress = NULL;
		INT localAddressLen = 0, remoteAddressLen = 0;
		fncGetAcceptExSockaddrs((PVOID) AcceptBuffer, 0, sizeof(SOCKADDR_IN) + 16,
			sizeof(SOCKADDR_IN) + 16, &localAddress, &localAddressLen, &remoteAddress, &remoteAddressLen);
		sockaddr_in *localAddressIn = (sockaddr_in *) localAddress;
		sockaddr_in *remoteAddressIn = (sockaddr_in *) remoteAddress;
		Logger::writeToLOG(L"Connection established with TCP client %S:%d from %S:%d\r\n",
			inet_ntoa(localAddressIn->sin_addr), localAddressIn->sin_port,
			inet_ntoa(remoteAddressIn->sin_addr), remoteAddressIn->sin_port);
		BOOL keepAliveTrue = TRUE;
		if (setsockopt(incomingCall, SOL_SOCKET, SO_KEEPALIVE, (char *) &keepAliveTrue, sizeof(BOOL)) == SOCKET_ERROR)
			Logger::writeToLOG(L"Cannot set the socket with the KEEPALIVE flag");
		ClientInfo ci; ci.s = incomingCall;
		ci.stopEvent = WSACreateEvent();
		ci.active = true;
		WSAEventSelect(ci.s, ci.stopEvent, FD_CLOSE);
		currentClients.push_back(ci);

		CloseHandle(overStruct.hEvent);
	}
	for each (ClientInfo ci in currentClients) {
		closesocket(ci.s);
		WSACloseEvent(ci.stopEvent);
	}
	closesocket(ListenSocket);
}

bool TCPServer::StopPreprocedure() {
	SetEvent(hStopRunningEvent);
	return true;
}

void TCPServer::NotifyClients(std::vector<TrackedObject *> &persons) {
	static int round = 0;
	int notificationStep = 1; //How many frames should pass between a measurement and another? >=1
	/*for (stdext::unordered_map<int, CvPoint3D32f>::iterator it = filtering.begin(); it != filtering.end(); ) {
		bool found = false;
		for (int i = 0; i < persons.size() && !found; i++) {
			if (persons[i]->ID == it->first)
				found = true;
		}
		if (!found)
			it = filtering.erase(it);
		else
			it++;
	}*/
	char rowToWrite[1024];
	int texelSide = (int) RoomSettings::GetInstance()->data.texelSide;
	for each (ClientInfo ci in currentClients) {
		if (ci.active && WSAWaitForMultipleEvents(1, &ci.stopEvent, FALSE, 0, FALSE) == WSA_WAIT_EVENT_0)
			ci.active = false;
		if (ci.active) {
			for (int i = 0; i < int(persons.size()); i++) {
				if (persons[i]->type == TRACKED || persons[i]->type == LOST) {
					int newX = ((persons[i]->bottomRight.x + persons[i]->upperLeft.x)/2)*texelSide;
					int newY = ((persons[i]->bottomRight.y + persons[i]->upperLeft.y)/2)*texelSide;
					char prefix[2];
					if (persons[i]->type == TRACKED)
						sprintf_s(prefix, "t");
					else if (persons[i]->type == LOST)
						sprintf_s(prefix, "l");
					char identityPrefix[2];
					if (persons[i]->nameID != -1) {
						if (persons[1]->justIdentified)
							sprintf_s(identityPrefix, "j");
						else
							sprintf_s(identityPrefix, "i");
					} else {
						sprintf_s(identityPrefix, "-");
					}
					/*if (filtering.find(persons[i]->ID) == filtering.end()) {
						CvPoint3D32f c;
						c.x = float(newX);
						c.y = float(newY);
						c.z = 1.0;
						filtering[persons[i]->ID] = c;
					} else {
						int currentNumber = (int) filtering[persons[i]->ID].z;
						if (currentNumber < 2) {
							currentNumber++;
							filtering[persons[i]->ID].z = float(currentNumber);
						}
						float learningFactor = 1.f/float(currentNumber);
						filtering[persons[i]->ID].x = (1.f - learningFactor)*filtering[persons[i]->ID].x + learningFactor*newX;
						filtering[persons[i]->ID].y = (1.f - learningFactor)*filtering[persons[i]->ID].y + learningFactor*newY;
					}*/
					/*sprintf_s(rowToWrite, "%d\t(%d,%d)\t%d\t%s\r\n", persons[i]->ID, (int) filtering[persons[i]->ID].x,
						(int) filtering[persons[i]->ID].y,
						persons[i]->nameID, (persons[i]->nameID == -1 ? "-" : persons[i]->name));*/
					sprintf_s(rowToWrite, "%s\t%d\t(%d,%d)\t%d\t%s\t%s\r\n", prefix, persons[i]->ID, newX, newY,
						persons[i]->nameID, identityPrefix, (persons[i]->nameID == -1 ? "-" : persons[i]->name));
					if (round == 0)
						send(ci.s, rowToWrite, (int) strlen(rowToWrite), 0);
				}
			}
			if (round == 0)
				sprintf_s(rowToWrite, "-\r\n"); send(ci.s, rowToWrite, (int) strlen(rowToWrite), 0); //End of a frame
		}
	}
	round++; round %= notificationStep;
}