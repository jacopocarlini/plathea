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

#ifndef LEOSTORM_COLOR_SPACES_H
#define LEOSTORM_COLOR_SPACES_H

#include "LeoStormSTLExtension.h"

#ifndef uchar
#define uchar unsigned char
#endif //uchar

namespace leostorm {
	namespace colorspaces {

		enum DataTypeConversion {UCHARSRC_UCHARDST, UCHARSRC_FLOATDST, FLOATSRC_UCHARDST, FLOATSRC_FLOATDST};

		void BGR2HSL(void *src, void *dst, DataTypeConversion types);
		void HSL2BGR(void *src, void *dst, DataTypeConversion types);
		void BGR2HSV(const void *src, void *dst, DataTypeConversion types);
		void HSV2BGR(void *src, void *dst, DataTypeConversion types);

		void BGR2LUV(void *src, void *dst, DataTypeConversion types);
		void LUV2BGR(void *src, void *dst, DataTypeConversion types);
		float LUVChromaticityDistance(float *p1, float *p2);

		void BGR2GRAYSCALE(uchar *src, uchar *dst);
		void RGB2BGR(const uchar* rgb, int rgb_step, uchar* bgr, int bgr_step, LeoSize<int> size);
	}
}

#endif //LEOSTORM_COLOR_SPACES_H