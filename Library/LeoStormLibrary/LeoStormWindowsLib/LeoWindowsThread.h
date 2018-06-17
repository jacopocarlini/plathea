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

#ifndef LEOSTORM_WINDOWS_THREAD_H
#define LEOSTORM_WINDOWS_THREAD_H

#include "LeoWindowsCommonHeader.h"

#include <unordered_map>
#include <unordered_set>

class Thread {
private:
	HANDLE hThread;
	DWORD threadID;
	static DWORD WINAPI Win32ThreadProc(LPVOID lpParam);
public:
	Thread();
	~Thread();
	bool IsRunning();

	virtual bool Start();
	virtual bool Stop();
protected:
	bool runningFlag;
	virtual void Run(void *param = NULL) = 0;
	virtual bool StopPreprocedure() = 0;
};

class EventRaiser {
public:
	EventRaiser();
	~EventRaiser();
	void SubscribeEvent(int ev, HANDLE hEvent);
	void UnSubscribeEvent(int ev, HANDLE hEvent);
protected:
	void RaiseEvent(int ev);
	void RetireEvent(int ev);
private:
	std::unordered_map<int, std::unordered_set<HANDLE>> * subTable;
	HANDLE hMutex;
};

class RWLock {
public:
	RWLock();
	~RWLock();
	void AcquireReadLock();
	void ReleaseReadLock();
	void AcquireWriteLock();
	void ReleaseWriteLock();
private:
	int readerCount;
	HANDLE hMutex, hAccess;
};

#endif //LEOSTORM_WINDOWS_THREAD_H