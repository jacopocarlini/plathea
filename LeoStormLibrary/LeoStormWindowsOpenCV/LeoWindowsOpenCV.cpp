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

#include "LeoWindowsOpenCV.h"
#include <LeoWindowsGDI.h>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

HBITMAP IplImage2HBITMAP(HWND destWindow, HBITMAP templ, CvArr *img) {
	CvMat dst;
	if (CV_IS_MAT(img)) {
		CvMat *imgMAT = (CvMat *) img;
		if (templ==NULL)
			templ = CreateCompatibleHBITMAP(destWindow, imgMAT->cols, imgMAT->rows, CV_MAT_CN(imgMAT->type) * 8, 0);
		BITMAP bmp; GetObject(templ, sizeof(BITMAP), &bmp);
		cvInitMatHeader(&dst, imgMAT->rows, imgMAT->cols, CV_MAKETYPE(CV_8U, CV_MAT_CN(imgMAT->type)), bmp.bmBits,
			(imgMAT->cols * CV_MAT_CN(imgMAT->type) + CV_MAT_CN(imgMAT->type)) & -4);
	} else if (CV_IS_IMAGE(img)) {
		IplImage *imgIPL = (IplImage *) img;
		if (templ==NULL)
			templ = CreateCompatibleHBITMAP(destWindow, imgIPL->width, imgIPL->height, imgIPL->nChannels * 8, imgIPL->origin);
		BITMAP bmp; GetObject(templ, sizeof(BITMAP), &bmp);
		cvInitMatHeader(&dst, imgIPL->height, imgIPL->width, CV_MAKETYPE(CV_8U, imgIPL->nChannels), bmp.bmBits,
			(imgIPL->width * imgIPL->nChannels + imgIPL->nChannels) & -4);
	}
	cvConvertImage(img, &dst, 0);
	return templ;
}

bool ConditionalShowImage(char *windowName, CvArr *data) {
	if (FindWindowA(NULL, windowName)) {
		cvShowImage(windowName, data);
		return true;
	} else
		return false;
}