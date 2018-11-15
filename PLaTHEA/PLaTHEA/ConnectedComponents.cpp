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

#include "ConnectedComponents.h"
#include "Shared.h"

using namespace leostorm::settingspersistence;

class SpecializedParametersSetter: public ParameterSetter {
public:
	float perimScale;
	bool useContourScanner;
	SpecializedParametersSetter();
	void ModifyParameter(char *var, int delta); //For perimScale
};

void SpecializedParametersSetter::ModifyParameter(char *var, int delta) {
	if (strcmp(var, "perimScale") == 0) {
		perimScale+=(delta*0.1f);
		this->perimScale = ApplyThreshold(perimScale, LEO_APPLYTHRESHOLD_MIN, 0.f, 0.f);
	}
}

SpecializedParametersSetter::SpecializedParametersSetter(): ParameterSetter("findConnectedComponents") {
	AddParameter("perimScale", &perimScale);
	perimScale = 2.f;
	AddParameter("useContourScanner", &useContourScanner);
	useContourScanner = false;
}

static SpecializedParametersSetter parameters;

FindConnectedComponents::FindConnectedComponents(int height, int width): perimScale(parameters.perimScale), useContourScanner(parameters.useContourScanner) {
	this->connectedComponents = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	this->mem_storage = cvCreateMemStorage();
}

FindConnectedComponents::~FindConnectedComponents() {
	cvReleaseImage(&connectedComponents);
	cvReleaseMemStorage(&mem_storage);
}

CvSeq *FindConnectedComponents::UpdateConnectedComponents(const IplImage *rawForeground) {
	if (!useContourScanner) {
		//cvCopy(rawForeground, connectedComponents);
		cvSmooth(rawForeground, connectedComponents, CV_MEDIAN);
		return NULL;
	} else {
		//cvCopy(rawForeground, connectedComponents);
		cvSmooth(rawForeground, connectedComponents, CV_MEDIAN);
		cvClearMemStorage(mem_storage);

		CvContourScanner scanner = cvStartFindContours(connectedComponents, mem_storage, sizeof(CvContour),
			CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

		int height = connectedComponents->height, width = connectedComponents->width;
		double q = (height + width)/perimScale;
		CvSeq *c;
		
		while ((c = cvFindNextContour(scanner)) != NULL) {
			double len = cvContourPerimeter(c);
			if (len < q) {
				cvSubstituteContour(scanner, NULL);
			}
		}
		CvSeq *contours = cvEndFindContours(&scanner);

		const CvScalar CVX_WHITE = cvScalar(0xff);
		cvZero(connectedComponents);

		for (c = contours; c!=NULL; c = c->h_next) {
			cvDrawContours(connectedComponents, c, CVX_WHITE, CVX_WHITE, -1, CV_FILLED, 8);
		}

		return contours;
	}
}