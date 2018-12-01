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

#include "video_output.h"
#include <LeoWindowsGDI.h>
#include "PlatheaDLL.h"
#include <fstream>
#include <string>
#include <iostream>

#include "JackSettings.h"

int leftcount=0;

int roomID;

ProcessingStageOutput processingStageOutput;

VideoEvent VideoOutput::GetNoneEvent() {
	VideoEvent res;
	res.ev = NONE_EVENT;
	return res;
}

void VideoOutput::setRoomID(int roomId) {
	roomID = roomId;
}

VideoOutput::VideoOutput(HWND hwndLeftPic, HWND hwndRightPic, VideoEvent *veLeft, VideoEvent *veRight) {
	hwndPic[LEFT_SIDE_SCREEN]=hwndLeftPic;
	hwndPic[RIGHT_SIDE_SCREEN]=hwndRightPic;
	ve[LEFT_SIDE_SCREEN]=GetNoneEvent();
	ve[RIGHT_SIDE_SCREEN]=GetNoneEvent();

	hReadyEvent[LEFT_SIDE_SCREEN] = CreateEvent(NULL, FALSE, FALSE, NULL);
	hReadyEvent[RIGHT_SIDE_SCREEN] = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	hBitmap[LEFT_SIDE_SCREEN]=NULL; hBitmap[RIGHT_SIDE_SCREEN]=NULL;
	hdcMem[LEFT_SIDE_SCREEN]=CreateCompatibleDC(GetDC(hwndLeftPic));
	hdcMem[RIGHT_SIDE_SCREEN]=CreateCompatibleDC(GetDC(hwndRightPic));
	
	ChangeSource(veLeft, veRight);
	
	Start();
}

void VideoOutput::Dlg_OnPaint(DestinationSide ds) {
	RECT rect;
	HDC dc = GetDC(hwndPic[ds]);
	
	SelectObject(hdcMem[ds],hBitmap[ds]);
	GetClientRect(hwndPic[ds],&rect);
	AdaptiveBitBlt(dc,0,0,rect.right-rect.left,rect.bottom-rect.top,hdcMem[ds], lastImageSize.width, lastImageSize.height);
	
	ReleaseDC(hwndPic[ds],dc);
}

void VideoOutput::selectFrame(DestinationSide ds, IplImage *image) {
	hBitmap[ds] = IplImage2HBITMAP(hwndPic[ds], hBitmap[ds], image);
	Dlg_OnPaint(ds);
}

VideoOutput::~VideoOutput() {
	DeleteDC(hdcMem[LEFT_SIDE_SCREEN]);
	DeleteDC(hdcMem[RIGHT_SIDE_SCREEN]);
	CloseHandle(hReadyEvent[LEFT_SIDE_CAMERA]);
	CloseHandle(hReadyEvent[RIGHT_SIDE_CAMERA]);
}

void VideoOutput::ChangeSource(VideoEvent *veLeft, VideoEvent *veRight) {
	for (int i = 0; i<2; i++) {
		VideoEvent * temp = (i==0 ? veLeft: veRight);
		if (hBitmap[i])
			DeleteObject(hBitmap[i]);
		hBitmap[i] = NULL;
		if (ve[i].ev!=NONE_EVENT)
			ve[i].eventSource->UnSubscribeEvent(ve[i].ev, hReadyEvent[i]);
		ve[i]= (temp==NULL ? GetNoneEvent() : *temp);
		if (ve[i].ev!=NONE_EVENT)
			ve[i].eventSource->SubscribeEvent(ve[i].ev, hReadyEvent[i]);
	}
}

void VideoOutput::Run(void *param) {
	printf("VideoOutput: run \n");
	continueRunning = true;
	while (continueRunning) {
		printf("VideoOutput: continue running \n");
		DWORD res = WaitForMultipleObjects(2, hReadyEvent, FALSE, INFINITE);
		DWORD curr = res - WAIT_OBJECT_0;
		switch (ve[curr].st) {
			case NET_CAMERA:
				{
					NetworkCamera * asr = (NetworkCamera *) ve[curr].eventSource;
					asr->ImageLock.AcquireReadLock();
						IplImage * temp = asr->GetImage();
						if (curr == LEFT_SIDE_SCREEN)
							selectFrame(LEFT_SIDE_SCREEN, temp);
						else
							selectFrame(RIGHT_SIDE_SCREEN, temp);
						lastImageSize = cvSize(temp->width, temp->height);
						// Save 
						std::string nameframe = "D:\\github\\plathea\\jaxrs-jersey-server-generated\\room"+ std::to_string(roomID) +"\\frame" + std::to_string(leftcount) + ".jpeg";

						cvSaveImage(nameframe.c_str(), temp);
						std::string input = std::to_string(leftcount);
						std::fstream out;
						out.open("D:\\github\\plathea\\jaxrs-jersey-server-generated\\room" + std::to_string(roomID) + "\\mutex.txt");						
						out << input;
						out.close();
						leftcount++;
					asr->ImageLock.ReleaseReadLock();
				}
				break;
			case NETWORK_STEREO_RIG:
				{
					printf("video_output: NETWORK_STEREO_RIG");
					StereoRig * asr = (StereoRig *) ve[curr].eventSource;
					asr->StereoLock.AcquireReadLock();
						IplImage * left, * right;
						asr->GetStereoImages(&left, &right, false);
						// Save 
						std::string nameframe = "D:\\github\\plathea\\jaxrs-jersey-server-generated\\room" + std::to_string(roomID) + "\\frame"+ std::to_string(leftcount) +".jpeg";
						
						cvSaveImage(nameframe.c_str(), left);
						std::string input = std::to_string(leftcount);						
						std::ofstream out("D:\\github\\plathea\\jaxrs-jersey-server-generated\\room" + std::to_string(roomID) + "\\mutex.txt");
						out << input;
						out.close();
						leftcount++;

						selectFrame(LEFT_SIDE_SCREEN, left);
						selectFrame(RIGHT_SIDE_SCREEN, right);
						lastImageSize = cvSize(left->width, left->height);
					asr->StereoLock.ReleaseReadLock();
				}
				break;
		}
	}
}

bool VideoOutput::StopPreprocedure() {
	continueRunning = false;
	return true;
}

HWND *VideoOutput::GetStaticControls() {
	return hwndPic;
}