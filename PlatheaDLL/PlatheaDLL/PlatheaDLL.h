// PLATHEADLL.h - Contains declarations 
#pragma once

#ifdef PLATHEADLL_EXPORTS
#define PLATHEADLL_API __declspec(dllexport)
#else
#define PLATHEADLL_API __declspec(dllimport)
#endif


extern "C" PLATHEADLL_API void system_hidevideoinput();
extern "C" PLATHEADLL_API void test_savetrackstofile();
extern "C" PLATHEADLL_API void sistema_initializesystem();
extern "C" PLATHEADLL_API void system_alignrightframe();
extern "C" PLATHEADLL_API void test_plathearecorder();
extern "C" PLATHEADLL_API void test_startvideorecording();
extern "C" PLATHEADLL_API void test_stopvideorecordingandsave();
extern "C" PLATHEADLL_API void system_stopacquisition();
extern "C" PLATHEADLL_API void system_acquisitionstats();
extern "C" PLATHEADLL_API void system_editfacedatabase();
extern "C" PLATHEADLL_API void system_esci();
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



