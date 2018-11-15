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

#include "VideoRecorder.h"

BufferedVideoRecorder::BufferedVideoRecorder(CvVideoWriter *leftWriter, CvVideoWriter *rightWriter, HANDLE hOffsetFile) {
	this->leftWriter = leftWriter;
	this->rightWriter = rightWriter;
	this->hOffsetFile = hOffsetFile;
	hNewDataToRead = CreateEvent(NULL, FALSE, FALSE, NULL);
	hStopRunningEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

BufferedVideoRecorder::BufferedVideoRecorder(wchar_t *path, CvSize resolution, int rate) {
	char sourceFileName[_MAX_PATH]; 

	sprintf_s(sourceFileName, "%S\\%s", path, "left.avi");
	DeleteFileA(sourceFileName);
	leftWriter = cvCreateVideoWriter(sourceFileName, 0, rate, resolution);

	sprintf_s(sourceFileName, "%S\\%s", path, "right.avi");
	DeleteFileA(sourceFileName);
	rightWriter = cvCreateVideoWriter(sourceFileName, 0, rate, resolution);
	
	sprintf_s(sourceFileName, "%S\\offsets.bin", path);
	hOffsetFile = CreateFileA(sourceFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	hNewDataToRead = CreateEvent(NULL, FALSE, FALSE, NULL);
	hStopRunningEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

BufferedVideoRecorder::~BufferedVideoRecorder() {
	CloseHandle(hNewDataToRead);
	CloseHandle(hStopRunningEvent);
}

void BufferedVideoRecorder::Run(void *param) {
	HANDLE hEventsToWait[] = {hNewDataToRead, hStopRunningEvent};
	while (WaitForMultipleObjects(2, hEventsToWait, FALSE, INFINITE) == WAIT_OBJECT_0) {
		listLock.AcquireReadLock();
		int numberToRead = (int) leftFrames.size();
		listLock.ReleaseReadLock();
		for (int i = 0; i < numberToRead; i++) {
			IplImage *leftFrame = leftFrames[i];
			IplImage *rightFrame = rightFrames[i];
			DWORD offset = offsets[i];
			DWORD offsetWrittenBytes;
			WriteFile(hOffsetFile, &offset, sizeof(DWORD), &offsetWrittenBytes, NULL);
			cvWriteFrame(leftWriter, leftFrame);
			cvWriteFrame(rightWriter, rightFrame);
			cvReleaseImage(&leftFrame);
			cvReleaseImage(&rightFrame);
		}
		listLock.AcquireWriteLock();
		for (int i = 0; i < numberToRead; i++) {
			leftFrames.erase(leftFrames.begin());
			rightFrames.erase(rightFrames.begin());
			offsets.erase(offsets.begin());
		}
		listLock.ReleaseWriteLock();
	}
	cvReleaseVideoWriter(&leftWriter);
	cvReleaseVideoWriter(&rightWriter);
	CloseHandle(hOffsetFile);
}

bool BufferedVideoRecorder::StopPreprocedure() {
	SetEvent(hStopRunningEvent);
	return true;
}

void BufferedVideoRecorder::AddStereoFrame(IplImage *left, IplImage *right, DWORD offset) {
	listLock.AcquireWriteLock();
	leftFrames.push_back(cvCloneImage(left));
	rightFrames.push_back(cvCloneImage(right));
	offsets.push_back(offset);
	SetEvent(hNewDataToRead);
	listLock.ReleaseWriteLock();
}