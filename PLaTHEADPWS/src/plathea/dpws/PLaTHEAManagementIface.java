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

public class PLaTHEAManagementIface extends DefaultService {

	public final static URI	MGT_SERVICE_ID	= new URI("http://www.dis.uniroma1.it/plathea" + "/PLaTHEAManagementIface");
	
	private QName        serviceType             = new QName("PLaTHEAManagementIface", "http://www.dis.uniroma1.it/plathea");

	public PLaTHEAManagementIface() {
		super();
		this.setServiceId(MGT_SERVICE_ID);
		this.addOperation(new StartInstance());
		this.addOperation(new StopInstance());
		this.addOperation(new GetParameterList());
		this.addOperation(new GetParameter());
		this.addOperation(new SetParameter());
	}

}

class StartInstance extends Operation {
	public StartInstance() {
		super("StartInstance", new QName("PLaTHEAManagementIface", "http://www.dis.uniroma1.it/plathea"));
		Element valueElem = new Element(new QName("startInstanceRet", "http://www.dis.uniroma1.it/plathea"), SchemaUtil.TYPE_BOOLEAN);
		this.setOutput(valueElem);
	}

	@Override
	protected ParameterValue invokeImpl(ParameterValue arg0, CredentialInfo arg1)
			throws InvocationException, CommunicationException {
		ParameterValue paramValue = createOutputValue();
		return paramValue;
	}
	
}

class StopInstance extends Operation {
	public StopInstance() {
		super("StopInstance", new QName("PLaTHEAManagementIface", "http://www.dis.uniroma1.it/plathea"));
		Element valueElem = new Element(new QName("stopInstanceRet", "http://www.dis.uniroma1.it/plathea"), SchemaUtil.TYPE_BOOLEAN);
		this.setOutput(valueElem);
	}

	@Override
	protected ParameterValue invokeImpl(ParameterValue arg0, CredentialInfo arg1)
			throws InvocationException, CommunicationException {
		ParameterValue paramValue = createOutputValue();
		return paramValue;
	}
	
}

class GetParameterList extends Operation {
	public GetParameterList() {
		super("GetParameterList", new QName("PLaTHEAManagementIface", "http://www.dis.uniroma1.it/plathea"));
		Element valueElem = new Element(new QName("getParameterListRet", "http://www.dis.uniroma1.it/plathea"), SchemaUtil.TYPE_STRING);
		this.setOutput(valueElem);
	}

	@Override
	protected ParameterValue invokeImpl(ParameterValue arg0, CredentialInfo arg1)
			throws InvocationException, CommunicationException {
		ParameterValue paramValue = createOutputValue();
		return paramValue;
	}
	
}

class GetParameter extends Operation {
	public GetParameter() {
		super("GetParameter", new QName("PLaTHEAManagementIface", "http://www.dis.uniroma1.it/plathea"));
		Element valueElem = new Element(new QName("getParameterRet", "http://www.dis.uniroma1.it/plathea"), SchemaUtil.TYPE_STRING);
		this.setOutput(valueElem);
	}

	@Override
	protected ParameterValue invokeImpl(ParameterValue arg0, CredentialInfo arg1)
			throws InvocationException, CommunicationException {
		ParameterValue paramValue = createOutputValue();
		return paramValue;
	}
	
}

class SetParameter extends Operation {
	public SetParameter() {
		super("SetParameter", new QName("PLaTHEAManagementIface", "http://www.dis.uniroma1.it/plathea"));
		Element valueElem = new Element(new QName("setParameterRet", "http://www.dis.uniroma1.it/plathea"), SchemaUtil.TYPE_STRING);
		this.setOutput(valueElem);
	}

	@Override
	protected ParameterValue invokeImpl(ParameterValue arg0, CredentialInfo arg1)
			throws InvocationException, CommunicationException {
		ParameterValue paramValue = createOutputValue();
		return paramValue;
	}
	
}