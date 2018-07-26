// PLATHEADLL.h - Contains declarations 
#pragma once

#ifdef PLATHEADLL_EXPORTS
#define PLATHEADLL_API __declspec(dllexport)
#else
#define PLATHEADLL_API __declspec(dllimport)
#endif

#include "CommonHeader.h"
//#include "FaceDatabase.h"
//#include "NetworkCamera.h"
#include "video_output.h"
#include "ElaborationCore.h"
//#include "ApplicationWorkFlow.h"
#include "StereoCalibration.h"
#include "PlanViewMap.h"
//#include "SyncTest.h"
//#include "RoomSettings.h"
//#include "TestDesigner.h"
#include "PositionTestDialog.h"
//#include "TestAnalyzerDialog.h"
#include "RecordPlayerDialog.h"
#include "Shared.h"
//#include "SVMDialog.h"
//#include "AcquisitionStatistics.h"
//#include "TCPServer.h"

#include <LeoLog4CPP.h>
#include <LeoWindowsGUI.h>
#include <LeoWindowsConsole.h>
#include <LeoSettingsPersistence.h>


#include <Windows.h>
#include <iostream>
#include <sstream>


#define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s << "\n";                   \
   OutputDebugStringW( os_.str().c_str() );  \
}



extern "C" PLATHEADLL_API void loadConfigurationFile(const char str[]);
extern "C" PLATHEADLL_API void internalCalibration(const char str[], int selectedMask);
extern "C" PLATHEADLL_API void externalCalibration(const char str[]);
extern "C" PLATHEADLL_API void svmClassifier(const char str[]);
extern "C" PLATHEADLL_API void initializeSystemDemo(); // ONLY for testing purpose
extern "C" PLATHEADLL_API void initializeSystem(const char* username, const char* password, const char* type, const char* resolution,
	int fps, const char* cameraModel, const char* IPAddress1, int port1, const char* IPAddress2, int port2);
extern "C" PLATHEADLL_API void startLocalEngine(bool withoutTracking, bool saveTracksToFile);
extern "C" PLATHEADLL_API void openPositionTest();
extern "C" PLATHEADLL_API void startPositionTest();




extern "C" PLATHEADLL_API void system_hidevideoinput();
extern "C" PLATHEADLL_API void test_savetrackstofile();
extern "C" PLATHEADLL_API void system_alignrightframe(int hDisplacement, int vDisplacement);
extern "C" PLATHEADLL_API void test_plathearecorder();
extern "C" PLATHEADLL_API void test_startvideorecording();
extern "C" PLATHEADLL_API void test_stopvideorecordingandsave(bool save, const char* dir);
extern "C" PLATHEADLL_API void system_stopacquisition();
extern "C" PLATHEADLL_API void system_acquisitionstats();
extern "C" PLATHEADLL_API void system_editfacedatabase();
//extern "C" PLATHEADLL_API void system_esci();
extern "C" PLATHEADLL_API void calibration_internalcalibrationprocedure();
extern "C" PLATHEADLL_API void calibration_startexternalcalibration();
extern "C" PLATHEADLL_API void calibration_loadexternalcalibrationdata();
extern "C" PLATHEADLL_API void calibration_saveexternalcalibrationdata();
extern "C" PLATHEADLL_API void localizationengine_startlocalizationengine();
extern "C" PLATHEADLL_API void localizationengine_endinitializationphase();
extern "C" PLATHEADLL_API void localizationengine_stoplocalizationengine();
extern "C" PLATHEADLL_API void localizationengine_setengineparameters();
extern "C" PLATHEADLL_API void localizationengine_selectsvmclassifier();
extern "C" PLATHEADLL_API void localizationengine_opticalflow();
extern "C" PLATHEADLL_API void localizationengine_svm();
extern "C" PLATHEADLL_API void showwindow_currentbackground();
extern "C" PLATHEADLL_API void showwindow_foreground();
extern "C" PLATHEADLL_API void showwindow_edgeactivity();
extern "C" PLATHEADLL_API void showwindow_rawdisparitymap();
extern "C" PLATHEADLL_API void showwindow_filteredforeground();
extern "C" PLATHEADLL_API void showwindow_planviewoccupancymap();
extern "C" PLATHEADLL_API void showwindow_planviewheightsmap();
extern "C" PLATHEADLL_API void calibration_editroomsettings();
extern "C" PLATHEADLL_API void system_loadconfigurationfile();
extern "C" PLATHEADLL_API void system_saveconfigurationfileas();
extern "C" PLATHEADLL_API void system_saveconfigurationfile();
extern "C" PLATHEADLL_API void system_selecthaarcascadexml();
extern "C" PLATHEADLL_API void test_synchronizationtest();
extern "C" PLATHEADLL_API void test_showdebugconsole();
extern "C" PLATHEADLL_API void test_testdesigner();
extern "C" PLATHEADLL_API void test_positiontest();
extern "C" PLATHEADLL_API void test_statisticanalyzer();
extern "C" PLATHEADLL_API void test_svmlearning();



