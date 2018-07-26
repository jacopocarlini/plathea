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

import org.ws4d.java.authorization.AuthorizationManager;
import org.ws4d.java.communication.CommunicationException;
import org.ws4d.java.schema.Element;
import org.ws4d.java.schema.SchemaUtil;
import org.ws4d.java.security.CredentialInfo;
import org.ws4d.java.service.DefaultService;
import org.ws4d.java.service.InvocationException;
import org.ws4d.java.service.Operation;
import org.ws4d.java.service.parameter.ParameterValue;
import org.ws4d.java.types.QName;
import org.ws4d.java.types.URI;
import org.ws4d.java.service.parameter.ParameterValueManagement;

public class PLaTHEAAsynchronousIface extends DefaultService {
	
	public final static URI	MGT_SERVICE_ID	= new URI("http://www.dis.uniroma1.it/plathea" + "/PLaTHEAAsynchronousIface");
	
	private QName        serviceType             = new QName("PLaTHEAAsynchronousIface", "http://www.dis.uniroma1.it/plathea");

	public PLaTHEAAsynchronousIface() {
		super();
		this.setServiceId(MGT_SERVICE_ID);
		//this.addOperation(new MyFirstOperation());
		
		SimpleEvent notification = new SimpleEvent();
		addEventSource(notification);
	}

}