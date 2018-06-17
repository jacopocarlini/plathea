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

import org.ws4d.java.communication.connection.ip.IPAddress;
import org.ws4d.java.service.DefaultDevice;
import org.ws4d.java.types.QName;
import org.ws4d.java.types.QNameSet;

public class PLaTHEADevice extends DefaultDevice {

	public PLaTHEADevice()  {
		super();
		
		this.setPortTypes(new QNameSet(new QName("PLaTHEADevice", "http://www.dis.uniroma1.it/plathea")));
		this.addFriendlyName("en-US", "PLaTHEADevice");
		this.setManufacturerUrl("http://www.dis.uniroma1.it");
		this.addManufacturer("en-US", "DIS");
		
		//this.addBinding(new HTTPBinding(new IPAddress("127.0.0.1"), 4567, "PLaTHEADevice"));
	}

}
