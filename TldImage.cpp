//
//  TldImage.cpp
//  majd marian
//
//  Created by admin on 10/15/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "TldImage.h"
#include "Utils.h"
#include <stdlib.h>
static float gaussian_12x12[][12] = {
    0.0000 ,0.0001,0.0002,0.0004,0.0007,0.0009,0.0009,0.0007,0.0004,0.0002,0.0001,0.0000,
    0.0001 ,0.0003,0.0007,0.0015,0.0024,0.0031,0.0031,0.0024,0.0015,0.0007,0.0003,0.0001,
    0.0002 ,0.0007,0.0019,0.0040,0.0065,0.0084,0.0084,0.0065,0.0040,0.0019,0.0007,0.0002,
    0.0004 ,0.0015,0.0040,0.0084,0.0138,0.0177,0.0177,0.0138,0.0084,0.0040,0.0015,0.0004,
    0.0007 ,0.0024,0.0065,0.0138,0.0228,0.0293,0.0293,0.0228,0.0138,0.0065,0.0024,0.0007,
    0.0009 ,0.0031,0.0084,0.0177,0.0293,0.0376,0.0376,0.0293,0.0177,0.0084,0.0031,0.0009,
    0.0009 ,0.0031,0.0084,0.0177,0.0293,0.0376,0.0376,0.0293,0.0177,0.0084,0.0031,0.0009,
    0.0007 ,0.0024,0.0065,0.0138,0.0228,0.0293,0.0293,0.0228,0.0138,0.0065,0.0024,0.0007,
    0.0004 ,0.0015,0.0040,0.0084,0.0138,0.0177,0.0177,0.0138,0.0084,0.0040,0.0015,0.0004,
    0.0002 ,0.0007,0.0019,0.0040,0.0065,0.0084,0.0084,0.0065,0.0040,0.0019,0.0007,0.0002,
    0.0001 ,0.0003,0.0007,0.0015,0.0024,0.0031,0.0031,0.0024,0.0015,0.0007,0.0003,0.0001,
    0.0000 ,0.0001,0.0002,0.0004,0.0007,0.0009,0.0009,0.0007,0.0004,0.0002,0.0001,0.0000};



unsigned char* blur_image(const unsigned char* values, int width, int height) {
	unsigned char* blurred = (unsigned char*) malloc (sizeof(unsigned char) * width * height);
    for(int y=0; y<height; y++) {
		for(int x=0; x<width; x++) {
			int val=0;
            
			for(int dy=0; dy<12; dy++) {
				for(int dx=0; dx<12; dx++) {
                    int y_offset = dy+y-5;
                    int x_offset = dx+x-5;
                    if(y_offset <0)
                        y_offset =0;
                    if(x_offset < 0)
                        x_offset = 0;
                    if(y_offset >= height)
                        y_offset = height - 1;
                    if(x_offset >= width)
                        x_offset = width-1;
                    int pixel=values[y_offset*width + x_offset];
					val+=pixel * gaussian_12x12[dy][dx];
				}
			}

            blurred[y*width+x] = val;
		}
	}
    return blurred;
    
}


unsigned char* flip_image(const unsigned char* values, int width, int height) {
    unsigned char* flipped = (unsigned char*) malloc (sizeof(unsigned char) * width * height);
    for(int i =0 ; i < height ;i++ ) {
        for(int j =0 ; j < width ; j++) {
             flipped[j*width + (height-i-1)] = values[i*width+j];
        }
    }
    return flipped;
}

TldImage::TldImage() {}


void TldImage::createFromImage( unsigned char* values, int _width, int _height, bool blur) {
    // Get pointer
   // const unsigned char *values = image;
    width = _width;
    height = _height;
    if (blur) {
        data = blur_image(values,_width,_height);;
    } else {
        data = values;   
    }  
}
void TldImage::createWarp(TldImage *image, double *bb, float *m) {
    // Initialise variables
    // Remember an TldImage has dimensions (width + 1)x(height + 1)
    width = (int)bb[2];
    height = (int)bb[3];
    
    //planar_data = new int [(width+1)*(height+1)];
    data=(unsigned char*)malloc((width)*(height)*sizeof(unsigned char));
    memset(data,0,(width)*(height)*sizeof(unsigned char));
    
    unsigned char *imageData = image->getData();
    
    // Get centre of bounding-box (cx, cy) and the offset relative to this of
    // the top-left of the bounding-box (ox, oy)
    int ox = -(int)((width) * 0.5);
    int oy = -(int)((height) * 0.5);
    int cx = (int)(bb[0] - ox);
    int cy = (int)(bb[1] - oy);
    
    // Loop through pixels of this image, width then height, calculating the
    // position of corresponding pixels in the source image
    for (int x = ox; x < ox + width; x++) {
        for (int y = oy; y < oy + height; y++) {
            int xp = (int)(m[0] * (float)x + m[1] * (float)y + cx);
            int yp = (int)(m[2] * (float)x + m[3] * (float)y + cy);
            
            // Limit pixels to those in the given bounding-box
            xp = std::max(std::min(xp, (int)bb[0] + width), (int)bb[0]);
            yp = std::max(std::min(yp, (int)bb[1] + height), (int)bb[1]);
            
            data[(x - ox) + (y - oy)*(width)] = imageData[xp + yp*image->getWidth()];
        }
    }
    //width++;
    //height++;
    
}



int TldImage::getWidth() {
    return width;
}


int TldImage::getHeight() {
    return height;
}


unsigned char *TldImage::getData() {
    return data;
}


void TldImage::setData(unsigned char *d) {
	data = d;
}


TldImage::~TldImage() {
    // Only delete all the data if it was created from Matlab
    
    if(data!=NULL)
    	free(data);
}