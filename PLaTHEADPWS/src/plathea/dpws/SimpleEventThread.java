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

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.Socket;
import java.util.LinkedList;
import java.util.StringTokenizer;

public class SimpleEventThread extends Thread {

	private int eventCounter = 0;
	private SimpleEvent event;
	
	private BufferedReader br = null;
	
	public SimpleEventThread(SimpleEvent event) {
		this.event = event;
		
		try {
			Socket clientSocket = new Socket("localhost", 9999);
			br = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public void run() {
		while (true) {
			try {
				LinkedList<PLaTHEAMeasurement> toTrigger = new LinkedList<PLaTHEAMeasurement>();
				String newLine = null;
				while (!(newLine=br.readLine()).equals("-")) {
					PLaTHEAMeasurement a = new PLaTHEAMeasurement(newLine);
					toTrigger.add(a);
				}
				if (Common.getInstance().lastMeasurement==null)
					Common.getInstance().lastMeasurement = toTrigger;
				else {
					synchronized(Common.getInstance().lastMeasurement) {
						Common.getInstance().lastMeasurement = toTrigger;
					}
				}
				event.fireHelloWorldEvent(toTrigger, eventCounter++);
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
}
