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

#include "LeoWindowsThread.h"

DWORD WINAPI Thread::Win32ThreadProc(LPVOID lpParam) {
	Thread *thisThread = (Thread *) lpParam;
	thisThread->Run();
	return 0;
}

Thread::Thread() {
	runningFlag = false;
}

Thread::~Thread() {
	
}

bool Thread::Start() {
	if (!runningFlag) {
		hThread = chBEGINTHREADEX(NULL, 0, Thread::Win32ThreadProc, (LPVOID) this, 0, &threadID);
		if (hThread != INVALID_HANDLE_VALUE) {
			runningFlag = true;
			return true;
		}
	}
	return false;
}

bool Thread::Stop() {
	if (runningFlag) {
		if (!StopPreprocedure())
			return false;
		DWORD res = WaitForSingleObject(hThread, INFINITE);
		if (res == WAIT_OBJECT_0) {
			runningFlag = false;
			CloseHandle(hThread);
			return true;
		}
	}
	return false;
}

bool Thread::IsRunning() {
	return runningFlag;
}

using namespace std;
using namespace stdext;

EventRaiser::EventRaiser() {
	subTable = new unordered_map<int, unordered_set<HANDLE>>();
	hMutex = CreateMutex(NULL, FALSE, NULL);
}

EventRaiser::~EventRaiser() {
	delete this->subTable;
	CloseHandle(hMutex);
}

void EventRaiser::SubscribeEvent(int ev, HANDLE hEvent) {
	WaitForSingleObject(hMutex, INFINITE);
	(*subTable)[ev].insert(hEvent);
	ReleaseMutex(hMutex);
}

void EventRaiser::UnSubscribeEvent(int ev, HANDLE hEvent) {
	WaitForSingleObject(hMutex, INFINITE);
	(*subTable)[ev].erase(hEvent);
	ReleaseMutex(hMutex);
}

void EventRaiser::RaiseEvent(int ev) {
	WaitForSingleObject(hMutex, INFINITE);
	unordered_map<int, unordered_set<HANDLE>>::iterator it = subTable->find(ev);
	if (it!=subTable->end()) {
		unordered_set<HANDLE>::iterator it2;
		for (it2 = (*it).second.begin(); it2!=(*it).second.end(); it2++)
			SetEvent(*it2);
	}
	ReleaseMutex(hMutex);
}

void EventRaiser::RetireEvent(int ev) {
	WaitForSingleObject(hMutex, INFINITE);
	unordered_map<int, unordered_set<HANDLE>>::iterator it = subTable->find(ev);
	if (it!=subTable->end()) {
		unordered_set<HANDLE>::iterator it2;
		for (it2 = (*it).second.begin(); it2!=(*it).second.end(); it2++)
			ResetEvent(*it2);
	}
	ReleaseMutex(hMutex);
}

//**** RWLock Implementation ****//
RWLock::RWLock() {
	readerCount = 0;
	hMutex = CreateMutex(NULL, FALSE, NULL);
	hAccess = CreateSemaphore(NULL, 1, 1, NULL);
}

RWLock::~RWLock() {
	if (hMutex != INVALID_HANDLE_VALUE)
		CloseHandle(hMutex);
	if (hAccess != INVALID_HANDLE_VALUE)
		CloseHandle(hAccess);
}

void RWLock::AcquireReadLock() {
	WaitForSingleObject(hMutex, INFINITE);
	readerCount++;
	if (readerCount == 1)
		WaitForSingleObject(hAccess, INFINITE);
	ReleaseMutex(hMutex);
}

void RWLock::ReleaseReadLock() {
	WaitForSingleObject(hMutex, INFINITE);
	readerCount--;
	if (readerCount == 0)
		ReleaseSemaphore(hAccess, 1, NULL);
	ReleaseMutex(hMutex);
}

void RWLock::AcquireWriteLock() {
	WaitForSingleObject(hAccess, INFINITE);
}

void RWLock::ReleaseWriteLock() {
	ReleaseSemaphore(hAccess, 1, NULL);
}