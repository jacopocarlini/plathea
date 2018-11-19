// ClientDLL.cpp: definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include <iostream>
#include "PlatheaDLL.h"
#include <vector>
#include <time.h>
/*
#include <LeoWindowsGDI.h>
#include <LeoWindowsOpenCV.h>
#include <LeoLog4CPP.h>
#include <LeoWindowsGUI.h>
#include <LeoOpenCVExtension.h>
*/



using namespace std;

int main()
{	
	char str[] = "D:\\PLaTHEATest\\experiment.xml ";
	
	
	try {
		system_loadconfigurationfile(str);
	}
	catch (exception e) {
		cout << e.what();
		}
	
	internalCalibrationDemo("D:\\PLaTHEATest\\InternalCalibration");
	calibration_loadexternalcalibrationdata("D:\\PLaTHEATest\\ExternalCalibration");
	localizationengine_selectsvmclassifier("D:\\PLaTHEATest\\Tracking\\svmclassifier.xml");
	initializeSystemDemo();
	localizationengine_startlocalizationengine(false, false, "");
	test_plathearecorder();
	
	const char* s = "D:\\PLaTHEATest\\Tests\\21-12-2012 - 11-25-10-165";
	ElaborationCore* ec=getElaborationCore();
	printf("ec: %d\n",ec);
	test_plathearecorder_start(s);
	//test_plathearecorder_stop();
	
	std::vector<TrackedObject*> tracked = getTrackedPeople();
	printf("\ntracked: %d\n",tracked.size());
	
	for (std::vector<TrackedObject*>::iterator it = tracked.begin(); it != tracked.end(); ++it)
			std::cout << ' ' << *it;

	//test_plathearecorder_stop();
	
	cout << "Done!" << endl;
	getchar();
	return 0;
}


