// PLATHEADLL.h - Contains declarations 
#pragma once

#ifdef PLATHEADLL_EXPORTS
#define PLATHEADLL_API __declspec(dllexport)
#else
#define PLATHEADLL_API __declspec(dllimport)
#endif

#include "CommonHeader.h"
#include "FaceDatabase.h"
#include "NetworkCamera.h"
#include "video_output.h"
#include "ElaborationCore.h"
#include "ApplicationWorkFlow.h"
#include "StereoCalibration.h"
#include "PlanViewMap.h"
#include "SyncTest.h"
#include "RoomSettings.h"
#include "TestDesigner.h"
#include "PositionTestDialog.h"
#include "TestAnalyzerDialog.h"
#include "RecordPlayerDialog.h"
#include "Shared.h"
#include "SVMDialog.h"
#include "AcquisitionStatistics.h"
//#include "TCPServer.h"

#include <LeoLog4CPP.h>
#include <LeoWindowsGUI.h>
#include <LeoWindowsConsole.h>
#include <LeoSettingsPersistence.h>



//  da cancellare, è solo una prova
class Point {
public:
	int x;
	void printPoint();
};


extern "C" PLATHEADLL_API Point* createPoint();




//Plathea API

extern "C" PLATHEADLL_API void internalCalibrationDemo(const char str[]); // ONLY for testing purpose
extern "C" PLATHEADLL_API void initializeSystemDemo(); // ONLY for testing purpose

extern "C" PLATHEADLL_API void openPositionTest();
extern "C" PLATHEADLL_API void startPositionTest();



extern "C" PLATHEADLL_API void system_initializesystem(const char* username, const char* password, const char* type, const char* resolution,
	int fps, const char* cameraModel, const char* IPAddress1, int port1, const char* IPAddress2, int port2); //DEMO
extern "C" PLATHEADLL_API void system_stopacquisition();
extern "C" PLATHEADLL_API void system_alignrightframe(int hDisplacement, int vDisplacement); 
extern "C" PLATHEADLL_API float* system_acquisitionstats(); // return float array: { _leftRate , _rightRate, _stereoRate};
extern "C" PLATHEADLL_API void system_acquisitionstats_start(); 
extern "C" PLATHEADLL_API void system_acquisitionstats_stop(); 
extern "C" PLATHEADLL_API void system_hidevideoinput();
extern "C" PLATHEADLL_API void system_loadconfigurationfile(const char str[]);  //DEMO
extern "C" PLATHEADLL_API void system_saveconfigurationfileas(const char str[]);
//extern "C" PLATHEADLL_API void system_saveconfigurationfile();   
extern "C" PLATHEADLL_API void system_selecthaarcascadexml(const char dir[]);
extern "C" PLATHEADLL_API void system_editfacedatabase();  //TODO
//extern "C" PLATHEADLL_API void system_esci();

extern "C" PLATHEADLL_API void test_synchronizationtest(); // TODO
extern "C" PLATHEADLL_API void test_testdesigner(); // TODO 
extern "C" PLATHEADLL_API void test_positiontest(bool exitInitPhase);  // DEMO menu *Da chiedere al prof
extern "C" PLATHEADLL_API void test_positiontest_start();  // TODO DEMO menu
extern "C" PLATHEADLL_API void test_positiontest_stop();  // TODO DEMO menu
extern "C" PLATHEADLL_API void test_statisticanalyzer(); // TODO
//extern "C" PLATHEADLL_API void test_showdebugconsole();
extern "C" PLATHEADLL_API void test_plathearecorder(); //demo 
extern "C" PLATHEADLL_API void test_plathearecorder_start(const char* dir); //demo
extern "C" PLATHEADLL_API void test_plathearecorder_stop(); //demo
extern "C" PLATHEADLL_API void test_svmlearning(const char dir[]);
extern "C" PLATHEADLL_API void test_savetrackstofile();

//extern "C" PLATHEADLL_API void test_startvideorecording();
//extern "C" PLATHEADLL_API void test_stopvideorecordingandsave(bool save, const char* dir);

//extern "C" PLATHEADLL_API void calibration_internalcalibrationprocedure(); // menu
extern "C" PLATHEADLL_API void calibration_internalcalibration_save(const char dir[], int selectedMask); 
extern "C" PLATHEADLL_API void calibration_internalcalibration_load(const char dir[], int selectedMask); // DEMO 
extern "C" PLATHEADLL_API void calibration_internalcalibration_rectificationbyparam(); 
extern "C" PLATHEADLL_API void calibration_internalcalibration_rectification_bouguet(bool singleCalibration); 
extern "C" PLATHEADLL_API void calibration_internalcalibration_rectification_hartley(bool singleCalibration); 
extern "C" PLATHEADLL_API void calibration_internalcalibration_startleft(); 
extern "C" PLATHEADLL_API void calibration_internalcalibration_startright(); 
extern "C" PLATHEADLL_API void calibration_internalcalibration_startstereo(); // TODO 
extern "C" PLATHEADLL_API void calibration_internalcalibration_stop(); // sub-function of calibration_internalcalibrationprocedure menu
extern "C" PLATHEADLL_API void calibration_startexternalcalibration(); //TODO
extern "C" PLATHEADLL_API void calibration_loadexternalcalibrationdata(const char dir[]); // DEMO
extern "C" PLATHEADLL_API void calibration_saveexternalcalibrationdata(const char dir[]);
extern "C" PLATHEADLL_API void calibration_editroomsettings(float WXmin, float WXmax, float WYmin, float WYmax, float WZmin, float WZmax,
	float texelSide, float personMaximumHeight, float personAverageHeight, float personAverageWidth, float personMimimumHeight);

extern "C" PLATHEADLL_API void localizationengine_startlocalizationengine(bool withoutTracking, bool saveTracksToFile, const char dir[]);
extern "C" PLATHEADLL_API void localizationengine_endinitializationphase();
extern "C" PLATHEADLL_API void localizationengine_stoplocalizationengine();
extern "C" PLATHEADLL_API void localizationengine_setengineparameters(); //TODO
extern "C" PLATHEADLL_API void localizationengine_selectsvmclassifier(const char dir[]); // DEMO
extern "C" PLATHEADLL_API void localizationengine_opticalflow();
extern "C" PLATHEADLL_API void localizationengine_svm();

//extern "C" PLATHEADLL_API void showwindow_currentbackground();
//extern "C" PLATHEADLL_API void showwindow_foreground();
//extern "C" PLATHEADLL_API void showwindow_edgeactivity();
//extern "C" PLATHEADLL_API void showwindow_rawdisparitymap();
//extern "C" PLATHEADLL_API void showwindow_filteredforeground();
//extern "C" PLATHEADLL_API void showwindow_planviewoccupancymap();
//extern "C" PLATHEADLL_API void showwindow_planviewheightsmap();


extern "C" PLATHEADLL_API std::vector<TrackedObject*> getTrackedPeople();
extern "C" PLATHEADLL_API ElaborationCore* getElaborationCore();

void updateTrackedPeople(std::vector<TrackedObject*> trackedPersons);


