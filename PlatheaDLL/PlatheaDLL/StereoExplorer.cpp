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

#include "StereoExplorer.h"
#include "StereoCalibration.h"
#include "ApplicationWorkFlow.h"
#include "Shared.h"

extern SystemInfo *si;

StereoExplorer *StereoExplorer::instance = NULL;

StereoExplorer *StereoExplorer::GetInstance(CvMat *reprojection3D, CvMat *disparity) {
	if (instance && reprojection3D)
		return NULL;
	if (reprojection3D && disparity) {
		instance = new StereoExplorer();
		instance->reprojection3D = reprojection3D;
		instance->disparity = disparity;
	}
	return instance;
}

StereoExplorer::StereoExplorer() {
	PreviousFunction = (WNDPROC) SetWindowLongPtr(si->GetVideoOutput()->GetStaticControls()[0],
		GWLP_WNDPROC, (LONG_PTR) StereoExplorerProc);
}

StereoExplorer::~StereoExplorer() {
	instance = NULL;
	SetWindowLongPtr(si->GetVideoOutput()->GetStaticControls()[0], GWLP_WNDPROC, (LONG_PTR) PreviousFunction);
}

LRESULT CALLBACK StereoExplorer::StereoExplorerProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		case WM_SETCURSOR:
			{
				HCURSOR hc= LoadCursor(NULL, MAKEINTRESOURCE(IDC_CROSS));
				SetCursor(hc);
				return TRUE;
			}
		case WM_LBUTTONDOWN:
			{
				wchar_t msg[1024];
				int x = LOWORD(lParam), y = HIWORD(lParam);
				if (x >= 0 && x < instance->reprojection3D->width && y>=0 && y <instance->reprojection3D->height) {
					float *original_ptr = (float *) (instance->reprojection3D->data.ptr + y*instance->reprojection3D->step);
					original_ptr += 3*x;
					float *disp_ptr = (float *) (instance->disparity->data.ptr + y*instance->disparity->step);
					disp_ptr += x;
					float dispValue = (*disp_ptr);
					//We have to work in double because External Calibration data is double
					double cc[] = {original_ptr[0], original_ptr[1], original_ptr[2]};
					double wc[] = {cc[0], cc[1], cc[2]};
					if (ExternalCalibration::GetInstance()->IsComplete()) {
						CvMat aux; cvInitMatHeader(&aux, 3, 1, CV_64FC1, wc);
						cvSub(&aux, ExternalCalibration::GetInstance()->Traslation_Vector, &aux);
						cvGEMM(ExternalCalibration::GetInstance()->Rotation_Matrix, &aux, 1, NULL, 0, &aux);
						swprintf_s(msg, 1024,
							L"Pixel x: %d, y: %d, d: %.2f\r\nCamera Coordinate X: %.2lf, Y: %.2lf, Z: %.2lf\r\nWorld Coordinate: X: %.2lf, Y: %.2lf, Z: %.2lf",
							x, y, dispValue, cc[0], cc[1], cc[2], wc[0], wc[1], wc[2]);
					} else {
						swprintf_s(msg, 1024,
							L"Pixel x: %d, y: %d, d: %.2f\r\nCamera Coordinate X: %.2lf, Y: %.2lf, Z: %.2lf",
							x, y, dispValue, cc[0], cc[1], cc[2]);
					}
					MessageBox(hwnd, msg, L"Pixel Info", MB_ICONINFORMATION);
				}
				return 0;
			}
	}
	return CallWindowProc(StereoExplorer::GetInstance()->PreviousFunction, hwnd, uMsg, wParam, lParam);
}