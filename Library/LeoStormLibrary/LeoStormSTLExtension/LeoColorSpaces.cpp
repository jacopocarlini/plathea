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

#include "LeoColorSpaces.h"

using namespace leostorm::colorspaces;

void leostorm::colorspaces::BGR2HSL(void *src, void *dst, DataTypeConversion types) {
	float b, g, r;

	if (types == UCHARSRC_FLOATDST || types == UCHARSRC_UCHARDST) {
		uchar *castSrc = (uchar *) src;
		b = castSrc[0]/255.f, g = castSrc[1]/255.f, r = castSrc[2]/255.f;
	} else {
		float *castSrc = (float *) src;
		b = castSrc[0]/255.f, g = castSrc[1]/255.f, r = castSrc[2]/255.f;
	}

	float min_rgb = LEO_MIN3(b, g, r), max_rgb = LEO_MAX3(b, g, r);
	float hue = 0.f, saturation= 0.f, luma, diff;

	diff = max_rgb - min_rgb;
	luma = (max_rgb + min_rgb)*0.5f;

	if (diff>FLT_EPSILON) {
		saturation = luma < 0.5f ? diff/(max_rgb + min_rgb) : diff/(2 - max_rgb - min_rgb);
		diff = 60.f/diff;

		if (max_rgb == r) {
			hue = (g - b)*diff;
		} else if (max_rgb == g) {
			hue = 120.f + (b - r)*diff;
		} else {
			hue = 240.f + (r - g)*diff;
		}

		if (hue < 0.f)
			hue+=360.f;
	}

	saturation*=255.f;
	luma*=255.f;

	if (types == UCHARSRC_UCHARDST || types == FLOATSRC_UCHARDST) {
		uchar *castDst = (uchar *) dst;
		castDst[0] = (uchar) (hue/2.f);
		castDst[1] = (uchar) saturation;
		castDst[2] = (uchar) luma;
	} else {
		float *castDst = (float *) dst;
		castDst[0] = hue;
		castDst[1] = saturation;
		castDst[2] = luma;
	}
}

void leostorm::colorspaces::HSL2BGR(void *src, void *dst, DataTypeConversion types) {
	float h, s, l;

	if (types == UCHARSRC_FLOATDST || types == UCHARSRC_UCHARDST) {
		uchar *castSrc = (uchar *) src;
		h = castSrc[0]*2.f, s = castSrc[1]/255.f, l = castSrc[2]/255.f;
	} else {
		float *castSrc = (float *) src;
		h = castSrc[0], s = castSrc[1]/255.f, l = castSrc[2]/255.f;
	}
	
	float blue, green, red;

	if( s == 0 )
		blue = green = red = l;
	else {
		static const int sector_data[][3]=
			{{1,3,0}, {1,0,2}, {3,0,1}, {0,2,1}, {0,1,3}, {2,1,0}};
		float tab[4];
		int sector;
	    
		float p2 = l <= 0.5f ? l*(1 + s) : l + s - l*s;
		float p1 = 2*l - p2;

		h *= 0.016666666666666666f; // h /= 60;
		if( h < 0 )
			do h += 6; while( h < 0 );
		else if( h >= 6 )
			do h -= 6; while( h >= 6 );

		sector = (int) floor(h);
		h -= sector;

		tab[0] = p2;
		tab[1] = p1;
		tab[2] = p1 + (p2 - p1)*(1-h);
		tab[3] = p1 + (p2 - p1)*h;

		blue = tab[sector_data[sector][0]];
		green = tab[sector_data[sector][1]];
		red = tab[sector_data[sector][2]];
	}

	blue*=255.f;
	green*=255.f;
	red*=255.f;

	if (types == UCHARSRC_UCHARDST || types == FLOATSRC_UCHARDST) {
		uchar *castDst = (uchar *) dst;
		castDst[0] = (uchar) blue;
		castDst[1] = (uchar) green;
		castDst[2] = (uchar) red;
	} else {
		float *castDst = (float *) dst;
		castDst[0] = blue;
		castDst[1] = green;
		castDst[2] = red;
	}
}

void leostorm::colorspaces::BGR2HSV(const void *src, void *dst, DataTypeConversion types) {
	float b, g, r;

	if (types == UCHARSRC_FLOATDST || types == UCHARSRC_UCHARDST) {
		uchar *castSrc = (uchar *) src;
		b = castSrc[0]/255.f, g = castSrc[1]/255.f, r = castSrc[2]/255.f;
	} else {
		float *castSrc = (float *) src;
		b = castSrc[0]/255.f, g = castSrc[1]/255.f, r = castSrc[2]/255.f;
	}

	float min_rgb = LEO_MIN3(b, g, r), max_rgb = LEO_MAX3(b, g, r);
	float hue, saturation, value;
	value = max_rgb;
	if (max_rgb==0) {
		saturation = hue = 0.f;
	} else {
		saturation = (value - min_rgb)/value;
		if (saturation == 0) {
			hue = 0;
		} else {
			if (r == max_rgb) {
				hue = 0.f + (60.f*(g - b))/(value-min_rgb);
			} else if (g == max_rgb) {
				hue = 120.f + (60.f*(b - r))/(value-min_rgb);
			} else {
				hue = 240.f + (60.f*(r - g))/(value-min_rgb);
			}
			if (hue<0)
				hue += 360.f;
		}
	}
	saturation*=255.f;
	value*=255.f;

	if (types == UCHARSRC_UCHARDST || types == FLOATSRC_UCHARDST) {
		uchar *castDst = (uchar *) dst;
		castDst[0] = (uchar) (hue/2.f);
		castDst[1] = (uchar) saturation;
		castDst[2] = (uchar) value;
	} else {
		float *castDst = (float *) dst;
		castDst[0] = hue;
		castDst[1] = saturation;
		castDst[2] = value;
	}
}

void leostorm::colorspaces::HSV2BGR(void *src, void *dst, DataTypeConversion types) {
	float h, s, v;

	if (types == UCHARSRC_FLOATDST || types == UCHARSRC_UCHARDST) {
		uchar *castSrc = (uchar *) src;
		h = castSrc[0]*2.f, s = castSrc[1]/255.f, v = castSrc[2]/255.f;
	} else {
		float *castSrc = (float *) src;
		h = castSrc[0], s = castSrc[1]/255.f, v = castSrc[2]/255.f;
	}
	
	float red, green, blue;

	static const int sector_data[][3] = {{1,3,0}, {1,0,2}, {3,0,1}, {0,2,1}, {0,1,3}, {2,1,0}};
	float tab[4];
    int sector;
    h *= 0.016666666666666666f; // h /= 60;
	while( h >= 6 ) { //In the case of angles greater or equals than k2pi with k>0
		h -= 6;
	}
    sector = (int) floor(h);
    h -= sector;

    tab[0] = v;
    tab[1] = v*(1.f - s);
    tab[2] = v*(1.f - s*h);
    tab[3] = v*(1.f - s*(1.f - h));
    
    blue = tab[sector_data[sector][0]];
    green = tab[sector_data[sector][1]];
    red = tab[sector_data[sector][2]];
	
	blue*=255.f; green*=255.f; red*=255.f;

	if (types == UCHARSRC_UCHARDST || types == FLOATSRC_UCHARDST) {
		uchar *castDst = (uchar *) dst;
		castDst[0] = (uchar) blue;
		castDst[1] = (uchar) green;
		castDst[2] = (uchar) red;
	} else {
		float *castDst = (float *) dst;
		castDst[0] = blue;
		castDst[1] = green;
		castDst[2] = red;
	}
}

void leostorm::colorspaces::BGR2GRAYSCALE(uchar *src, uchar *dst) {
	*dst = (uchar) (0.299*src[2] + 0.587*src[1] + 0.114*src[0]);
}

void leostorm::colorspaces::RGB2BGR(const uchar* rgb, int rgb_step,
							uchar* bgr, int bgr_step, LeoSize<int> size) {
	int i;
	for( ; size.height--; ) {
		for( i = 0; i < size.width; i++, bgr += 3, rgb += 3 ) {
			uchar b, g, r;
			b = rgb[0];
			g = rgb[1];
			r = rgb[2];

			bgr[2] = b; bgr[1] = g; bgr[0] = r;
		}
		bgr += bgr_step - size.width*3;
		rgb += rgb_step - size.width*3;
	}
}

#define xyzXr_32f  0.412453f
#define xyzXg_32f  0.357580f
#define xyzXb_32f  0.180423f

#define xyzYr_32f  0.212671f
#define xyzYg_32f  0.715160f
#define xyzYb_32f  0.072169f

#define xyzZr_32f  0.019334f
#define xyzZg_32f  0.119193f
#define xyzZb_32f  0.950227f

#define  labT_32f   0.008856f
#define labLScale_32f      116.f
#define labLShift_32f      16.f
#define labLScale2_32f     903.3f

#define luvUn_32f  0.19793943f 
#define luvVn_32f  0.46831096f

void leostorm::colorspaces::BGR2LUV(void *src, void *dst, DataTypeConversion types) {
	float b, g, r;

	if (types == UCHARSRC_FLOATDST || types == UCHARSRC_UCHARDST) {
		uchar *castSrc = (uchar *) src;
		b = castSrc[0]/255.f, g = castSrc[1]/255.f, r = castSrc[2]/255.f;
	} else {
		float *castSrc = (float *) src;
		b = castSrc[0]/255.f, g = castSrc[1]/255.f, r = castSrc[2]/255.f;
	}

	float x, y, z;
	float L, u, v, t;

	x = b*xyzXb_32f + g*xyzXg_32f + r*xyzXr_32f;
	y = b*xyzYb_32f + g*xyzYg_32f + r*xyzYr_32f;
	z = b*xyzZb_32f + g*xyzZg_32f + r*xyzZr_32f;

	if( !x && !y && !z )
		L = u = v = 0.f;
	else {
		if( y > labT_32f )
			L = labLScale_32f * pow(y, 1/3.f) - labLShift_32f;
		else
			L = labLScale2_32f * y;

		t = 1.f / (x + 15 * y + 3 * z);            
		u = 4.0f * x * t;
		v = 9.0f * y * t;

		u = 13*L*(u - luvUn_32f);
		v = 13*L*(v - luvVn_32f);
	}

	if (types == UCHARSRC_UCHARDST || types == FLOATSRC_UCHARDST) {
		uchar *castDst = (uchar *) dst;
		float L_mod = 2.55f, u_mod = 0.7203389830508475f, v_mod = 0.9732824427480916f;
		castDst[0] = (uchar) (L_mod*L);
		castDst[1] = (uchar) ((u+134.f)*u_mod);
		castDst[2] = (uchar) ((v+140.f)*v_mod);
	} else {
		float *castDst = (float *) dst;
		castDst[0] = L;
		castDst[1] = u;
		castDst[2] = v;
	}
}

#define xyzRx_32f  3.240479f
#define xyzRy_32f  (-1.53715f)
#define xyzRz_32f  (-0.498535f)

#define xyzGx_32f  (-0.969256f)
#define xyzGy_32f  1.875991f
#define xyzGz_32f  0.041556f

#define xyzBx_32f  0.055648f
#define xyzBy_32f  (-0.204043f)
#define xyzBz_32f  1.057311f

void leostorm::colorspaces::LUV2BGR(void *src, void *dst, DataTypeConversion types) {
	float L, u, v;

	if (types == UCHARSRC_FLOATDST || types == UCHARSRC_UCHARDST) {
		uchar *castSrc = (uchar *) src;
		float L_mod = 0.392156862745098f, u_mod = 1.388235294117647f, v_mod = 1.027450980392157f;
		L = (castSrc[0]*L_mod), u = (castSrc[1]*u_mod)-134.f, v = (castSrc[2]*v_mod)-140.f;
	} else {
		float *castSrc = (float *) src;
		L = castSrc[0], u = castSrc[1], v = castSrc[2];
	}

	float x, y, z, t, u1, v1, b, g, r;

	if( L > 8 ) {
		t = (L + labLShift_32f) * (1.f/labLScale_32f);
		y = t*t*t;
	} else {
        y = L * (1.f/labLScale2_32f);
		L = LEO_MAX( L, 0.001f );
	}

    t = 1.f/(13.f * L);
    u1 = u*t + luvUn_32f;
    v1 = v*t + luvVn_32f;
    x = 2.25f * u1 * y / v1 ;
    z = (12 - 3 * u1 - 20 * v1) * y / (4 * v1);                
               
    b = xyzBx_32f*x + xyzBy_32f*y + xyzBz_32f*z;
    g = xyzGx_32f*x + xyzGy_32f*y + xyzGz_32f*z;
    r = xyzRx_32f*x + xyzRy_32f*y + xyzRz_32f*z;

	b*=255.f; g*=255.f; r*=255.f;

	if (types == UCHARSRC_UCHARDST || types == FLOATSRC_UCHARDST) {
		uchar *castDst = (uchar *) dst;
		castDst[0] = (uchar) b;
		castDst[1] = (uchar) g;
		castDst[2] = (uchar) r;
	} else {
		float *castDst = (float *) dst;
		castDst[0] = b;
		castDst[1] = g;
		castDst[2] = r;
	}

}

float leostorm::colorspaces::LUVChromaticityDistance(float *p1, float *p2) {
	float uDiff = (p1[1] - p2[2]); uDiff*=uDiff;
	float vDiff = (p1[2] - p2[2]); vDiff*=vDiff;
	return sqrt(uDiff + vDiff);
}