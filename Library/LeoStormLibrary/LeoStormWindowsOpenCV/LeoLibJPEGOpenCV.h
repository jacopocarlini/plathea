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

#ifndef LEOSTORM_LIBJPEG_OPENCV_H
#define LEOSTORM_LIBJPEG_OPENCV_H

#include <csetjmp>
#include <cstdio>
#include <jpeglib.h>

#include <opencv2\core.hpp>

struct my_error_mgr {
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
};

class JPeg2IplImage {
public:
	JPeg2IplImage();
	~JPeg2IplImage();
	bool Decompress(unsigned char * buf, unsigned int len, IplImage * image);
private:
	jpeg_source_mgr jsrc;
	jpeg_decompress_struct cinfo;
	my_error_mgr jerr;
};

#endif //LEOSTORM_LIBJPEG_OPENCV_H