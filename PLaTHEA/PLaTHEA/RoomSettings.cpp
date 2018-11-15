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

#include "RoomSettings.h"
#include "ApplicationWorkFlow.h"

#include <LeoSettingsPersistence.h>

using namespace leostorm::settingspersistence;

class RoomSettingsSetter: public ParameterSetter {
public:
	RoomSettingsStruct data;
	RoomSettingsSetter();
	bool SaveToXMLElement(TiXmlElement *xmlElement) const;
};

RoomSettingsSetter::RoomSettingsSetter(): ParameterSetter("roomSettings") {
	memset(&data, 0, sizeof(data));
	data.WXmax = 5000;
	data.WYmax = 5000;
	data.WZmax = 3000;
	data.personMaximumHeight = 2100;
	data.personMimimumHeight = 1000;
	data.personAverageHeight = 1700;
	data.personAverageWidth = 500;
	data.texelSide = 30;
	AddParameter("WXmin", &data.WXmin, L"Minimum X coordinate of the room, usually set to 0");
	AddParameter("WXmax", &data.WXmax, L"Maximum X coordinate of the room, usually set to the length of the X side of the room");
	AddParameter("WYmin", &data.WYmin, L"Minimum Y coordinate of the room, usually set to 0");
	AddParameter("WYmax", &data.WYmax, L"Maximum Y coordinate of the room, usually set to the length of the Y side of the room");
	AddParameter("WZmin", &data.WZmin, L"Minimum Z coordinate of the room, usually set to 0");
	AddParameter("WZmax", &data.WZmax, L"Maximum Z coordinate of the room, usually set to the height of the room");
	AddParameter("texelSide", &data.texelSide, L"During elaboration the floor is partitioned into texels; specify the side of the texel");
	AddParameter("personAverageHeight", &data.personAverageHeight, L"The average height of the persons is used into tracked objects recognition process");
	AddParameter("personAverageWidth", &data.personAverageWidth, L"The average width of the persons is used to calculate how many texels a person should occupy");
	AddParameter("personMaximumHeight", &data.personMaximumHeight, L"Used to ignore false positives");
	AddParameter("personMinimumHeight", &data.personMimimumHeight, L"Used to ignore false positives");
}

bool RoomSettingsSetter::SaveToXMLElement(TiXmlElement *elem) const {
	TiXmlComment *comment = new TiXmlComment("All measures are expressed in mm");
	elem->LinkEndChild(comment);
	return true;
};

static RoomSettingsSetter roomSettingsSetter;

RoomSettings *RoomSettings::instance = NULL;

RoomSettings *RoomSettings::GetInstance() {
	if (!instance)
		instance = new RoomSettings();
	return instance;
}

RoomSettings::RoomSettings(): data(roomSettingsSetter.data) {
}

INT_PTR CALLBACK RoomSettings::DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	RoomSettingsStruct *rs = &RoomSettings::GetInstance()->data;
	wchar_t aux[20];
	switch (uMsg) {
		case WM_INITDIALOG:
			swprintf_s(aux, 20, L"%d", (int) rs->WXmin);
			SetDlgItemText(hwndDlg, IDC_EDIT1, aux);
			swprintf_s(aux, 20, L"%d", (int) rs->WXmax);
			SetDlgItemText(hwndDlg, IDC_EDIT4, aux);
			swprintf_s(aux, 20, L"%d", (int) rs->WYmin);
			SetDlgItemText(hwndDlg, IDC_EDIT2, aux);
			swprintf_s(aux, 20, L"%d", (int) rs->WYmax);
			SetDlgItemText(hwndDlg, IDC_EDIT5, aux);
			swprintf_s(aux, 20, L"%d", (int) rs->WZmin);
			SetDlgItemText(hwndDlg, IDC_EDIT3, aux);
			swprintf_s(aux, 20, L"%d", (int) rs->WZmax);
			SetDlgItemText(hwndDlg, IDC_EDIT6, aux);
			swprintf_s(aux, 20, L"%d", (int) rs->texelSide);
			SetDlgItemText(hwndDlg, IDC_EDIT7, aux);
			swprintf_s(aux, 20, L"%d", (int) rs->personMaximumHeight);
			SetDlgItemText(hwndDlg, IDC_EDIT8, aux);
			swprintf_s(aux, 20, L"%d", (int) rs->personAverageHeight);
			SetDlgItemText(hwndDlg, IDC_EDIT13, aux);
			swprintf_s(aux, 20, L"%d", (int) rs->personAverageWidth);
			SetDlgItemText(hwndDlg, IDC_EDIT14, aux);
			swprintf_s(aux, 20, L"%d", (int) rs->personMimimumHeight);
			SetDlgItemText(hwndDlg, IDC_EDIT15, aux);
			return TRUE;
		case WM_COMMAND:
			if (LOWORD(wParam)==IDCANCEL) {
				EndDialog(hwndDlg, 0);
			} else if (LOWORD(wParam)==IDOK) {
				GetDlgItemText(hwndDlg, IDC_EDIT1, aux, 20);
				rs->WXmin = (float) _wtof(aux);
				GetDlgItemText(hwndDlg, IDC_EDIT4, aux, 20);
				rs->WXmax = (float) _wtof(aux);
				GetDlgItemText(hwndDlg, IDC_EDIT2, aux, 20);
				rs->WYmin = (float) _wtof(aux);
				GetDlgItemText(hwndDlg, IDC_EDIT5, aux, 20);
				rs->WYmax = (float) _wtof(aux);
				GetDlgItemText(hwndDlg, IDC_EDIT3, aux, 20);
				rs->WZmin = (float) _wtof(aux);
				GetDlgItemText(hwndDlg, IDC_EDIT6, aux, 20);
				rs->WZmax = (float) _wtof(aux);
				GetDlgItemText(hwndDlg, IDC_EDIT7, aux, 20);
				rs->texelSide = (float) _wtof(aux);
				GetDlgItemText(hwndDlg, IDC_EDIT8, aux, 20);
				rs->personMaximumHeight = (float) _wtof(aux);
				GetDlgItemText(hwndDlg, IDC_EDIT13, aux, 20);
				rs->personAverageHeight = (float) _wtof(aux);
				GetDlgItemText(hwndDlg, IDC_EDIT14, aux, 20);
				rs->personAverageWidth = (float) _wtof(aux);
				GetDlgItemText(hwndDlg, IDC_EDIT15, aux, 20);
				rs->personMimimumHeight = (float) _wtof(aux);
				EndDialog(hwndDlg, 0);
			}
			return TRUE;
	}

	return FALSE;
}

void RoomSettings::OpenDialogWindow(HWND parentHwnd) {
	DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ROOMDATA), parentHwnd, RoomSettings::DlgProc);
}