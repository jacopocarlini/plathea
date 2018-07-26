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

#ifndef VIDEO_RECORDER_HEADER
#define VIDEO_RECORDER_HEADER

#include "CommonHeader.h"
#include "LeoWindowsThread.h"
#include <vector>

class BufferedVideoRecorder: public Thread {
private:
	RWLock listLock;
	std::vector<IplImage *> leftFrames;
	std::vector<IplImage *> rightFrames;
	std::vector<DWORD> offsets;
	HANDLE hStopRunningEvent, hNewDataToRead;
	CvVideoWriter *leftWriter, *rightWriter;
	HANDLE hOffsetFile;
public:
	BufferedVideoRecorder(CvVideoWriter *leftWriter, CvVideoWriter *rightWriter, HANDLE hOffsetFile);
	BufferedVideoRecorder(wchar_t *path, CvSize resolution, int rate);
	~BufferedVideoRecorder();
	void AddStereoFrame(IplImage *left, IplImage *right, DWORD offset);
protected:
	virtual void Run(void *param = NULL);
	virtual bool StopPreprocedure();
};

#endif //VIDEO_RECORDER_HEADER