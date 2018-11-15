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

#ifndef APPLICATION_WORK_FLOW
#define APPLICATION_WORK_FLOW

#include "CommonHeader.h"

enum InternalEvent {APPLICATION_STARTED, ACQUISITION_STARTED_CALIB, ACQUISITION_STARTED_NO_CALIB,
	ACQUISITION_STOPPED, EXTERNAL_CALIBRATION_STARTED, CALIBRATION_ENDED, ELABORATION_STARTED, ELABORATION_ENDED};
enum InternalState {STANDBY, ACQUISITION_INTERNAL_CALIB, ACQUISITION_NO_INTERNAL_CALIB,
	INTERNAL_CALIBRATION, EXTERNAL_CALIBRATION, ELABORATION};

class ApplicationWorkFlow {
private:
	InternalState currentState;
	static ApplicationWorkFlow *instance;
	ApplicationWorkFlow() {
		currentState = STANDBY;
	}
public:
	static ApplicationWorkFlow *GetInstance() {
		if (!instance)
			instance = new ApplicationWorkFlow();
		return instance;
	}
	InternalState GetCurrentState() {return currentState;}
	void UpdateSystemState (InternalEvent ev);
};

#endif //APPLICATION_WORK_FLOW