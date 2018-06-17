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

#ifndef LEOSTORM_STL_EXTENSION_H
#define LEOSTORM_STL_EXTENSION_H

#include <map>
#include <vector>
#include <cmath>

#define LEO_MIN(a, b) (a > b ? b : a)
#define LEO_MAX(a, b) (a > b ? a : b)

#define LEO_MIN3(x,y,z)  ((y) <= (z) ? ((x) <= (y) ? (x) : (y)) : ((x) <= (z) ? (x) : (z)))

#define LEO_MAX3(x,y,z)  ((y) >= (z) ? ((x) >= (y) ? (x) : (y)) : ((x) >= (z) ? (x) : (z)))

#define LEO_XOR(a, b) (!(a && b) && (a || b))

#define LEO_CONSTRAINED_MAX_UPDATE(originalVariable, newValue, uninitValue) \
	(originalVariable = (originalVariable == uninitValue ? newValue : MAX(newValue, originalVariable)))
#define LEO_CONSTRAINED_MIN_UPDATE(originalVariable, newValue, uninitValue) \
	(originalVariable = (originalVariable == uninitValue ? newValue : MIN(newValue, originalVariable)))

template <class T> struct Leo2DPoint {
	T x;
	T y;
};

template <class T> struct Leo3DPoint {
	T x;
	T y;
	T z;
};

template <class T> struct LeoRect {
	Leo2DPoint<T> upperLeft;
	Leo2DPoint<T> bottomRight;
};

template <class T> struct LeoSize {
	T width;
	T height;
	LeoSize(T width, T height) {
		this->width = width;
		this->height = height;
	}
	LeoSize() {width = 0; height = 0;}
};

#define LEO_APPLYTHRESHOLD_MIN 1
#define LEO_APPLYTHRESHOLD_MAX 2
template <class T> T ApplyThreshold(T value, int mask, T minimo, T massimo) {
	if ((mask & LEO_APPLYTHRESHOLD_MAX) && value > massimo) {
		value = massimo;
	} else if ((mask & LEO_APPLYTHRESHOLD_MIN) && value < minimo) {
		value = minimo;
	}
	return value;
}


inline double CalculateVectorialDistance(double x1, double y1, double x2, double y2) {
	double xDifference = (x1 - x2);
	double yDifference = (y1 - y2);
	return sqrt((xDifference * xDifference) + (yDifference * yDifference));
}

template <typename T> void Dispositions(std::vector<T> &vect, int z, int groupSize, std::vector<std::vector<T>> &total) {
	int totalNumberOfElements = int(vect.size());
	T scambio;
	if (z < groupSize) {
		for (int k = z; k < totalNumberOfElements; k++) {
			scambio = vect[k];
			vect[k] = vect[z];
			vect[z] = scambio;

			Dispositions(vect, z + 1, groupSize, total);

			scambio = vect[k];
			vect[k] = vect[z];
			vect[z] = scambio;
		}
	} else {
		std::vector<T> vectToFill;
		for (int i = 0; i < groupSize; i++)
			vectToFill.push_back(vect[i]);
		total.push_back(vectToFill);
	}
}

template <typename T> void Dispositions(std::vector<T> &vect, int groupSize, std::vector<std::vector<T>> &total) {
	Dispositions(vect, 0, groupSize, total);
}

template <typename T> void Permutations(std::vector<T> &vect, std::vector<std::vector<T>> &total) {
	Dispositions(vect, 0, int(vect.size()), total);
}

inline double EvaluateNormalDistribution(Leo2DPoint<double> mean, float variance, Leo2DPoint<double> x) {
	double pi_inverse = 0.39894228040143267793994605993438;
	double preMult = double(sqrt(variance));
	preMult = pi_inverse/preMult;
	double distance = (mean.x - x.x)*(mean.x - x.x) + (mean.y - x.y)*(mean.y - x.y);
	double exponent = distance / (2*variance);
	return preMult*exp(-exponent);
}

void trim(std::string& str);
void trim(char *str);


class PrerequisitesChecker {
public:
	virtual bool PrerequisitesCheck(wchar_t *errMsg = 0, int bufferSize = 0, bool *warning = 0) {
		return true;
	}
};


template <class T> inline bool AreRectIntersected(LeoRect<T> a, LeoRect<T> b) {
	int verticalSum = (a.bottomRight.y - a.upperLeft.y) + (b.bottomRight.y - b.upperLeft.y),
		horizontalSum = (a.bottomRight.x - a.upperLeft.x) + (b.bottomRight.x - b.upperLeft.x);
	int verticalProjection = LEO_MAX(a.bottomRight.y, b.bottomRight.y) - LEO_MIN(a.upperLeft.y, b.upperLeft.y);
	int horizontalProjection = LEO_MAX(a.bottomRight.x, b.bottomRight.x) - LEO_MIN(a.upperLeft.x, b.upperLeft.x);
	if (horizontalProjection < horizontalSum && verticalProjection < verticalSum)
		return true;
	else 
		return false;
}

bool ToRelativePath(wchar_t *configurationPath, wchar_t *destFile, wchar_t *outputFile, int bufSize);

#endif //LEOSTORM_STL_EXTENSION_H