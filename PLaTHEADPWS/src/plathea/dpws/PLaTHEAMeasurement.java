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

import java.util.StringTokenizer;

public class PLaTHEAMeasurement {
	public String objectType; //'t' = tracked, 'l' = lost
	public int objectID;
	public String position; //'(x,y)'
	public int identityID;
	public String identityType; //'j' = just identified, 'i' = identified, '-' = not identified
	public String identity;
	public PLaTHEAMeasurement(String newLine) {
		StringTokenizer st = new StringTokenizer(newLine, "\t");
		objectType = st.nextToken();
		objectID = Integer.parseInt(st.nextToken());
		position = st.nextToken();
		identityID = Integer.parseInt(st.nextToken());
		identityType = st.nextToken();
		identity = st.nextToken();
	}
}
