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

#ifndef SHARED
#define SHARED

#include "CommonHeader.h"
#include <map>

inline LeoRect<int> RECTToLeoRect(RECT a) {
	LeoRect<int> returnValue;
	returnValue.bottomRight.y = a.bottom;
	returnValue.upperLeft.x = a.left;
	returnValue.bottomRight.x = a.right;
	returnValue.upperLeft.y = a.top;
	return returnValue;
}

inline double EvaluateNormalDistribution(CvPoint mean, float variance, CvPoint x) {
	double pi_inverse = 0.39894228040143267793994605993438;
	double preMult = double(sqrt(variance));
	preMult = pi_inverse/preMult;
	double distance = (mean.x - x.x)*(mean.x - x.x) + (mean.y - x.y)*(mean.y - x.y);
	double exponent = distance / (2*variance);
	return preMult*exp(-exponent);
}

class StereoCalibration;
class VideoOutput;
class StereoRig;
class ElaborationCore;

class SystemInfo {
public:
	SystemInfo(HWND statusBar, HWND mainWnd, HWND leftPic, HWND rightPic);
	~SystemInfo();
	
	HWND GetStatusBar();
	
	VideoOutput * GetVideoOutput();
	void SetStereoRig(StereoRig *asr);
	StereoRig * GetStereoRig();
	void SetElaborationCore(ElaborationCore *ec);
	ElaborationCore * GetElaborationCore();
	HWND GetMainWindow();
private:
	HWND statusBar, mainWnd;
	StereoRig *asr;
	VideoOutput *vo;
	ElaborationCore *ec;
};


void GetPLaTHEATempPath(wchar_t* pathBuf, int bufSize);

#endif