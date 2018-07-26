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

import java.util.LinkedList;

import org.ws4d.java.schema.ComplexType;
import org.ws4d.java.schema.Element;
import org.ws4d.java.schema.SchemaUtil;
import org.ws4d.java.security.CredentialInfo;
import org.ws4d.java.service.DefaultEventSource;
import org.ws4d.java.service.parameter.ParameterValue;
import org.ws4d.java.service.parameter.ParameterValueManagement;
import org.ws4d.java.types.QName;

public class SimpleEvent extends DefaultEventSource {

	public SimpleEvent() {
		super("notifyPositions", new QName("PLaTHEAAsynchronousIface", "http://www.dis.uniroma1.it/plathea"));

		this.setOutput(Common.getInstance().listElement);
	}
	
	public void fireHelloWorldEvent(LinkedList<PLaTHEAMeasurement> toTrigger, int eventCounter) {
		ParameterValue paramValue = createOutputValue();
		for (int i = 0; i < toTrigger.size(); i++) {
			PLaTHEAMeasurement a = toTrigger.get(i);
			ParameterValue currentMeasurement = paramValue.get("measurementInfoElem["+i+"]");
			ParameterValueManagement.setString(currentMeasurement, "objectType", a.objectType);
			ParameterValueManagement.setString(currentMeasurement, "objectID", a.objectID + "");
			ParameterValueManagement.setString(currentMeasurement, "position", a.position);
			ParameterValueManagement.setString(currentMeasurement, "identityID", a.identityID + "");
			ParameterValueManagement.setString(currentMeasurement, "identityType", a.identityType);
			ParameterValueManagement.setString(currentMeasurement, "identity", a.identity);
		}
		fire(paramValue, eventCounter, CredentialInfo.EMPTY_CREDENTIAL_INFO);
	}
}
