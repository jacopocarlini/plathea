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


	Point p = *createPoint();
	p.printPoint();
	cout << "Done!" << endl;
	return 0;
}
