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

#ifndef PLATHEA_TCP_SERVER
#define PLATHEA_TCP_SERVER

#include "CommonHeader.h"
#include <WinSock2.h>
#include <LeoWindowsThread.h>
#include <vector>
#include "PlanViewMap.h"

class TCPServer: public Thread {
private:
	//stdext::unordered_map<int, CvPoint3D32f> filtering;
	struct ClientInfo {
		SOCKET s;
		HANDLE stopEvent;
		bool active;
	};
	HANDLE hStopRunningEvent;
	int port;
	std::vector<ClientInfo> currentClients;
public:
	TCPServer(const int port);
	~TCPServer();
	void NotifyClients(std::vector<TrackedObject *> &persons);
protected:
	virtual void Run(void *param = NULL);
	virtual bool StopPreprocedure();
};

extern TCPServer* serv;

#endif //PLATHEA_TCP_SERVER