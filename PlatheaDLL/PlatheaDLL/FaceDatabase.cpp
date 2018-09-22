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

#include "FaceDatabase.h"
#include "Shared.h"

#include <LeoWindowsGDI.h>
#include <LeoWindowsGUI.h>
#include <LeoWindowsOpenCV.h>
#include <LeoWindowsConsole.h>
#include <Shlobj.h>
#include "ResourceManager.h"

#include <tinyxml.h>

#include "NetworkCamera.h"

CvMemStorage *storage = NULL;
CvHaarClassifierCascade *cascade = NULL;
IplImage *grayScale = NULL, *preSizedImage = NULL, *colorImage = NULL;
static HBITMAP preSizedImageHBITMAP = NULL, grayScaleHBITMAP = NULL; HDC memDC = NULL;
HWND preSizedImageStatic = NULL, grayScaleStatic = NULL, imageFaceFileNameEdit = NULL, faceDatabaseWindow = NULL;

HWND currentImageStatic;
int currentlyDisplayedFaceIndex;

static Person underConstructionSubject;

extern SystemInfo *si;

bool acquireFromVideoInput = false;

void ShowCurrentlySelectedFace() {
	wchar_t textBuf[16]; swprintf_s(textBuf, 16, L"%d/%d", currentlyDisplayedFaceIndex + 1, (int) underConstructionSubject.size());
	SetWindowText(currentImageStatic, textBuf);
	if (currentlyDisplayedFaceIndex > -1) {
		if (underConstructionSubject[currentlyDisplayedFaceIndex].GetFileName()) {
			SetWindowTextA(imageFaceFileNameEdit, underConstructionSubject[currentlyDisplayedFaceIndex].GetFileName());
		} else {
			SetWindowText(imageFaceFileNameEdit, L"<Stored into default directory>");
		}
		cvCopy(underConstructionSubject[currentlyDisplayedFaceIndex].image, preSizedImage);
		draw_features(preSizedImage, underConstructionSubject[currentlyDisplayedFaceIndex].features,
			underConstructionSubject[currentlyDisplayedFaceIndex].total);
		preSizedImageHBITMAP = IplImage2HBITMAP(preSizedImageStatic, preSizedImageHBITMAP, preSizedImage);
		RedrawStaticControl(&preSizedImageStatic, 1, false, false);
	} else {
		SetWindowText(imageFaceFileNameEdit, L"");
		if (preSizedImageHBITMAP)
			DeleteObject(preSizedImageHBITMAP);
		preSizedImageHBITMAP = NULL;
		RedrawStaticControl(&preSizedImageStatic, 1, true, false);
	}
}

void InsertUsersIntoCombo(HWND comboHandle) {
	wchar_t buf[256], previouslySelectedUser[256];
	ComboBox_GetText(comboHandle, previouslySelectedUser, 256);
	ComboBox_ResetContent(comboHandle);
	for (std::unordered_map<int, Person>::const_iterator it = mainFaceDatabase.begin(); it != mainFaceDatabase.end(); it++) {
		swprintf_s(buf, 256, L"%S", it->second.name);
		ComboBox_AddString(comboHandle, buf);
	}
	if (ComboBox_FindString(comboHandle, 0, previouslySelectedUser) != CB_ERR)
		ComboBox_SetText(comboHandle, previouslySelectedUser);
	else
		ComboBox_SetText(comboHandle, L"");
}

static HANDLE hTakeImage = CreateEvent(NULL, FALSE, FALSE, NULL);
static bool testEnabled = false;

static HANDLE videoInputImageReady = CreateEvent(NULL, FALSE, FALSE, NULL);

static cv::VideoCapture *cameraCapture = NULL;

VOID CALLBACK GrabberCallback(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
//void GrabberCallback(void * pBits, int width, int height, int size) {
	//cv::namedWindow("Ciao");
	IplImage *currentColorImage = NULL;
	if (acquireFromVideoInput) {
		if (WaitForSingleObject(videoInputImageReady, 0) == WAIT_OBJECT_0) {
			si->GetStereoRig()->StereoLock.AcquireReadLock();
			IplImage *leftImage = NULL; si->GetStereoRig()->GetStereoImages(&leftImage, NULL, true);
			currentColorImage = cvCloneImage(leftImage);
			si->GetStereoRig()->StereoLock.ReleaseReadLock();
		}
	} else {
		//currentColorImage = cvQueryFrame(cameraCapture);
		cv::Mat currentColorImageMat;
		(*cameraCapture) >> currentColorImageMat;
		IplImage currentColorImageIpl = currentColorImageMat;
		currentColorImage = cvCreateImage(cvSize(currentColorImageMat.cols, currentColorImageMat.rows), 8, 3);
		cvCopy(&currentColorImageIpl, currentColorImage);
		//cv::imshow("Ciao", currentColorImageMat);
	}
	if (currentColorImage == NULL)
		return;
	if (!grayScale) {
		grayScale = cvCreateImage(cvSize(currentColorImage->width, currentColorImage->height), IPL_DEPTH_8U, 1);
	}
	if (!colorImage) {
		colorImage = cvCreateImage(cvSize(currentColorImage->width, currentColorImage->height), IPL_DEPTH_8U, 3);
	}
	//memcpy(colorImage->imageData, pBits, size);
	//cvConvertImage(colorImage, colorImage, CV_CVTIMG_FLIP);
	cvResize(currentColorImage, colorImage, CV_INTER_AREA);
	if (acquireFromVideoInput)
		cvReleaseImage(&currentColorImage);
	cvCvtColor(colorImage, grayScale, CV_BGR2GRAY);
	cvEqualizeHist(grayScale, grayScale);
	cvClearMemStorage(storage);
	CvSeq *faces = cvHaarDetectObjects(grayScale, cascade, storage, 1.1, 3,
		(testEnabled ? 0 : CV_HAAR_FIND_BIGGEST_OBJECT), cvSize(40, 40), cvSize(150,150));
	
	CvFont selectedFont; cvInitFont(&selectedFont, CV_FONT_HERSHEY_PLAIN, 1.0, 1.0, 0.0, 2, CV_AA);
	for (int i = 0; i < (faces ? faces->total : 0); i++) {
		CvRect *seqElem = (CvRect *) cvGetSeqElem(faces, i);
		cvRectangle(grayScale, cvPoint(seqElem->x, seqElem->y),
			cvPoint(seqElem->x + seqElem->width - 1, seqElem->y + seqElem->height - 1), cvScalar(255));
		
		if (testEnabled) {
			cvSetImageROI(colorImage, *seqElem);
			cvResize(colorImage, preSizedImage, CV_INTER_CUBIC);
			cvResetImageROI(colorImage);
			int bestPersonScore;
			int bestPerson = mainFaceDatabase.FaceMatch(preSizedImage, &bestPersonScore);
			if (bestPerson >= 0 && bestPersonScore > 0) {
				char searchResult[128];
				sprintf_s(searchResult, "%s: %d", mainFaceDatabase[bestPerson].name, bestPersonScore);
				cvPutText(grayScale, searchResult, cvPoint(seqElem->x, seqElem->y), &selectedFont, cvScalar(255));
			}
		}
	}
	
	grayScaleHBITMAP = IplImage2HBITMAP(grayScaleStatic, grayScaleHBITMAP, grayScale);
	RedrawStaticControl(&grayScaleStatic, 1, false, false);
	DWORD waitResult = WaitForSingleObject(hTakeImage, 0);
	if (waitResult == WAIT_OBJECT_0 && faces && faces->total) {
		HCURSOR currentCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
		CvRect *seqElem = (CvRect *) cvGetSeqElem(faces, 0);
		cvSetImageROI(colorImage, *seqElem);
		cvResize(colorImage, preSizedImage, CV_INTER_CUBIC);
		cvResetImageROI(colorImage);
		TrainingFace* trainingFace = &TrainingFace(preSizedImage);
		int faceIndex = underConstructionSubject.AddTrainingFace(*trainingFace);
		currentlyDisplayedFaceIndex = faceIndex;
		if (underConstructionSubject[faceIndex].features) {
			ShowCurrentlySelectedFace();
		}
		SetCursor(currentCursor);
	}
}
/*
INT_PTR CALLBACK FaceDBaseDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static int selectedAcquisitionCamera = 0;
	wchar_t buf[256]; char buf_c[256];
	HWND comboHandle = GetDlgItem(hwndDlg, IDC_COMBO1);
	switch (uMsg) {
		case WM_INITDIALOG:
			{
				InsertUsersIntoCombo(comboHandle);
				faceDatabaseWindow = hwndDlg;
				preSizedImageStatic = GetDlgItem(hwndDlg, IDC_PRESIZEDSTATIC);
				grayScaleStatic = GetDlgItem(hwndDlg, IDC_CAPTURESTATIC);
				imageFaceFileNameEdit = GetDlgItem(hwndDlg, IDC_FACEFILENAMEEDIT);
				HDC frameDC = GetDC(grayScaleStatic);
				memDC = CreateCompatibleDC(frameDC);
				ReleaseDC(grayScaleStatic, frameDC);
				//cameraCapture = cvCaptureFromCAM(selectedAcquisitionCamera);
				cameraCapture = new cv::VideoCapture(selectedAcquisitionCamera);
				SetTimer(hwndDlg, 1, 100, GrabberCallback);
			
				preSizedImage = cvCreateImage(cvSize(150, 150), IPL_DEPTH_8U, 3);
				storage = cvCreateMemStorage(0);
				cascade = (CvHaarClassifierCascade *) cvLoad(mainFaceDatabase.GetHaarClassifierFilename());
				currentImageStatic = GetDlgItem(hwndDlg, IDC_CURRENTFACESTATIC);
				currentlyDisplayedFaceIndex = -1;
				SetDlgItemText(hwndDlg, IDC_CURRENTFACESTATIC, L"0/0");
				SetDlgItemTextA(hwndDlg, IDC_PICDIRECTORYEDIT, mainFaceDatabase.GetFaceDatabaseDirectory());
				return TRUE;
			}
		case WM_CLOSE:
			{
				HCURSOR defaultCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
				KillTimer(hwndDlg, 1);
				acquireFromVideoInput = false;
				if (si->GetStereoRig() != NULL)
					si->GetStereoRig()->UnSubscribeEvent(STEREO_IMAGE_READY,videoInputImageReady);
				//cvReleaseCapture(&cameraCapture);
				cameraCapture->release();
				//GrabberStop();
				//GrabberClose();
				cvReleaseMemStorage(&storage);
				cvReleaseHaarClassifierCascade(&cascade);
				cvReleaseImage(&grayScale);
				cvReleaseImage(&preSizedImage);
				cvReleaseImage(&colorImage);
				if (preSizedImageHBITMAP)
					DeleteObject(preSizedImageHBITMAP);
				preSizedImageHBITMAP = NULL;
				if (grayScaleHBITMAP)
					DeleteObject(grayScaleHBITMAP);
				grayScaleHBITMAP = NULL;
				DeleteDC(memDC);
				underConstructionSubject.clear();
				strcpy_s(underConstructionSubject.name, 256, "");
				SetCursor(defaultCursor);
				EndDialog(hwndDlg, 0);
			}
			return TRUE;
		case WM_COMMAND:
			{
				WORD button = LOWORD(wParam);
				ComboBox_GetText(comboHandle, buf, 256);
				sprintf_s(buf_c, 256, "%S", buf);
				trim(buf_c);
				int userIndex = mainFaceDatabase.FindPersonIndexByName(buf_c);
				if (button == IDC_BUTTONDELUSER) {
					if (mainFaceDatabase.RemovePersonFromDBase(buf_c)) {
						InsertUsersIntoCombo(comboHandle);
					} else {
						MessageBox(hwndDlg, L"Unable to find selected user", L"Error", MB_OK | MB_ICONWARNING);
					}
				} else if (button == IDC_ACQUIREFACE) {
					SetEvent(hTakeImage);
				} else if (button == IDC_BUTTONPREVIOUSFACE || button == IDC_BUTTONNEXTFACE) {
					if (button == IDC_BUTTONPREVIOUSFACE) {
						currentlyDisplayedFaceIndex = MAX(0, currentlyDisplayedFaceIndex - 1);
					} else {
						currentlyDisplayedFaceIndex = MIN(int(underConstructionSubject.size()) - 1, currentlyDisplayedFaceIndex + 1);
					}
					ShowCurrentlySelectedFace();
				} else if (button == IDC_BUTTONDELFACE) {
					if (currentlyDisplayedFaceIndex > -1) {
						underConstructionSubject.erase(underConstructionSubject.begin() + currentlyDisplayedFaceIndex);
						currentlyDisplayedFaceIndex = MIN(currentlyDisplayedFaceIndex, int(underConstructionSubject.size()) - 1);
						ShowCurrentlySelectedFace();
					}
				} else if (button == IDC_BUTTONNEWUSER) {
					int createNewUser = MessageBox(hwndDlg, L"All unsaved data will be lost. Do you want to continue?", L"Warning", MB_YESNO | MB_ICONQUESTION);
					if (createNewUser = IDYES) {
						ComboBox_SetText(comboHandle, L"");
						underConstructionSubject.clear();
						currentlyDisplayedFaceIndex = -1;
						ShowCurrentlySelectedFace();
					}
				} else if (button == IDC_BUTTONLOADUSER) {
					if (userIndex == -1) {
						MessageBox(hwndDlg, L"The selected user does not exist", L"Warning", MB_OK | MB_ICONWARNING);
					} else {
						underConstructionSubject = mainFaceDatabase[userIndex];
						currentlyDisplayedFaceIndex = int(underConstructionSubject.size()) - 1;
						ShowCurrentlySelectedFace();
						swprintf_s(buf, 256, L"%d", underConstructionSubject.height);
						SetDlgItemText(hwndDlg, IDC_PERSONEDIT, buf);
						swprintf_s(buf, 256, L"%d", underConstructionSubject.shoulders);
						SetDlgItemText(hwndDlg, IDC_PERSONEDIT2, buf);
						swprintf_s(buf, 256, L"%d", underConstructionSubject.chest);
						SetDlgItemText(hwndDlg, IDC_PERSONEDIT3, buf);
					}
				} else if (button == IDC_BUTTONSAVEUSER) {
					if (strcmp(buf_c, "") == 0) {
						MessageBox(hwndDlg, L"Invalid user name", L"Warning", MB_OK | MB_ICONERROR);
					} else {
						int replaceUser = IDYES;
						if (userIndex >= 0) {
							replaceUser = MessageBox(hwndDlg, L"The selected user already exists. Replace it?", L"Warning", MB_YESNO | MB_ICONQUESTION);
						}
						if (replaceUser == IDYES) {
							strcpy_s(underConstructionSubject.name, sizeof(underConstructionSubject.name), buf_c);
							GetDlgItemText(hwndDlg, IDC_PERSONEDIT, buf, 256);
							swscanf_s(buf, L"%d", &underConstructionSubject.height);
							GetDlgItemText(hwndDlg, IDC_PERSONEDIT2, buf, 256);
							swscanf_s(buf, L"%d", &underConstructionSubject.shoulders);
							GetDlgItemText(hwndDlg, IDC_PERSONEDIT3, buf, 256);
							swscanf_s(buf, L"%d", &underConstructionSubject.chest);
							mainFaceDatabase.AddPersonToDBase(underConstructionSubject);
							InsertUsersIntoCombo(comboHandle);
						}
					}
				} else if (button == IDC_BROWSEPICDIRECTORY) {
					char buf[MAX_PATH];
					if (UseCommonItemDialog(buf, sizeof(buf), hwndDlg, CLSID_FileOpenDialog, FOS_PICKFOLDERS)) {
						mainFaceDatabase.SetFaceDatabaseDirectory(buf);
						SetDlgItemTextA(hwndDlg, IDC_PICDIRECTORYEDIT, buf);
					}
				} else if (button == IDC_ACQUIREFACEFROMURI) {
					char buf[MAX_PATH];
					COMDLG_FILTERSPEC filtro[] = {L"JPG Face Picture", L"*.jpg"};
					if (UseCommonItemDialog(buf, sizeof(buf), hwndDlg, CLSID_FileOpenDialog, 0, filtro, 1)) {
						std::string fileName = ResourceManager::GetLocalURL(buf);
						int faceIndex = underConstructionSubject.AddTrainingFace(TrainingFace(fileName.c_str(), buf, true));
						currentlyDisplayedFaceIndex = faceIndex;
						ShowCurrentlySelectedFace();
					}
				} else if (button == IDC_FACERECOGNITIONTEST) {
					testEnabled = !testEnabled;
				} else if (button == IDC_BUTTON1 || button == IDC_BUTTON3) {
					HCURSOR defaultCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
				
					int lastFunctioningCamera = selectedAcquisitionCamera;
					if (button == IDC_BUTTON1) {
						selectedAcquisitionCamera++;
					} else if (button == IDC_BUTTON3 && selectedAcquisitionCamera > 0) {
						selectedAcquisitionCamera--;
					}
					if (selectedAcquisitionCamera != lastFunctioningCamera) {
						KillTimer(hwndDlg, 1);
						acquireFromVideoInput = false;
						if (si->GetStereoRig() != NULL)
							si->GetStereoRig()->UnSubscribeEvent(STEREO_IMAGE_READY,videoInputImageReady);
						//cvReleaseCapture(&cameraCapture);
						cameraCapture->release();
						cameraCapture = new cv::VideoCapture(selectedAcquisitionCamera);
						
						if (cameraCapture == NULL) {
							cameraCapture = new cv::VideoCapture(selectedAcquisitionCamera);
							selectedAcquisitionCamera = lastFunctioningCamera;
						}

						SetTimer(hwndDlg, 1, 100, GrabberCallback);
					}
					SetCursor(defaultCursor);
				} else if (button == IDC_BUTTON5) {
					KillTimer(hwndDlg, 1);
					acquireFromVideoInput = true;
					if (cameraCapture != NULL) {
						//cvReleaseCapture(&cameraCapture);
						cameraCapture->release();
					}
					selectedAcquisitionCamera = 0;
					ResetEvent(videoInputImageReady);
					if (si->GetStereoRig() != NULL)
						si->GetStereoRig()->SubscribeEvent(STEREO_IMAGE_READY, videoInputImageReady);
					SetTimer(hwndDlg, 1, 100, GrabberCallback);
				}
			}
			return TRUE;
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC windowHDC = BeginPaint(hwndDlg, &ps);

				RECT frameClientRect; GetClientRect(grayScaleStatic, &frameClientRect);
				MapWindowPoints(grayScaleStatic, hwndDlg, (POINT *) &frameClientRect, 2);
				if (AreRectIntersected(RECTToLeoRect(frameClientRect), RECTToLeoRect(ps.rcPaint)) && grayScaleHBITMAP) {
					SelectObject(memDC, grayScaleHBITMAP);
					AdaptiveBitBlt(ps.hdc, frameClientRect.left, frameClientRect.top, frameClientRect.right - frameClientRect.left,
						frameClientRect.bottom - frameClientRect.top, memDC, grayScale->width, grayScale->height);
				}

				GetClientRect(preSizedImageStatic, &frameClientRect);
				MapWindowPoints(preSizedImageStatic, hwndDlg, (POINT *) &frameClientRect, 2);
				if (AreRectIntersected(RECTToLeoRect(frameClientRect), RECTToLeoRect(ps.rcPaint)) && preSizedImageHBITMAP) {
					SelectObject(memDC, preSizedImageHBITMAP);
					AdaptiveBitBlt(ps.hdc, frameClientRect.left, frameClientRect.top, frameClientRect.right - frameClientRect.left,
						frameClientRect.bottom - frameClientRect.top, memDC, preSizedImage->width, preSizedImage->height);
				}

				EndPaint(hwndDlg, &ps);
			}
			return TRUE;
	}
	return FALSE;
}
*/

int intvls = SIFT_INTVLS;
double sigma = SIFT_SIGMA;
double contr_thr = SIFT_CONTR_THR;
int curv_thr = SIFT_CURV_THR;
int img_dbl = SIFT_IMG_DBL;
int descr_width = SIFT_DESCR_WIDTH;
int descr_hist_bins = SIFT_DESCR_HIST_BINS;

int KDTREE_BBF_MAX_NN_CHKS = 200;
double NN_SQ_DIST_RATIO_THR = 0.49;

void TrainingFace::DeleteSIFTStructures() {
	//writeToConsole(L"DeleteData\r\n");
	if (features)
		free(features);
	if (kdRoot)
		kdtree_release(kdRoot);
}

void TrainingFace::CreateSIFTStructures() {
	if (!image)
		return;
	DeleteSIFTStructures();
	total = _sift_features(image, &features, intvls, sigma, contr_thr, curv_thr,
		img_dbl, descr_width, descr_hist_bins);
	if (total > 0) {
		kdRoot = kdtree_build(features, total);
	}
}

Person::Person(void *xmlElem) {
	char c_buf[MAX_PATH];
	TiXmlElement *elem = (TiXmlElement *) xmlElem;
	sprintf_s(name, 256, "%s", elem->Attribute("name"));
	sscanf_s(elem->Attribute("Height"), "%d", &height);
	sscanf_s(elem->Attribute("Shoulders"), "%d", &shoulders);
	sscanf_s(elem->Attribute("Chest"), "%d", &chest);

	for (TiXmlElement *faceElem = elem->FirstChildElement("Face"); faceElem != NULL; faceElem = faceElem->NextSiblingElement("Face")) {
		bool externalImage = (strcmp(faceElem->Attribute("externalImage"), "true") == 0);
		//Only file image is supported at this time
		sprintf_s(c_buf, MAX_PATH, "%s", faceElem->Attribute("uri"));
		push_back(TrainingFace(ResourceManager::GetLocalURL(c_buf).c_str(), c_buf, externalImage));
		/*int numberOfEncodedChars = int(wcslen(faceElem->getTextContent()));
		char *faceBuffer = new char[numberOfEncodedChars + 1];
		sprintf_s(faceBuffer, numberOfEncodedChars + 1, "%S", faceElem->getTextContent());
		char *imageBytes; decodeBase64(faceBuffer, numberOfEncodedChars, &imageBytes);
		delete[] faceBuffer;
		IplImage *image = cvCreateImageHeader(cvSize(150, 150), IPL_DEPTH_8U, 3);
		image->imageData = imageBytes;
		push_back(TrainingFace(image));
		cvReleaseImageHeader(&image);
		delete[] imageBytes;*/
	}
}

int Person::AddTrainingFace(TrainingFace &tf) {
	if (tf.total > 0 && tf.kdRoot) {
		push_back(tf);
		return int(size()) - 1;
	} else {
		return -1;
	}
}

void Person::DeleteData() {
	clear();
}

void Person::toXMLElement(void *xmlElem) const {
	char buf_c[256];
	char xmlFileName[256];
	TiXmlElement *elem = (TiXmlElement *) xmlElem;
	elem->SetAttribute("name", name);
	sprintf_s(buf_c, 256, "%d", height);
	elem->SetAttribute("Height", buf_c);
	sprintf_s(buf_c, 256, "%d", shoulders);
	elem->SetAttribute("Shoulders", buf_c);
	sprintf_s(buf_c, 256, "%d", chest);
	elem->SetAttribute("Chest", buf_c);
	int systemCapturedImageIndex = 0;
	for (std::vector<TrainingFace>::const_iterator it = begin(); it != end(); it++) {
		TiXmlElement *faceElement = new TiXmlElement("Face");
		if (!it->externalImage) {
			sprintf_s(buf_c, 256, "%s\\%s - %d.bmp", mainFaceDatabase.GetFaceDatabaseDirectory(), name, systemCapturedImageIndex++);
			cvSaveImage(buf_c, it->image);
			sprintf_s(xmlFileName, 256, "file://%s", buf_c);
		} else {
			sprintf_s(xmlFileName, 256, "%s", it->filename);
		}
		faceElement->SetAttribute("externalImage", (it->externalImage ? "true" : "false"));
		faceElement->SetAttribute("uri", xmlFileName);
		/*char *base64EncodedImage; int strLength = encodeBase64(it->image->imageData, it->image->imageSize, &base64EncodedImage);
		wchar_t *wideMemory = new wchar_t[strLength + 1];
		swprintf_s(wideMemory, strLength + 1, L"%S", base64EncodedImage);
		delete[] base64EncodedImage;
		base64Face->setTextContent(wideMemory);
		delete[] wideMemory;*/
		elem->LinkEndChild(faceElement);
	}
}

FaceDatabase mainFaceDatabase;

FaceDatabase::FaceDatabase(): ParameterSetter("FaceDatabase") {
	currentPersonID = 0;
	haarFileName[0] = '\0';
	wchar_t *folderName;
	SHGetKnownFolderPath(FOLDERID_Desktop, 0, NULL, &folderName);
	sprintf_s(faceDatabaseDirectory, MAX_PATH, "%S", folderName);
	CoTaskMemFree(folderName);
}

FaceDatabase::~FaceDatabase() {
	ClearMemory();
}

bool FaceDatabase::PrerequisitesCheck(wchar_t *errMsg, int bufferSize, bool *warning) {
	WIN32_FIND_DATAA findStructure;
	if (FindFirstFileA(haarFileName, &findStructure) == INVALID_HANDLE_VALUE) {
		if (errMsg) {
			wcscpy_s(errMsg, bufferSize, L"The haar cascade filename is not valid.");
		}
		return false;
	}
	return true;
}

int FaceDatabase::FindPersonIndexByName(char *name) {
	for (std::unordered_map<int, Person>::const_iterator it = begin(); it != end(); it++) {
		if (strcmp(it->second.name, name) == 0) {
			return it->first;
		}
	}
	return -1;
}

void FaceDatabase::ClearMemory() {
	for (std::unordered_map<int, Person>::iterator it = begin(); it != end(); it++) {
		it->second.DeleteData();
	}
	clear();
}

int FaceDatabase::AddPersonToDBase(Person &p) {
	int index = FindPersonIndexByName(p.name);
	if (index == -1)
		index = currentPersonID++;
	(*this)[index] = p;
	return index;
}

bool FaceDatabase::RemovePersonFromDBase(char *name) {
	int index = FindPersonIndexByName(name);
	if (index == -1)
		return false;
	(*this)[index].DeleteData();
	erase(find(index));
	return true;
}

int FaceDatabase::FaceMatch(IplImage *face, int *score) {
	feature *features;
	int n = _sift_features(face, &features, intvls, sigma, contr_thr, curv_thr,
		img_dbl, descr_width, descr_hist_bins);
	if (n > 0) {
		int bestPerson = -1, bestPersonScore = -1;
		for (std::unordered_map<int, Person>::iterator person_it = begin(); person_it != end(); person_it++) {
			int personScore = 0;
			for (std::vector<TrainingFace>::iterator face_it = person_it->second.begin(); face_it != person_it->second.end(); face_it++) {
				for (int a = 0; a < n; a++) {
					feature *feat = features + a;
					feature **nbrs;
					int k = kdtree_bbf_knn(face_it->kdRoot, feat, 2,
						&nbrs, KDTREE_BBF_MAX_NN_CHKS);
					if (k == 2) {
						double d0 = descr_dist_sq(feat, nbrs[0]);
						double d1 = descr_dist_sq(feat, nbrs[1]);
						if (d0 < 50000 && d0 < d1 * NN_SQ_DIST_RATIO_THR) {
							personScore++;
						}
					}
					if (k != -1)
						free(nbrs);
				}
			}
			if (personScore > bestPersonScore) {
				bestPersonScore = personScore;
				bestPerson = person_it->first;
			}
		}
		*score = bestPersonScore;
		free(features);
		return bestPerson;
	} else {
		free(features);
		return -1;
	}
}

char *FaceDatabase::GetHaarClassifierFilename() {
	return haarFileName;
}

void FaceDatabase::SetHaarClassifierFileName(const char *fileName) {
	strcpy_s(haarFileName, MAX_PATH, fileName);
}

bool FaceDatabase::LoadFromXMLElement(TiXmlElement *elem) {
	sprintf_s(haarFileName, MAX_PATH, "%s", elem->FirstChildElement("HaarClassifierFileName")->GetText());
	sprintf_s(faceDatabaseDirectory, MAX_PATH, "%s", elem->FirstChildElement("FaceDatabaseDirectory")->GetText());
	std::string localFileName = ResourceManager::GetLocalURL(haarFileName);
	strcpy_s(haarFileName, MAX_PATH, localFileName.c_str());
	sscanf_s(elem->FirstChildElement("NextFreeID")->GetText(), "%d", &currentPersonID);
	for (TiXmlElement *personElem = elem->FirstChildElement("Person"); personElem != NULL; personElem = personElem->NextSiblingElement("Person")) {
		int id; sscanf_s(personElem->Attribute("id"), "%d", &id);
		(*this)[id] = Person(personElem);
	}
	return true;
}

bool FaceDatabase::SaveToXMLElement(TiXmlElement *elem) const {
	char wideBuffer[256];

	TiXmlElement *haarFileElem = new TiXmlElement("HaarClassifierFileName");
	sprintf_s(wideBuffer, 256, "file://%s", haarFileName);
	haarFileElem->LinkEndChild(new TiXmlText(wideBuffer));
	elem->LinkEndChild(haarFileElem);

	TiXmlElement *nextFreeElem = new TiXmlElement("NextFreeID");
	sprintf_s(wideBuffer, 256, "%d", currentPersonID);
	nextFreeElem->LinkEndChild(new TiXmlText(wideBuffer));
	elem->LinkEndChild(nextFreeElem);

	TiXmlElement *faceDatabaseDirectoryElem = new TiXmlElement("FaceDatabaseDirectory");
	sprintf_s(wideBuffer, 256, "%s", faceDatabaseDirectory);
	faceDatabaseDirectoryElem->LinkEndChild(new TiXmlText(wideBuffer));
	elem->LinkEndChild(faceDatabaseDirectoryElem);

	for (std::unordered_map<int, Person>::const_iterator it = begin(); it != end(); it++) {
		TiXmlElement *personElem = new TiXmlElement("Person");
		sprintf_s(wideBuffer, 256, "%d", it->first);
		personElem->SetAttribute("id", wideBuffer);
		it->second.toXMLElement(personElem);
		elem->LinkEndChild(personElem);
	}

	return true;
}