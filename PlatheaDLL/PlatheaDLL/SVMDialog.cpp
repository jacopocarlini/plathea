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

#include "SVMDialog.h"
#include "MLDialog.h"
#include "Shared.h"
#include "PlanViewMap.h"

#include <LeoLog4CPP.h>
#include <LeoWindowsGUI.h>

using namespace leostorm::logging;

static CvMat *currentTrainingData = NULL, *currentTrainingClass = NULL;

static HWND svmDialogHwnd = NULL;

static cv::ml::SVM *theSvm;

SVMResult::SVMResult() {
	svm = cv::ml::SVM::create();
}

SVMResult::~SVMResult() {
	if (svm)
		delete svm;
}

int CreateFold(CvMat *trainingData, CvMat *trainingClass, CvMat ***folding, int numberOfFolds) {
	//Shuffle data a little bit
	srand((unsigned int) GetTickCount());
	for (int i = 0; i < trainingData->rows / 2; i++) {
		int r_a = rand() % trainingData->rows;
		int r_b = r_a;
		do {
			r_b = rand() % trainingData->rows;
		} while (r_b == r_a);
		float *data_a = (float *) (trainingData->data.ptr + r_a * trainingData->step);
		float *class_a = (float *) (trainingClass->data.ptr + r_a * trainingClass->step);
		float *data_b = (float *) (trainingData->data.ptr + r_b * trainingData->step);
		float *class_b = (float *) (trainingClass->data.ptr + r_b * trainingClass->step);
		float dataBuffer[3]; float classBuffer; memcpy(dataBuffer, data_b, 3 * sizeof(float)); classBuffer = class_b[0];
		memcpy(data_b, data_a, 3 * sizeof(float)); class_b[0] = class_a[0];
		memcpy(data_a, dataBuffer, 3 * sizeof(float)); class_a[0] = classBuffer;
	}

	int numberOfElementsPerFold = int(floor(float(trainingData->rows) / float(numberOfFolds)));
	*folding = new CvMat*[numberOfFolds * 4];
	CvMat** foldingPtr = *folding;
	for (int k = 0; k < numberOfFolds; k++, foldingPtr += 4) {
		foldingPtr[0] = cvCreateMat(trainingData->rows - numberOfElementsPerFold, 3, CV_32FC1);
		foldingPtr[1] = cvCreateMat(trainingData->rows - numberOfElementsPerFold, 1, CV_32FC1);
		foldingPtr[2] = cvCreateMat(numberOfElementsPerFold, 3, CV_32FC1);
		foldingPtr[3] = cvCreateMat(numberOfElementsPerFold, 1, CV_32FC1);
		float *destTrainingDataPtr = foldingPtr[0]->data.fl;
		float *destTrainingClassPtr = foldingPtr[1]->data.fl;
		float *destTestDataPtr = foldingPtr[2]->data.fl;
		float *destTestClassPtr = foldingPtr[3]->data.fl;
		float *trainingDataPtr = trainingData->data.fl;
		float *trainingClassPtr = trainingClass->data.fl;
		int minimumIndex = k * numberOfElementsPerFold; int maximumIndex = minimumIndex + numberOfElementsPerFold - 1;
		for (int i = 0; i < trainingData->rows; i++) {
			if (i < minimumIndex || i > maximumIndex) {
				memcpy(destTrainingDataPtr, trainingDataPtr, 3*sizeof(float));
				memcpy(destTrainingClassPtr, trainingClassPtr, sizeof(float));
				destTrainingDataPtr = (float *) ((uchar *) destTrainingDataPtr + foldingPtr[0]->step);
				destTrainingClassPtr = (float *) ((uchar *) destTrainingClassPtr + foldingPtr[1]->step);
			} else {
				memcpy(destTestDataPtr, trainingDataPtr, 3*sizeof(float));
				memcpy(destTestClassPtr, trainingClassPtr, sizeof(float));
				destTestDataPtr = (float *) ((uchar *) destTestDataPtr + foldingPtr[2]->step);
				destTestClassPtr = (float *) ((uchar *) destTestClassPtr + foldingPtr[3]->step);
			}
			trainingDataPtr = (float *) ((uchar *) trainingDataPtr + trainingData->step);
			trainingClassPtr = (float *) ((uchar *) trainingClassPtr + trainingClass->step);
		}
	}
	return numberOfElementsPerFold * numberOfFolds;
}

void DestroyFold(CvMat ***folding, int numberOfFolds) {
	CvMat **foldingPtr = *folding;
	for (int j = 0; j < numberOfFolds; j++, foldingPtr += 4) {
		for (int i = 0; i < 4; i++) {
			cvReleaseMat(foldingPtr + i);
		}
	}
	delete[] *folding;
	*folding = NULL;
}

SVMResult* SolveSVM(CvMat *trainingData, CvMat *trainingClass, double nu, double gamma, CvMat *testData, CvMat *testClass) {
	SVMResult* result = new SVMResult();

	result->svm->setNu(nu);
	result->svm->setGamma(gamma);
	result->svm->setKernel(cv::ml::SVM::RBF);
	result->svm->setType(cv::ml::SVM::NU_SVC); //CvSVM::C_SVC
	result->svm->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER, 100, 0.000001));

	bool res = result->svm->train(cv::cvarrToMat(trainingData), cv::ml::ROW_SAMPLE, cv::cvarrToMat(trainingClass));
	if (!res)
		return NULL;

	if (testData && testClass) {
		float *testDataPtr = testData->data.fl;
		float *testClassPtr = testClass->data.fl;
		result->numberOfHit = 0;
		for (int i = 0; i < testData->rows; i++) {
			CvMat sample = cvMat(1, 3, CV_32FC1, testDataPtr);
			float prediction = result->svm->predict(cv::cvarrToMat(&sample));
			if (prediction == *testClassPtr) {
				result->numberOfHit++;
			}
			testDataPtr = (float *) ((uchar *) testDataPtr + testData->step);
			testClassPtr = (float *) ((uchar *) testClassPtr + testClass->step);
		}
		result->accuracy =  float(result->numberOfHit) / float(testData->rows);
	}

	return result;
}

RBFSearchResult RBFOptimalKernelResearch(double minimumNu, double maximumNu, double stepNu, double minimumG, double maximumG, double stepG, int numberOfFolds) {
	RBFSearchResult result;
	int currentMaximum = 0;
	CvMat **folding;
	int totalTests = CreateFold(currentTrainingData, currentTrainingClass, &folding, numberOfFolds);
	int a = int((maximumNu - minimumNu) / stepNu) + 1;
	int b = int((maximumG - minimumG) / stepG) + 1;
	int numberOfIterations = a*b*numberOfFolds;
	//SendMessage(GetDlgItem(svmDialogHwnd, IDC_PROGRESS1), PBM_SETRANGE, (WPARAM) 0, MAKELPARAM(0, numberOfIterations));
	//SendMessage(GetDlgItem(svmDialogHwnd, IDC_PROGRESS1), PBM_SETSTEP, (WPARAM) 1, 0);
	for (double currentNu = minimumNu; currentNu <= maximumNu; currentNu += stepNu) {
		for (double currentG = minimumG; currentG <= maximumG; currentG += stepG) {
			int currentTotal = 0;
			CvMat **foldPtr = folding;
			bool noError = true;
			for (int i = 0; i < numberOfFolds && noError; i++, foldPtr += 4) {
				SVMResult* aux = SolveSVM(foldPtr[0], foldPtr[1], currentNu, pow(2.0, currentG), foldPtr[2], foldPtr[3]);
				if (aux) {
					currentTotal += aux->numberOfHit;
					//SendMessage(GetDlgItem(svmDialogHwnd, IDC_PROGRESS1), PBM_STEPIT, 0, 0);
					delete aux;
				} else {
					noError = false;
				}
			}
			if (noError && currentTotal > currentMaximum) {
				currentMaximum = currentTotal;
				result.nu = currentNu;
				result.gamma = currentG;
			}
			if (noError)
				Logger::writeToLOG(L"nu: %f log2(gamma): %f accuracy: %f\r\n", currentNu, currentG, double(currentTotal) / totalTests);
			else
				Logger::writeToLOG(L"nu: %f log2(gamma): %f accuracy: error\r\n", currentNu, currentG); 
		}
	}
	DestroyFold(&folding, numberOfFolds);
	//SendMessage(GetDlgItem(svmDialogHwnd, IDC_PROGRESS1), PBM_SETPOS, (WPARAM) 0, (LPARAM) 0);
	return result;
}

/*
static INT_PTR CALLBACK SVMDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	char buf[_MAX_PATH];
	wchar_t buffer[100];
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			//SetDlgItemText(hwndDlg, IDC_EDIT1, L"0.1");
			//SetDlgItemText(hwndDlg, IDC_EDIT2, L"0.8");
			//SetDlgItemText(hwndDlg, IDC_EDIT3, L"-15");
			//SetDlgItemText(hwndDlg, IDC_EDIT4, L"3");
			//SetDlgItemText(hwndDlg, IDC_EDIT5, L"0.2");
			//SetDlgItemText(hwndDlg, IDC_EDIT6, L"2");
			//SetDlgItemText(hwndDlg, IDC_EDIT7, L"10");
			svmDialogHwnd = hwndDlg;
		}
		return TRUE;
	case WM_COMMAND:
		{
			WORD messageId = HIWORD(wParam);
			WORD controlId = LOWORD(wParam);
			if (controlId == IDC_BUTTON1) {
				COMDLG_FILTERSPEC filtro[] = {L"ML Training Set File", L"*.mlt"};
				if (UseCommonItemDialog(buf, sizeof(buf), hwndDlg, CLSID_FileOpenDialog, 0, filtro, 1)) {
					HCURSOR originalCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
					TrainingDataFileToMemory(buf, &currentTrainingData, &currentTrainingClass);
					SetCursor(originalCursor);
				}
			} else if (controlId == IDC_BUTTON2) {
				if (currentTrainingData == NULL || currentTrainingClass == NULL) {
					MessageBox(hwndDlg, L"No training data have been loaded!", L"Error", MB_OK | MB_ICONERROR);
					return TRUE;
				}
				HCURSOR originalCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
				GetDlgItemText(hwndDlg, IDC_EDIT1, buffer, 10);
				double minimumNu = _wtof(buffer);
				GetDlgItemText(hwndDlg, IDC_EDIT2, buffer, 10);
				double maximumNu = _wtof(buffer);
				GetDlgItemText(hwndDlg, IDC_EDIT3, buffer, 10);
				double minimumGamma = _wtof(buffer);
				GetDlgItemText(hwndDlg, IDC_EDIT4, buffer, 10);
				double maximumGamma = _wtof(buffer);
				GetDlgItemText(hwndDlg, IDC_EDIT5, buffer, 10);
				double stepNu = _wtof(buffer);
				GetDlgItemText(hwndDlg, IDC_EDIT6, buffer, 10);
				double stepGamma = _wtof(buffer);
				GetDlgItemText(hwndDlg, IDC_EDIT7, buffer, 10);
				int numberOfFold = _wtoi(buffer);
				RBFSearchResult res = RBFOptimalKernelResearch(minimumNu, maximumNu, stepNu, minimumGamma, maximumGamma, stepGamma, numberOfFold);
				swprintf_s(buffer, 100, L"%f", res.nu);
				SetDlgItemText(hwndDlg, IDC_EDIT8, buffer);
				swprintf_s(buffer, 100, L"%f", res.gamma);
				SetDlgItemText(hwndDlg, IDC_EDIT9, buffer);
				SetCursor(originalCursor);
			} else if (controlId == IDC_BUTTON3) {
				if (currentTrainingData == NULL || currentTrainingClass == NULL) {
					MessageBox(hwndDlg, L"No training data have been loaded!", L"Error", MB_OK | MB_ICONERROR);
					return TRUE;
				}
				HCURSOR originalCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
				GetDlgItemText(hwndDlg, IDC_EDIT8, buffer, 10);
				double selectedNu = _wtof(buffer);
				GetDlgItemText(hwndDlg, IDC_EDIT9, buffer, 10);
				double selectedGamma = _wtof(buffer);
				SVMResult* res = SolveSVM(currentTrainingData, currentTrainingClass, selectedNu, pow(2.0, selectedGamma), NULL, NULL);
				if (res) {
					theSvm = res->svm;
					res->svm = NULL;
					delete res;
				}
				SetCursor(originalCursor);
				if (!res)
					MessageBox(hwndDlg, L"Unable to train with the selected parameters", L"Error", MB_OK | MB_ICONERROR);
			} else if (controlId == IDC_BUTTON6) {
				if (theSvm == NULL) {
					MessageBox(hwndDlg, L"No SVM classifier have been calculated.", L"Error", MB_OK | MB_ICONERROR);
					return TRUE;
				}
				COMDLG_FILTERSPEC filtro[] = {L"SVM Classifier", L"*.xml"};
				if (UseCommonItemDialog(buf, sizeof(buf), hwndDlg, CLSID_FileSaveDialog, 0, filtro, 1)) {
					HCURSOR originalCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
					theSvm->save(buf);
					SetCursor(originalCursor);
					if (MessageBox(hwndDlg, L"Do you want to set it as current classifier? Change will have effect after restarting processing.", L"Question", MB_YESNO | MB_ICONQUESTION) == IDYES) {
						PlanViewMap::SetSVMClassifierFileName(buf);
					}
				}
			}
		}
		return TRUE;
	case WM_CLOSE:
		{
			if (currentTrainingData)
				cvReleaseMat(&currentTrainingData);
			if (currentTrainingClass)
				cvReleaseMat(&currentTrainingClass);
			svmDialogHwnd = NULL;
			if (theSvm) {
				delete theSvm;
				theSvm = NULL;
			}
			EndDialog(hwndDlg, 0);
		}
		return TRUE;
	}
	return FALSE;
}
*/

void ShowSVMDialog(HWND parentWindow) {
	//DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SVMDIALOG), parentWindow, SVMDialogProc);
}

void ShowSVMDialog(HWND parentWindow, CvMat* trainingData, CvMat *trainingClass) {
	currentTrainingData = trainingData;
	currentTrainingClass = trainingClass;
	ShowSVMDialog(parentWindow);
}