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

import org.ws4d.java.communication.CommunicationException;
import org.ws4d.java.schema.Element;
import org.ws4d.java.schema.SchemaUtil;
import org.ws4d.java.security.CredentialInfo;
import org.ws4d.java.service.DefaultService;
import org.ws4d.java.service.InvocationException;
import org.ws4d.java.service.Operation;
import org.ws4d.java.service.parameter.ParameterValue;
import org.ws4d.java.service.parameter.ParameterValueManagement;
import org.ws4d.java.types.QName;
import org.ws4d.java.types.URI;

public class PLaTHEASynchronousIface extends DefaultService {

public final static URI	MGT_SERVICE_ID	= new URI("http://www.dis.uniroma1.it/plathea" + "/PLaTHEASynchronousIface");
	
	private QName        serviceType             = new QName("PLaTHEASynchronousIface", "http://www.dis.uniroma1.it/plathea");

	public PLaTHEASynchronousIface() {
		super();
		this.setServiceId(MGT_SERVICE_ID);
		this.addOperation(new GetAllPositionsOperation());
		this.addOperation(new GetPositionFromObjectIDOperation());
		this.addOperation(new GetPositionFromPersonIDOperation());
		this.addOperation(new GetRoomInfoOperation());
	}

}

class GetAllPositionsOperation extends Operation {
	public GetAllPositionsOperation() {
		super("GetAllPositions", new QName("PLaTHEASynchronousIface", "http://www.dis.uniroma1.it/plathea"));
		Element valueElem = new Element(new QName("getAllPositionsRet", "http://www.dis.uniroma1.it/plathea"), Common.getInstance().listType);
		this.setOutput(valueElem);
	}

	@Override
	protected ParameterValue invokeImpl(ParameterValue arg0, CredentialInfo arg1)
			throws InvocationException, CommunicationException {
		ParameterValue paramValue = createOutputValue();
		if (Common.getInstance().lastMeasurement != null) {
			synchronized(Common.getInstance().lastMeasurement) {
				for (int i = 0; i < Common.getInstance().lastMeasurement.size(); i++) {
					PLaTHEAMeasurement a = Common.getInstance().lastMeasurement.get(i);
					ParameterValue currentMeasurement = paramValue.get("measurementInfoElem["+i+"]");
					ParameterValueManagement.setString(currentMeasurement, "objectType", a.objectType);
					ParameterValueManagement.setString(currentMeasurement, "objectID", a.objectID + "");
					ParameterValueManagement.setString(currentMeasurement, "position", a.position);
					ParameterValueManagement.setString(currentMeasurement, "identityID", a.identityID + "");
					ParameterValueManagement.setString(currentMeasurement, "identityType", a.identityType);
					ParameterValueManagement.setString(currentMeasurement, "identity", a.identity);
				}
			}
		}
		return paramValue;
	}
	
}

class GetPositionFromObjectIDOperation extends Operation {
	public GetPositionFromObjectIDOperation() {
		super("GetPositionFromObjectID", new QName("PLaTHEASynchronousIface", "http://www.dis.uniroma1.it/plathea"));
		Element valueElem = new Element(new QName("getPositionFromObjectIDRet", "http://www.dis.uniroma1.it/plathea"), Common.getInstance().measurementInfo);
		Element valueInput = new Element(new QName("getPositionFromObjectIDIn", "http://www.dis.uniroma1.it/plathea"), SchemaUtil.TYPE_INT);
		this.setOutput(valueElem);
		this.setInput(valueInput);
	}

	@Override
	protected ParameterValue invokeImpl(ParameterValue arg0, CredentialInfo arg1)
			throws InvocationException, CommunicationException {
		int toFind = Integer.parseInt(ParameterValueManagement.getString(arg0, "getPositionFromObjectIDIn"));
		ParameterValue paramValue = createOutputValue();
		if (Common.getInstance().lastMeasurement != null) {
			synchronized(Common.getInstance().lastMeasurement) {
				for (int i = 0; i < Common.getInstance().lastMeasurement.size(); i++) {
					PLaTHEAMeasurement a = Common.getInstance().lastMeasurement.get(i);
					if (a.objectID == toFind) {
						ParameterValueManagement.setString(paramValue, "objectType", a.objectType);
						ParameterValueManagement.setString(paramValue, "objectID", a.objectID + "");
						ParameterValueManagement.setString(paramValue, "position", a.position);
						ParameterValueManagement.setString(paramValue, "identityID", a.identityID + "");
						ParameterValueManagement.setString(paramValue, "identityType", a.identityType);
						ParameterValueManagement.setString(paramValue, "identity", a.identity);
					}
				}
			}
		}
		return paramValue;
	}
	
}

class GetPositionFromPersonIDOperation extends Operation {
	public GetPositionFromPersonIDOperation() {
		super("GetPositionFromPersonID", new QName("PLaTHEASynchronousIface", "http://www.dis.uniroma1.it/plathea"));
		Element valueElem = new Element(new QName("getPositionFromPersonIDRet", "http://www.dis.uniroma1.it/plathea"), Common.getInstance().measurementInfo);
		Element valueInput = new Element(new QName("getPositionFromPersonIDIn", "http://www.dis.uniroma1.it/plathea"), SchemaUtil.TYPE_INT);
		this.setOutput(valueElem);
		this.setInput(valueInput);
	}

	@Override
	protected ParameterValue invokeImpl(ParameterValue arg0, CredentialInfo arg1)
			throws InvocationException, CommunicationException {
		int toFind = Integer.parseInt(ParameterValueManagement.getString(arg0, "getPositionFromPersonIDIn"));
		ParameterValue paramValue = createOutputValue();
		if (Common.getInstance().lastMeasurement != null) {
			synchronized(Common.getInstance().lastMeasurement) {
				for (int i = 0; i < Common.getInstance().lastMeasurement.size(); i++) {
					PLaTHEAMeasurement a = Common.getInstance().lastMeasurement.get(i);
					if (a.identityID == toFind && toFind >= 0) {
						ParameterValueManagement.setString(paramValue, "objectType", a.objectType);
						ParameterValueManagement.setString(paramValue, "objectID", a.objectID + "");
						ParameterValueManagement.setString(paramValue, "position", a.position);
						ParameterValueManagement.setString(paramValue, "identityID", a.identityID + "");
						ParameterValueManagement.setString(paramValue, "identityType", a.identityType);
						ParameterValueManagement.setString(paramValue, "identity", a.identity);
					}
				}
			}
		}
		return paramValue;
	}
	
}

class GetRoomInfoOperation extends Operation {
	public GetRoomInfoOperation() {
		super("GetRoomInfo", new QName("PLaTHEASynchronousIface", "http://www.dis.uniroma1.it/plathea"));
		Element valueElem = new Element(new QName("getRoomInfoRet", "http://www.dis.uniroma1.it/plathea"), SchemaUtil.TYPE_INT);
		this.setOutput(valueElem);
	}

	@Override
	protected ParameterValue invokeImpl(ParameterValue arg0, CredentialInfo arg1)
			throws InvocationException, CommunicationException {
		ParameterValue paramValue = createOutputValue();
		return paramValue;
	}
	
}