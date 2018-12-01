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

#ifndef VIDEOOUTPUT
#define VIDEOOUTPUT

#include "JackSettings.h"
#include "CommonHeader.h"
#include "NetworkCamera.h"
#include <LeoWindowsThread.h>
#include <LeoWindowsGDI.h>
#include <LeoWindowsOpenCV.h>

#include <fstream>
#include <string>
#include <iostream>

enum SourceType {NETWORK_STEREO_RIG, NET_CAMERA};

struct VideoEvent {
	SourceType st;
	EventRaiser * eventSource;
	RaisableEvent ev;
};



enum DestinationSide {LEFT_SIDE_SCREEN, RIGHT_SIDE_SCREEN};

class VideoOutput: public Thread {
public:
	VideoOutput(HWND hwndLeftPic, HWND hwndRightPic, VideoEvent *veLeft = NULL, VideoEvent *veRight = NULL);
	~VideoOutput();
	void selectFrame(DestinationSide ds, IplImage * image);
	void Dlg_OnPaint (DestinationSide ds);
	void ChangeSource(VideoEvent *veLeft = NULL, VideoEvent *veRight = NULL);
	static VideoEvent GetNoneEvent();
	HWND *GetStaticControls();
	void setRoomID(int roomId);
private:
	HWND hwndPic[2];
	HBITMAP hBitmap[2];
	HANDLE hThread, hReadyEvent[2];
	HDC hdcMem[2]; //Memory HDC compatibile con la destinazione
	BITMAP bmp[2];
	VideoEvent ve[2];
	CvSize lastImageSize;
	bool continueRunning;
protected:
	virtual void Run(void *param = NULL);
	virtual bool StopPreprocedure();
};

enum ProcessingStageImage {BACKGROUND_STAGE, RAW_FOREGROUND_STAGE, FILTERED_FOREGROUND_STAGE, DISPARITY_STAGE,
	EDGE_ACTIVITY_STAGE, PLANVIEW_OCCUPANCY_STAGE, PLANVIEW_HEIGHT_STAGE};

class ProcessingStageOutput {
private:
	bool testingPhase;
	HWND staticsHandle[7];
	HBITMAP staticsBitmaps[7];
	int backgroundcount = 0;
	int rawforegroundcount = 0;
	int foregroundcount = 0;
	int disparitycount = 0;
	int occupancycount = 0;
	int heightcount = 0;
	int edgecount = 0;

public:

	int roomID;

	void setRoomID(int roomId) {
		roomID = roomId;
	}

	void enterTestingPhase() {
		for (int i = 0; i < 7; i++) {
			staticsBitmaps[i] = NULL;
			staticsHandle[i] = NULL;
		}
		testingPhase = true;
	}
	void exitTestingPhase() {
		testingPhase = false;
		for (int i = 0; i < 7; i++)
			DeleteObject(staticsBitmaps[i]);
	}
	HWND & ImageDestinationInfo(ProcessingStageImage psi, char *buffer, int size) {
		if (buffer) {
			if (psi == BACKGROUND_STAGE) {
				strcpy_s(buffer, size, "Background");
			} else if (psi == RAW_FOREGROUND_STAGE) {
				strcpy_s(buffer, size, "Foreground Model");
			} else if (psi == FILTERED_FOREGROUND_STAGE) {
				strcpy_s(buffer, size, "Connected Components");
			} else if (psi == DISPARITY_STAGE) {
				strcpy_s(buffer, size, "Disparity");
			} else if (psi == EDGE_ACTIVITY_STAGE) {
				strcpy_s(buffer, size, "Edge Activity");
			} else if (psi == PLANVIEW_OCCUPANCY_STAGE) {
				strcpy_s(buffer, size, "Plan View Occupancy Map");
			} else if (psi == PLANVIEW_HEIGHT_STAGE) {
				strcpy_s(buffer, size, "Plan View Heights Map");
			}
		}
		return staticsHandle[psi];
	}
	bool showImage(ProcessingStageImage psi, CvArr *image) {
		char windowName[64];
		//HWND destStatic = ImageDestinationInfo(psi, windowName, 64);
		enterTestingPhase();
		if (!testingPhase) {
			return ConditionalShowImage(windowName, image);
		} else {
			//if (!destStatic)
				//return false;
			printf("showimage\n");
			//staticsBitmaps[psi] = IplImage2HBITMAP(destStatic, staticsBitmaps[psi], image);
			int imageWidth = (CV_IS_MAT(image) ? ((CvMat*) image)->cols : ((IplImage *) image)->width);
			int imageHeight = (CV_IS_MAT(image) ? ((CvMat*) image)->rows : ((IplImage *) image)->height);
			float imageWidthToHeightRatio = float(imageWidth) / float(imageHeight);
			RECT destRect; 
			//GetClientRect(destStatic, &destRect);
			/*
			float destWidthToHeightRatio = float(destRect.right-destRect.left) / float(destRect.bottom-destRect.top);
			int stretchWidth = (destWidthToHeightRatio > imageWidthToHeightRatio ?  int((destRect.bottom - destRect.top) * imageWidthToHeightRatio) : (destRect.right - destRect.left));
			int stretchHeight = (destWidthToHeightRatio > imageWidthToHeightRatio ? (destRect.bottom - destRect.top) : int((destRect.right-destRect.left) * (1 / imageWidthToHeightRatio)));
			if (imageHeight <= (destRect.bottom - destRect.top) && imageWidth <= (destRect.right - destRect.left)) {
				stretchWidth = imageWidth;
				stretchHeight = imageHeight;
			}
			*/
			printf("salvo\n");
			//HDC destDC = GetDC(destStatic);
			//HDC memoryDC = CreateCompatibleDC(destDC);
			//SelectObject(memoryDC, staticsBitmaps[psi]);
			//SetStretchBltMode(destDC, HALFTONE);
			//StretchBlt(destDC, 0, 0, stretchWidth, stretchHeight, memoryDC, 0, 0, imageWidth, imageHeight, SRCCOPY);
			std::string nameframe = "";
			std::string mutex = "";
			std::string input = "";
			roomID = IDroom;
			// Save 			
			if (psi == BACKGROUND_STAGE) {
				nameframe = "D:\\github\\plathea\\jaxrs-jersey-server-generated\\room" + std::to_string(roomID) + "\\background\\frame" + std::to_string(backgroundcount) + ".jpeg";
				mutex = "D:\\github\\plathea\\jaxrs-jersey-server-generated\\room" + std::to_string(roomID) + "\\background\\mutex.txt";
				input = std::to_string(backgroundcount);
			}
			else if (psi == RAW_FOREGROUND_STAGE) {
				nameframe = "D:\\github\\plathea\\jaxrs-jersey-server-generated\\room" + std::to_string(roomID) + "\\rawforeground\\frame" + std::to_string(rawforegroundcount) + ".jpeg";
				mutex = "D:\\github\\plathea\\jaxrs-jersey-server-generated\\room" + std::to_string(roomID) + "\\rawforeground\\mutex.txt";
				input = std::to_string(rawforegroundcount);
			}
			else if (psi == FILTERED_FOREGROUND_STAGE) {
				nameframe = "D:\\github\\plathea\\jaxrs-jersey-server-generated\\room" + std::to_string(roomID) + "\\foreground\\frame" + std::to_string(foregroundcount) + ".jpeg";
				mutex = "D:\\github\\plathea\\jaxrs-jersey-server-generated\\room" + std::to_string(roomID) + "\\foreground\\mutex.txt";
				input = std::to_string(foregroundcount);
			}
			else if (psi == DISPARITY_STAGE) {
				nameframe = "D:\\github\\plathea\\jaxrs-jersey-server-generated\\room" + std::to_string(roomID) + "\\disparity\\frame" + std::to_string(disparitycount) + ".jpeg";
				mutex = "D:\\github\\plathea\\jaxrs-jersey-server-generated\\room" + std::to_string(roomID) + "\\disparity\\mutex.txt";
				input = std::to_string(disparitycount);
			}
			else if (psi == EDGE_ACTIVITY_STAGE) {
				nameframe = "D:\\github\\plathea\\jaxrs-jersey-server-generated\\room" + std::to_string(roomID) + "\\edge\\frame" + std::to_string(edgecount) + ".jpeg";
				mutex = "D:\\github\\plathea\\jaxrs-jersey-server-generated\\room" + std::to_string(roomID) + "\\edge\\mutex.txt";
				input = std::to_string(edgecount);
			}
			else if (psi == PLANVIEW_OCCUPANCY_STAGE) {
				nameframe = "D:\\github\\plathea\\jaxrs-jersey-server-generated\\room" + std::to_string(roomID) + "\\occupancy\\frame" + std::to_string(occupancycount) + ".jpeg";
				mutex = "D:\\github\\plathea\\jaxrs-jersey-server-generated\\room" + std::to_string(roomID) + "\\occupancy\\mutex.txt";
				input = std::to_string(occupancycount);
			}
			else if (psi == PLANVIEW_HEIGHT_STAGE) {
				nameframe = "D:\\github\\plathea\\jaxrs-jersey-server-generated\\room" + std::to_string(roomID) + "\\height\\frame" + std::to_string(heightcount) + ".jpeg";
				mutex = "D:\\github\\plathea\\jaxrs-jersey-server-generated\\room" + std::to_string(roomID) + "\\height\\mutex.txt";
				input = std::to_string(heightcount);
			}			
			cvSaveImage(nameframe.c_str(), (IplImage *)image);			
			std::fstream out;
			out.open(mutex);
			out << input;
			out.close();

			if (psi == BACKGROUND_STAGE) {
				backgroundcount++;
			}
			else if (psi == RAW_FOREGROUND_STAGE) {
				rawforegroundcount++;
			}
			else if (psi == FILTERED_FOREGROUND_STAGE) {
				foregroundcount++;
			}
			else if (psi == DISPARITY_STAGE) {
				disparitycount++;
			}
			else if (psi == EDGE_ACTIVITY_STAGE) {
				edgecount++;
			}
			else if (psi == PLANVIEW_OCCUPANCY_STAGE) {
				occupancycount++;
			}
			else if (psi == PLANVIEW_HEIGHT_STAGE) {
				heightcount++;
			}

			//DeleteDC(memoryDC);
			//ReleaseDC(destStatic, destDC);
			return true;
		}
	}
};

extern ProcessingStageOutput processingStageOutput;

#endif