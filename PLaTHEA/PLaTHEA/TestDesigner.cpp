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

#include "TestDesigner.h"
#include "Shared.h"
#include <LeoWindowsGDI.h>
#include <LeoWindowsGUI.h>
#include <LeoWindowsOpenCV.h>
#include <Uxtheme.h>

void TestPath::GetExtremeValues(long &maxX, long &maxY) {
	for (std::vector<POINT>::const_iterator it = begin(); it != end(); it++) {
		if (it->x > maxX)
			maxX = it->x;
		if (it->y > maxY)
			maxY = it->y;
	}
}

void TestPath::DrawLines(IplImage *img, float normalizationFactor) {
	CvScalar scalarPathColor = cvScalar(GetBValue(pathColor), GetGValue(pathColor), GetRValue(pathColor));

	CvPoint p1, p2;
	for (int i = 0; i < int(size()); i++) {
		p1.x = int(at(i).x * normalizationFactor) + 15;
		p1.y = int(at(i).y * normalizationFactor) + 15;
		if (i > 0) {
			CvLineIterator cli; int lCount = cvInitLineIterator(img, p1, p2, &cli);
			int sequence = 0;
			for (int z = 0; z < lCount; z++) {
				if (z % 5 == 0)
					sequence++;
				if (sequence % 2 == 0) {
					cli.ptr[0] = GetBValue(pathColor);
					cli.ptr[1] = GetGValue(pathColor);
					cli.ptr[2] = GetRValue(pathColor);
				}
				CV_NEXT_LINE_POINT(cli);
			}
			//cvLine(img, p1, p2, scalarPathColor, 2, CV_AA);
		}
		p2 = p1;
	}
}

void TestPath::DrawLinesPS(char *destString, float normalizationFactor) {
	sprintf(destString, "%s%f %f %f setrgbcolor\n", destString, float(GetRValue(pathColor))/255.f, float(GetGValue(pathColor))/255.f, float(GetBValue(pathColor))/255.f);
	strcat(destString, "2 setlinewidth\n");
	strcat(destString, "newpath\n");
	sprintf(destString, "%s%d %d moveto\n", destString, int(at(0).x * normalizationFactor) + 15, int(at(0).y * normalizationFactor) + 15);
	for (int i = 1; i < int(size()); i++) {
		sprintf(destString, "%s%d %d lineto\n", destString, int(at(i).x * normalizationFactor) + 15, int(at(i).y * normalizationFactor) + 15);
	}
	//strcat(destString, "closepath\n");
	strcat(destString, "stroke\n");
}

void TestPath::DrawPoints(IplImage *img, float normalizationFactor) {
	CvScalar scalarPathColor = cvScalar(MAX(GetBValue(pathColor) - 50, 0),
		MAX(GetGValue(pathColor) - 50, 0), MAX(GetRValue(pathColor) - 50, 0));

	CvPoint p1;
	for (int i = 0; i < int(size()); i++) {
		p1.x = int(at(i).x * normalizationFactor) + 15;
		p1.y = int(at(i).y * normalizationFactor) + 15;
		cvCircle(img, p1, (i == 0 ? 4 : 2), scalarPathColor, 3, CV_AA);
	}
}

void TestPath::DrawPointsPS(char *destString, float normalizationFactor) {
	sprintf(destString, "%s%f %f %f setrgbcolor\n", destString, float(GetRValue(pathColor))/255.f, float(GetGValue(pathColor))/255.f, float(GetBValue(pathColor))/255.f);
	strcat(destString, "3 setlinewidth\n");
	for (int i = 0; i < int(size()); i++) {
		sprintf(destString, "%s%d %d %d 0 360 arc stroke\n", destString,
			int(at(i).x * normalizationFactor) + 15,
			int(at(i).y * normalizationFactor) + 15,
			(i == 0 ? 4 : 2));
	}
}

void TestPath::DrawNumbers(IplImage *img, CvScalar textColor, float normalizationFactor) {
	char normalBuffer[10];

	CvFont selectedFont; cvInitFont(&selectedFont, CV_FONT_HERSHEY_PLAIN, 1.0, 1.0, 0.0, 2, CV_AA);

	CvPoint p1;
	for (int i = 0; i < int(size()); i++) {
		p1.x = int(at(i).x * normalizationFactor) + 15;
		p1.y = int(at(i).y * normalizationFactor) + 15;
		sprintf_s(normalBuffer, "");
		bool first = true;
		for (int j = 0; j < i && first; j++) {
			if (at(i).x == at(j).x && at(i).y == at(j).y)
				first = false;
		}
		if (first) {
			for (int j = i; j < int(size()); j++) {
				if (at(i).x == at(j).x && at(i).y == at(j).y)
					sprintf_s(normalBuffer, 10, "%s%s%d", normalBuffer, (j>i ? "," : ""), j);
			}
			cvPutText(img, normalBuffer, p1, &selectedFont, textColor);
		}
	}
}

void TestPath::DrawNumbersPS(char *destString, float normalizationFactor) {
	char normalBuffer[10];

	strcat(destString, "0.0 0.0 0.0 setrgbcolor\n");
	strcat(destString, "/Times-Roman findfont\n");
	strcat(destString, "[ 20 0 0 -20 0 0 ] makefont\n");
	//strcat(destString, "12 scalefont\n");
	strcat(destString, "setfont\n");
	for (int i = 0; i < int(size()); i++) {
		sprintf_s(normalBuffer, "");
		bool first = true;
		for (int j = 0; j < i && first; j++) {
			if (at(i).x == at(j).x && at(i).y == at(j).y)
				first = false;
		}
		if (first) {
			for (int j = i; j < int(size()); j++) {
				if (at(i).x == at(j).x && at(i).y == at(j).y)
					sprintf_s(normalBuffer, 10, "%s%s%d", normalBuffer, (j>i ? "," : ""), j);
			}
			sprintf(destString, "%s%d %d moveto\n", destString, int(at(i).x * normalizationFactor) + 15, int(at(i).y * normalizationFactor) + 15);
			sprintf(destString, "%s(%s) show\n", destString, normalBuffer);
		}
	}
}

void TestPath::UpdateImage(IplImage *img) {
	cvRectangle(img, cvPoint(0,0), cvPoint(img->width - 1, img->height - 1), cvScalar(255,255,255), CV_FILLED);

	if (size()) {
		int width = img->width - 30, height = img->height - 30;
		long maxX = 0, maxY = 0;
		
		GetExtremeValues(maxX, maxY);

		float normalizationFactor = MIN(float(width)/float(maxX), float(height)/float(maxY)); 

		cvLine(img, cvPoint(0, 15), cvPoint(img->width, 15), cvScalar(0, 0, 0));
		cvLine(img, cvPoint(15, 0), cvPoint(15, img->height), cvScalar(0, 0, 0));
		cvLine(img, cvPoint(0, int(maxY * normalizationFactor) + 15),
			cvPoint(img->width, int(maxY*normalizationFactor) + 15), cvScalar(0, 0, 0));
		cvLine(img, cvPoint(int(maxX * normalizationFactor) + 15, 0),
			cvPoint(int(maxX * normalizationFactor) + 15, img->height), cvScalar(0, 0, 0));

		DrawLines(img, normalizationFactor);
		DrawPoints(img, normalizationFactor);
		DrawNumbers(img, cvScalar(0, 0, 0), normalizationFactor);
	}
}

void TestPath::SaveToPS(char *destString, int width, int height) {
	if (size()) {
		width = width - 30; height = height - 30;
		long maxX = 0, maxY = 0;
		
		GetExtremeValues(maxX, maxY);

		float normalizationFactor = MIN(float(width)/float(maxX), float(height)/float(maxY));
		int normalizedMaxX = int(maxX * normalizationFactor);
		int normalizedMaxY = int(maxY * normalizationFactor);

		strcat(destString, "0.0 0.0 0.0 setrgbcolor\n");
		strcat(destString, "1 setlinewidth\n");
		//BEGIN SURROUNDING BOX
		strcat(destString, "newpath\n");
		strcat(destString, "15 15 moveto\n"); sprintf(destString, "%s%d 15 lineto\n", destString, normalizedMaxX + 15);
		sprintf(destString, "%s%d %d lineto\n", destString, normalizedMaxX + 15, normalizedMaxY + 15);
		sprintf(destString, "%s%d %d lineto\n", destString, 15, normalizedMaxY + 15);
		sprintf(destString, "%s%d %d lineto\n", destString, 15, 15);
		strcat(destString, "closepath\n");
		strcat(destString, "stroke\n");
		//END SURROUNDING BOX

		DrawLinesPS(destString, normalizationFactor);
		DrawPointsPS(destString, normalizationFactor);
		DrawNumbersPS(destString, normalizationFactor);
	}
}

void UpdateTestDesignerWindowTitle(HWND hwnd, char *fileName, bool modified) {
	static const char baseWindowTitle[] = "PLaTHEA Test Designer";
	static char lastSelectedFilename[256];
	if (fileName)
		strcpy_s(lastSelectedFilename, 256, fileName);
	char normalBuffer[256];
	sprintf_s(normalBuffer, 256, "%s - %s%s", baseWindowTitle, (fileName ? fileName : lastSelectedFilename), (modified ? "*" : ""));
	SetWindowTextA(hwnd, normalBuffer);
}

HWND testDesignerWindow = NULL;

TestDesigner mainTestDesigner;

TestPath currentlySelectedPath;

static IplImage *currentImage = 0; static HBITMAP hBitmap = NULL; static HDC memDC = NULL;

void UpdatePathTable(HWND imageFrame, HWND pathTable, int rowToSelect = -1) {
	wchar_t temp[20];

	if (pathTable) {
		ListView_DeleteAllItems(pathTable);

		LVITEM li;
		li.mask = LVIF_TEXT | LVIF_STATE;
		li.state = 0; li.stateMask = 0;
		li.pszText = temp;
		li.cchTextMax = 20;
		li.iSubItem = 0;

		for (int i = 0; i < int(currentlySelectedPath.size()); i++) {
			li.iItem = i;

			swprintf_s(temp, 20, L"%d", i);
			ListView_InsertItem(pathTable, &li);

			swprintf_s(temp, 20, L"%d", int(currentlySelectedPath[i].x));
			ListView_SetItemText(pathTable, i, 1, temp);

			swprintf_s(temp, 20, L"%d", int(currentlySelectedPath[i].y));
			ListView_SetItemText(pathTable, i, 2, temp);
		}
		if (rowToSelect == - 1)
			rowToSelect = MAX(rowToSelect, int(currentlySelectedPath.size()) - 1);
		else
			rowToSelect = MIN(rowToSelect, int(currentlySelectedPath.size()) - 1);
		if (rowToSelect != -1) {
			ListView_SetItemState( pathTable, rowToSelect,
				LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED); 
		}
		int numberOfLearningMeasurement = ListView_GetItemCount(pathTable);
		if (numberOfLearningMeasurement > 0)
			ListView_EnsureVisible(pathTable, numberOfLearningMeasurement - 1, FALSE);
	}

	if (imageFrame) {
		currentlySelectedPath.UpdateImage(currentImage);
		hBitmap = IplImage2HBITMAP(imageFrame, hBitmap, currentImage);
		SelectObject(memDC, hBitmap);
		HDC frameDC = GetDC(imageFrame);
		BitBlt(frameDC, 0, 0, currentImage->width, currentImage->height, memDC, 0, 0, SRCCOPY);
		ReleaseDC(imageFrame, frameDC);
	}
	
}

void UpdatePathList(HWND pathCombo) {
	wchar_t wideBuffer[256];
	GetWindowText(pathCombo, wideBuffer, 256);
	ComboBox_ResetContent(pathCombo);
	for (std::unordered_map<int, TestPath>::const_iterator it = mainTestDesigner.begin(); it != mainTestDesigner.end(); it++) {
		SendMessageA(pathCombo, CB_ADDSTRING, (WPARAM) 0, (LPARAM) it->second.pathName);
	}
	SetWindowText(pathCombo, wideBuffer);
}

INT_PTR CALLBACK TestDesignerProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	wchar_t wideBuffer[256];
	char normalBuffer[256];
	HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST1);
	HWND hwndCombo = GetDlgItem(hwndDlg, IDC_COMBO1);
	HWND destFrame = GetDlgItem(hwndDlg, IDC_PATHBOX);
	HWND colorStatic = GetDlgItem(hwndDlg, IDC_COLORSTATIC);

	static HBRUSH hPathColorBrush = CreateSolidBrush(RGB(0, 255, 0));
	
	switch(uMsg) {
	case WM_ACTIVATE:
		if (wParam == WA_INACTIVE)
			hActiveModelessWindow = NULL;
		else
			hActiveModelessWindow = hwndDlg;
		return TRUE;
	case WM_CLOSE:
		cvReleaseImage(&currentImage);
		EnableWindow(GetParent(hwndDlg), TRUE);
		DestroyWindow(hwndDlg);
		DeleteObject(hBitmap); hBitmap = NULL;
		DeleteDC(memDC); memDC = NULL;
		return TRUE;
	case WM_INITDIALOG:
		{
			DWORD dwStyle = (DWORD) SendMessage(hwndList, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
			dwStyle |= LVS_EX_FULLROWSELECT;
			SendMessage(hwndList,LVM_SETEXTENDEDLISTVIEWSTYLE,0,dwStyle);
			
			LVCOLUMN lc;
			lc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
			lc.fmt = LVCFMT_LEFT;

			lc.cx = 60;
			lc.pszText = L"ID";
			lc.iSubItem = 0;
			ListView_InsertColumn(hwndList, 0, &lc);
			lc.pszText = L"X";
			lc.iSubItem = 1;
			ListView_InsertColumn(hwndList, 1, &lc);
			lc.pszText = L"Y";
			lc.iSubItem = 2;
			ListView_InsertColumn(hwndList, 2, &lc);

			SetWindowTheme(hwndList, L"Explorer", NULL);

			currentlySelectedPath.clear();

			RECT imageClientRect; GetClientRect(GetDlgItem(hwndDlg, IDC_PATHBOX), &imageClientRect);
			currentImage = cvCreateImage(cvSize(imageClientRect.right, imageClientRect.bottom), IPL_DEPTH_8U, 3);
			cvRectangle(currentImage, cvPoint(0,0), cvPoint(currentImage->width - 1, currentImage->height - 1),
				cvScalar(255,255,255), CV_FILLED);
			hBitmap = NULL;
			HDC frameDC = GetDC(destFrame);
			memDC = CreateCompatibleDC(frameDC);
			ReleaseDC(destFrame, frameDC);

			UpdatePathList(hwndCombo);

			EnableWindow(GetParent(hwndDlg), FALSE);
		}
		return TRUE;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hwndDlg, &ps);

			HDC frameDC = GetDC(destFrame);
			RECT frameClientRect; GetClientRect(destFrame, &frameClientRect);
			if (memDC && hBitmap) {
				SelectObject(memDC, hBitmap);
				BitBlt(frameDC, 0, 0, currentImage->width, currentImage->height, memDC, 0, 0, SRCCOPY);
			} else {
				FillRect(frameDC, &frameClientRect, (HBRUSH) GetStockObject(BLACK_BRUSH));
			}
			ReleaseDC(destFrame, frameDC);

			HDC colorDC = GetDC(colorStatic);
			RECT colorFrameRect; GetClientRect(colorStatic, &colorFrameRect);
			FillRect(colorDC, &colorFrameRect, hPathColorBrush);
			ReleaseDC(colorStatic, colorDC);

			EndPaint(hwndDlg, &ps);
		}
		return TRUE;
	case WM_CONTEXTMENU:
		{
			if (currentImage && HWND(wParam) == destFrame) {
				HMENU hPopupMenu = CreatePopupMenu();
				MENUITEMINFO mii; memset(&mii, 0, sizeof(mii));
				mii.cbSize = (UINT) sizeof(mii);
				mii.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE;
				mii.dwTypeData = wideBuffer;
				mii.fState = MFS_ENABLED;
				
				mii.wID = WM_USER;
				swprintf_s(wideBuffer, 255, L"Save as BMP image...");
				InsertMenuItem(hPopupMenu, 0, TRUE, &mii);
				mii.wID = WM_USER + 1;
				swprintf_s(wideBuffer, 255, L"Copy PS Code to Clipboard");
				InsertMenuItem(hPopupMenu, 1, TRUE, &mii);

				SetForegroundWindow(hwndDlg);
				POINT cursorPos; GetCursorPos(&cursorPos);
				UINT trackResult = (UINT) TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RETURNCMD | TPM_NONOTIFY, cursorPos.x, cursorPos.y,
					0, hwndDlg, NULL);
				DestroyMenu(hPopupMenu);
				if (trackResult == WM_USER) {
					COMDLG_FILTERSPEC filtro[] = {L"BMP Picture", L"*.bmp"};
					if (UseCommonItemDialog(normalBuffer, sizeof(normalBuffer), hwndDlg, CLSID_FileSaveDialog, 0, filtro, 1)) {
						cvSaveImage(normalBuffer, currentImage);
					}
				} else if (trackResult == WM_USER + 1) {
					char destBuffer[65000];
					//CHANGE ORIGIN TO UPPER-LEFT
					sprintf(destBuffer, "0 792 translate\n");
					strcat(destBuffer, "1 -1 scale\n");
					currentlySelectedPath.SaveToPS(destBuffer, currentImage->width, currentImage->height);
					const size_t len = strlen(destBuffer) + 1;
					HGLOBAL hMem =  GlobalAlloc(GMEM_MOVEABLE, len);
					memcpy(GlobalLock(hMem), destBuffer, len);
					GlobalUnlock(hMem);
					OpenClipboard(0);
					EmptyClipboard();
					SetClipboardData(CF_TEXT, hMem);
					CloseClipboard();
				}
			}
		}
		return TRUE;
	case WM_COMMAND:
		{
			WORD ctrlId = LOWORD(wParam);
			int selectedItem = ListView_GetSelectionMark(hwndList);
			if (HIWORD(wParam) == BN_CLICKED) {
				if (ctrlId == IDC_BUTTONADDPOINT) {
					POINT newPoint;
					GetDlgItemText(hwndDlg, IDC_EDITXCOORD, wideBuffer, 256);
					newPoint.x = _wtol(wideBuffer);
					GetDlgItemText(hwndDlg, IDC_EDITYCOORD, wideBuffer, 256);
					newPoint.y = _wtol(wideBuffer);
					currentlySelectedPath.push_back(newPoint);
					UpdatePathTable(destFrame, hwndList);
				} else if (ctrlId == IDC_BUTTONDELETE) {
					if (selectedItem != -1) {
						currentlySelectedPath.erase(currentlySelectedPath.begin() + selectedItem);
						UpdatePathTable(destFrame, hwndList, selectedItem);
					}
				} else if (ctrlId == IDC_BUTTONDOWN) {
					if (selectedItem != -1 && selectedItem < (int(currentlySelectedPath.size()) - 1)) {
						POINT aux = currentlySelectedPath[selectedItem + 1];
						currentlySelectedPath[selectedItem + 1] = currentlySelectedPath[selectedItem];
						currentlySelectedPath[selectedItem] = aux;
						UpdatePathTable(destFrame, hwndList, selectedItem + 1);
					}
				} else if (ctrlId == IDC_BUTTONUP) {
					if (selectedItem != -1 && selectedItem > 0) {
						POINT aux = currentlySelectedPath[selectedItem - 1];
						currentlySelectedPath[selectedItem - 1] = currentlySelectedPath[selectedItem];
						currentlySelectedPath[selectedItem] = aux;
						UpdatePathTable(destFrame, hwndList, selectedItem - 1);
					}
				} else if (ctrlId == IDC_BUTTONNEWPATH) {
					int msgBoxResult = MessageBox(hwndDlg, L"This operation will clean the current buffer. Are you sure?", L"Warning", MB_YESNO | MB_ICONWARNING);
					if (msgBoxResult == IDYES) {
						SetDlgItemText(hwndDlg, IDC_COMBO1, L"");
						currentlySelectedPath.clear();
						UpdatePathTable(destFrame, hwndList);
					}
				} else if (ctrlId == IDC_BUTTONDELETEPATH) {
					GetDlgItemTextA(hwndDlg, IDC_COMBO1, normalBuffer, 256);
					if (mainTestDesigner.FindPathIndexByName(normalBuffer) != -1) {
						int msgBoxResult = MessageBox(hwndDlg, L"Do you want to delete the selected path?", L"Warning", MB_YESNO | MB_ICONWARNING);
						if (msgBoxResult == IDYES) {
							mainTestDesigner.RemovePathFromDBase(normalBuffer);
							UpdatePathList(hwndCombo);
						}
					} else {
						MessageBox(hwndDlg, L"The selected path doesn't exist!", L"Warning", MB_OK | MB_ICONWARNING);
					}
				} else if (ctrlId == IDC_BUTTONLOADPATH) {
					GetDlgItemTextA(hwndDlg, IDC_COMBO1, normalBuffer, 256);
					int pathIndex = mainTestDesigner.FindPathIndexByName(normalBuffer);
					if (pathIndex != -1) {
						currentlySelectedPath = mainTestDesigner[pathIndex];
						UpdatePathTable(destFrame, hwndList);
						DeleteObject(hPathColorBrush);
						hPathColorBrush = CreateSolidBrush(currentlySelectedPath.pathColor);
						RedrawStaticControl(&colorStatic, 1);
					} else {
						MessageBox(hwndDlg, L"The selected path doesn't exist!", L"Warning", MB_OK | MB_ICONWARNING);
					}
				} else if (ctrlId == IDC_SAVEPATH) {
					GetDlgItemTextA(hwndDlg, IDC_COMBO1, normalBuffer, 256);
					trim(normalBuffer);
					if (strcmp(normalBuffer, "") != 0) {
						int msgBoxResult = IDYES;
						if (mainTestDesigner.FindPathIndexByName(normalBuffer) != -1) {
							msgBoxResult = MessageBox(hwndDlg, L"Do you want to replace the existing path?", L"Warning", MB_YESNO | MB_ICONWARNING);
						}
						if (msgBoxResult == IDYES) {
							strcpy_s(currentlySelectedPath.pathName, 256, normalBuffer);
							mainTestDesigner.AddPathToDBase(currentlySelectedPath);
							UpdatePathList(hwndCombo);
						}
					} else {
						MessageBox(hwndDlg, L"Please specify a non empty path name", L"Warning", MB_OK | MB_ICONWARNING);
					}
				} else if (ctrlId == IDC_BUTTONCOLOR) {
					if (UseCommonColorDialog(hwndDlg, currentlySelectedPath.pathColor)) {
						UpdatePathTable(destFrame, NULL);
						DeleteObject(hPathColorBrush);
						hPathColorBrush = CreateSolidBrush(currentlySelectedPath.pathColor);
						RedrawStaticControl(&colorStatic, 1);
					}
				}
				return TRUE;
			}
		}
		return FALSE;
	}
	return FALSE;
}

bool ShowTestDesignerWindow(HWND parentWindow) {
	testDesignerWindow = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_TESTDESIGNER), parentWindow, TestDesignerProc);
	ShowWindow(testDesignerWindow, SW_SHOW);
	UpdateWindow(testDesignerWindow);
	return true;
}

int TestDesigner::FindPathIndexByName(char *name) {
	for (std::unordered_map<int, TestPath>::const_iterator it = begin(); it != end(); it++) {
		if (strcmp(it->second.pathName, name) == 0) {
			return it->first;
		}
	}
	return -1;
}

int TestDesigner::AddPathToDBase(TestPath &t) {
	int index = FindPathIndexByName(t.pathName);
	if (index == -1)
		index = currentID++;
	(*this)[index] = t;
	return index;
}

bool TestDesigner::RemovePathFromDBase(char *name) {
	int index = FindPathIndexByName(name);
	if (index == -1)
		return false;
	erase(find(index));
	return true;
}

bool TestDesigner::SaveToXMLElement(TiXmlElement *elem) const {
	char wideBuffer[256];
	sprintf_s(wideBuffer, 256, "%d", currentID);
	elem->SetAttribute("currentPathID", wideBuffer);
	for (std::unordered_map<int, TestPath>::const_iterator it = begin(); it != end(); it++) {
		TiXmlElement *newPathElem = new TiXmlElement("TestPath");
		sprintf_s(wideBuffer, 256, "%d", it->first);
		newPathElem->SetAttribute("id", wideBuffer);
		newPathElem->SetAttribute("name", it->second.pathName);
		sprintf_s(wideBuffer, 256, "%#lX", it->second.pathColor);
		newPathElem->SetAttribute("color", wideBuffer);
		int i = 0;
		for (std::vector<POINT>::const_iterator pathIt = it->second.begin(); pathIt != it->second.end(); pathIt++) {
			TiXmlElement *newPointElem = new TiXmlElement("Point");
			sprintf_s(wideBuffer, 256, "%d", i++);
			newPointElem->SetAttribute("order", wideBuffer);
			sprintf_s(wideBuffer, 256, "%d", (int) pathIt->x);
			newPointElem->SetAttribute("X", wideBuffer);
			sprintf_s(wideBuffer, 256, "%d", (int) pathIt->y);
			newPointElem->SetAttribute("Y", wideBuffer);
			newPathElem->LinkEndChild(newPointElem);
		}
		elem->LinkEndChild(newPathElem);
	}
	return true;
}

bool TestDesigner::LoadFromXMLElement(TiXmlElement *elem) {
	clear();
	sscanf_s(elem->Attribute("currentPathID"), "%d", &currentID);
	for (TiXmlElement *pathElem = elem->FirstChildElement("TestPath"); pathElem != NULL; pathElem = pathElem->NextSiblingElement("TestPath")) {
		int pathID; sscanf_s(pathElem->Attribute("id"), "%d", &pathID);
		TestPath aux;
		sprintf_s(aux.pathName, 256, "%s", pathElem->Attribute("name"));
		sscanf_s(pathElem->Attribute("color"), "%lx", &aux.pathColor);
		
		aux.resize(0);
		int actualSize = 0;
		for (TiXmlElement *pointElem = pathElem->FirstChildElement("Point"); pointElem != NULL; pointElem = pointElem->NextSiblingElement("Point")) {
			POINT newPoint;
			newPoint.x = atol(pointElem->Attribute("X"));
			newPoint.y = atol(pointElem->Attribute("Y"));
			int order; sscanf_s(pointElem->Attribute("order"), "%d", &order);
			if (order + 1 > actualSize) {
				actualSize = order + 1;
				aux.resize(actualSize);
			}
			aux[order] = newPoint;
		}
		(*this)[pathID] = aux;
	}
	return true;
}
