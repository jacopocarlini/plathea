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

#include "LeoWindowsGDI.h"
#include <cmath>

HBITMAP CreateCompatibleHBITMAP(HWND destWindow, int width, int height, int bpp, int origin) {
	unsigned char * buf = new unsigned char[sizeof(BITMAPINFO)+255*sizeof(RGBQUAD)];
	BITMAPINFO* bi = (BITMAPINFO*) buf;
	BITMAPINFOHEADER* bih = &bi->bmiHeader;
	memset(bih, 0, sizeof(BITMAPINFOHEADER));
	bih->biSize = sizeof(BITMAPINFOHEADER);
	bih->biWidth = width; //Larghezza dell'immagine
	bih->biHeight = origin ? abs(height) : -abs(height); //Altezza dell'immagine
	bih->biPlanes = 1; //Le componenti vengono visualizzate come RGBRGBRGB
	bih->biBitCount = (unsigned short) bpp; //Numero di bit per punto
	bih->biCompression = BI_RGB;

	if( bpp == 8 ) { //Nel caso di immagine in scala di grigi
        RGBQUAD* palette = bi->bmiColors;
        int i;
        for( i = 0; i < 256; i++ )
        {
            palette[i].rgbBlue = palette[i].rgbGreen = palette[i].rgbRed = (BYTE)i;
            palette[i].rgbReserved = 0;
        }
    }

	void* dstPtr;
	HDC destDC = GetDC(destWindow);
	HBITMAP res = CreateDIBSection(destDC, bi, DIB_RGB_COLORS, &dstPtr, 0, 0);
	ReleaseDC(destWindow, destDC);
	delete[] buf;
	return res;
}

HBITMAP RawBits2HBITMAP(HWND destWindow, HBITMAP templ, int width, int height, int size, void* pBits) {
	if (templ==NULL)
		templ = CreateCompatibleHBITMAP(destWindow, width, height, 24, 1);
	BITMAP bmp; GetObject(templ, sizeof(BITMAP), &bmp);
	memcpy(bmp.bmBits, pBits, size);
	return templ;
}

void DrawRectangle(RECT &ret, HDC dc) {
	MoveToEx(dc, ret.left, ret.top, NULL); LineTo(dc, ret.right, ret.top);
	MoveToEx(dc, ret.right, ret.top, NULL); LineTo(dc, ret.right, ret.bottom);
	MoveToEx(dc, ret.right, ret.bottom, NULL); LineTo(dc, ret.left, ret.bottom);
	MoveToEx(dc, ret.left,ret.bottom, NULL); LineTo(dc, ret.left, ret.top);
}

BOOL AdaptiveBitBlt(HDC destDC, int destX, int destY, int destWidth, int destHeight, HDC srcDC, int srcImageWidth, int srcImageHeight) {
	float imageWidthToHeightRatio = float(srcImageWidth) / float(srcImageHeight);
	float destWidthToHeightRatio = float(destWidth) / float(destHeight);
	int stretchWidth = srcImageWidth;
	int stretchHeight = srcImageHeight;
	if (srcImageWidth > destWidth || srcImageHeight > destHeight) {
		stretchWidth = (destWidthToHeightRatio > imageWidthToHeightRatio ?  int(destHeight * imageWidthToHeightRatio) : destWidth);
		stretchHeight = (destWidthToHeightRatio > imageWidthToHeightRatio ? destHeight : int(destWidth * (1 / imageWidthToHeightRatio)));
	}
	SetStretchBltMode(destDC, HALFTONE);
	return StretchBlt(destDC, destX, destY, stretchWidth, stretchHeight, srcDC, 0, 0, srcImageWidth, srcImageHeight, SRCCOPY);
}