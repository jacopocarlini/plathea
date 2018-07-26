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

package plathea.dpws;

import org.ws4d.java.JMEDSFramework;
import org.ws4d.java.eventing.EventSource;
import org.ws4d.java.types.QName;
import org.ws4d.java.util.Log;

public class PLaTHEADeviceProvider {

	public static void main(String[] args) throws java.io.IOException {
		JMEDSFramework.start(args);
		
		Log.setLogLevel(Log.DEBUG_LEVEL_DEBUG);
		
		PLaTHEADevice dev = new PLaTHEADevice();
		PLaTHEAAsynchronousIface ms = new PLaTHEAAsynchronousIface();
		PLaTHEASynchronousIface syncIface = new PLaTHEASynchronousIface();
		PLaTHEAManagementIface mgmtIface = new PLaTHEAManagementIface();
		
		EventSource event = ms.getEventSource(new QName("PLaTHEAAsynchronousIface", "http://www.dis.uniroma1.it/plathea"), "notifyPositions", null, "notifyPositions");
		SimpleEventThread eventProvider = new SimpleEventThread((SimpleEvent) event);
		eventProvider.start();
		
		dev.addService(ms);
		dev.addService(syncIface);
		dev.addService(mgmtIface);
		
		dev.start();

	}

}
