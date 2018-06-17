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

#include "LeoLibJPEGOpenCV.h"
#include <LeoColorSpaces.h>
#include <opencv2/imgproc/imgproc.hpp>

using namespace leostorm::colorspaces;

//******* Functions needed by LibJPeg InputManager ********//
JOCTET eoi_buffer[2] = { 0xFF, JPEG_EOI };

void mem_init_source (j_decompress_ptr cinfo) {
	jpeg_source_mgr * jsrc = cinfo->src;

	/* Nothing to do */
}

boolean mem_fill_input_buffer (j_decompress_ptr cinfo) {
	//JOCTET è un unsigned char
	struct jpeg_source_mgr *jsrc = cinfo->src;

	/* Create a fake EOI marker */
	jsrc->next_input_byte = eoi_buffer;
	jsrc->bytes_in_buffer = 2;

	return TRUE;
}

void mem_skip_input_data (j_decompress_ptr cinfo, long num_bytes) {
	jpeg_source_mgr *jsrc = cinfo->src;

	if (num_bytes > 0) {
		while (num_bytes > (long)jsrc->bytes_in_buffer) {
			num_bytes -= (long)jsrc->bytes_in_buffer;
			mem_fill_input_buffer (cinfo);
		}

		jsrc->next_input_byte += num_bytes;
		jsrc->bytes_in_buffer -= num_bytes;
    }
}

void mem_term_source (j_decompress_ptr cinfo) {
	jpeg_source_mgr * jsrc = cinfo->src;
	/* Nothing to do */
}

void my_jpeg_errorfunction (j_common_ptr cinfo) {
	my_error_mgr * my_err = (my_error_mgr *) cinfo->err;
	char error_message[1024];
	my_err->pub.format_message(cinfo, error_message);
	//writeToConsole(L"%S\r\n", error_message);
	longjmp(my_err->setjmp_buffer, 1);
}

//********* JPeg2IplImage implementation ***********//

JPeg2IplImage::JPeg2IplImage() {
	//Initialization of LibJPeg source manager
	jsrc.init_source = mem_init_source;
	jsrc.fill_input_buffer = mem_fill_input_buffer;
	jsrc.skip_input_data = mem_skip_input_data;
	jsrc.resync_to_restart = jpeg_resync_to_restart;
	jsrc.term_source = mem_term_source;

	//Initialization of LibJPeg decompress struct and error manager
	jpeg_create_decompress(&cinfo);
	cinfo.err=jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_jpeg_errorfunction;
	cinfo.src=&jsrc;
}

JPeg2IplImage::~JPeg2IplImage() {
	jpeg_destroy_decompress (&cinfo);
}

bool JPeg2IplImage::Decompress(unsigned char *buf, unsigned int len, IplImage * image) {
	jsrc.next_input_byte = buf;
	jsrc.bytes_in_buffer = len;

	if (!setjmp(jerr.setjmp_buffer)) {
		jpeg_read_header (&cinfo, TRUE);
		jpeg_start_decompress (&cinfo);
		int row_stride = cinfo.output_width * cinfo.output_components;
		JSAMPARRAY j=(*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

		unsigned char * data = (unsigned char *) image->imageData;

		for (unsigned int i = 0; i < cinfo.image_height; ++i, data += image->widthStep) {
			jpeg_read_scanlines (&cinfo, j, 1);
			if (int(i) < image->height)
				RGB2BGR( j[0], 0, data, 0, LeoSize<int>((int) cinfo.output_width,1)); //DIB è in formato BGR
		}

		jpeg_finish_decompress (&cinfo);
		return true;
	} else {
		return false;
	}
}