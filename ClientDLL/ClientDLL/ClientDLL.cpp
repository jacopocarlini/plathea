// ClientDLL.cpp: definisce il punto di ingresso dell'applicazione console.
//

#include "stdafx.h"
#include <iostream>
#include "PlatheaDLL.h"
#include <vector>

using namespace std;

int main()
{	
 
	system_loadconfigurationfile("C:\\Users\\jack1\\Desktop\\PLaTHEATest\\");
	internalCalibrationDemo("C:\\Users\\jack1\\Desktop\\PLaTHEATest\\InternalCalibration");
	calibration_loadexternalcalibrationdata("C:\\Users\\jack1\\Desktop\\PLaTHEATest\\ExternalCalibration");
	localizationengine_selectsvmclassifier("C:\\Users\\jack1\\Desktop\\PLaTHEATest\\Tracking\\");
	initializeSystemDemo();
	localizationengine_startlocalizationengine(false, false, "");
	test_plathearecorder();
	const char* s = "D:\\PLaTHEATest\\Tests\\21-12-2012 - 11-25-10-165";
	ElaborationCore* ec=getElaborationCore();
	//printf("ec: %d\n",ec);
	//test_plathearecorder_start(s);
	//test_plathearecorder_stop();
	std::vector<TrackedObject*> tracked = getTrackedPeople();
	//printf("\ntracked: %d\n",tracked);
	
	for (std::vector<TrackedObject*>::iterator it = tracked.begin(); it != tracked.end(); ++it)
	//	std::cout << ' ' << *it;
	
	
	//cout << "Done!" << endl;
	return 0;
}
