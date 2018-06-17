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

#include "LeoOpenCVExtension.h"
#include <opencv2\imgproc\imgproc.hpp>

void applyMask(IplImage *src, IplImage *mask, IplImage *dest) {
	for (int row = 0; row<src->height; row++) {
		const uchar * srcPtr = (const uchar *) (src->imageData + row*src->widthStep);
		const uchar * maskPtr = (const uchar *) (mask->imageData + row*mask->widthStep);
		uchar * destPtr = (uchar *) (dest->imageData + row*dest->widthStep);
		for (int col = 0; col<src->width; col++, srcPtr++, maskPtr++, destPtr++) {
			if (*maskPtr)
				*destPtr = *srcPtr;
			else
				*destPtr = 0;
		}
	}
}

void CreateFrame(const IplImage *src, IplImage *dst, CvScalar color) {
	int horzDiff = dst->width - src->width; horzDiff/=2;
	int vertDiff = dst->height - src->height; vertDiff/=2;
	cvSet(dst, color);
	for (int row = 0; row < src->height; row++) {
		const uchar *srcPtr = (const uchar *) (src->imageData + row*src->widthStep);
		uchar *dstPtr = (uchar *) (dst->imageData + (row+vertDiff)*dst->widthStep); dstPtr+=3*horzDiff;
		for (int col = 0; col < src->width; col++, srcPtr+=3, dstPtr+=3) {
			dstPtr[0] = srcPtr[0]; dstPtr[1] = srcPtr[1]; dstPtr[2] = srcPtr[2];
		}
	}
}

