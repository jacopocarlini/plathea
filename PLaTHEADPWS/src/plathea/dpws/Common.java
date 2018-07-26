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
import org.ws4d.java.types.QName;

public class Common {

	private static Common instance = null;
	
	public LinkedList<PLaTHEAMeasurement> lastMeasurement = null;
	public ComplexType listType = null;
	public Element listElement = null;
	
	public ComplexType measurementInfo = null;
	public Element measurementInfoElement = null;
	
	private Common() {
		Element objectType = new Element(new QName("objectType", "http://www.dis.uniroma1.it/plathea"), SchemaUtil.TYPE_STRING);
		Element objectID = new Element(new QName("objectID", "http://www.dis.uniroma1.it/plathea"), SchemaUtil.TYPE_INT);
		Element position = new Element(new QName("position", "http://www.dis.uniroma1.it/plathea"), SchemaUtil.TYPE_STRING);
		Element identityID = new Element(new QName("identityID", "http://www.dis.uniroma1.it/plathea"), SchemaUtil.TYPE_INT);
		Element identityType = new Element(new QName("identityType", "http://www.dis.uniroma1.it/plathea"), SchemaUtil.TYPE_STRING);
		Element identity = new Element(new QName("identity", "http://www.dis.uniroma1.it/plathea"), SchemaUtil.TYPE_STRING);
		/*this.addOutputParameter("name", SchemaUtil.TYPE_STRING);
		this.addOutputParameter("surname", SchemaUtil.TYPE_STRING);*/
		
		measurementInfo = new ComplexType(new QName("measurementInfoType", "http://www.dis.uniroma1.it/plathea"),
				ComplexType.CONTAINER_SEQUENCE);
		measurementInfo.addElement(objectType);
		measurementInfo.addElement(objectID);
		measurementInfo.addElement(position);
		measurementInfo.addElement(identityID);
		measurementInfo.addElement(identityType);
		measurementInfo.addElement(identity);
		
		measurementInfoElement = new Element(new QName("measurementInfoElem", "http://www.dis.uniroma1.it/plathea"), measurementInfo);
		measurementInfoElement.setMaxOccurs(10);
		measurementInfoElement.setMinOccurs(0);
		
		listType = new ComplexType(new QName("measurementInfoListType","http://www.dis.uniroma1.it/plathea"),
				ComplexType.CONTAINER_ALL);
		listType.addElement(measurementInfoElement);
		listElement = new Element(new QName("measurementInfoList","http://www.dis.uniroma1.it/plathea"),listType);
	}
	
	public static Common getInstance() {
		if (instance==null)
			instance = new Common();
		return instance;
	}

}
