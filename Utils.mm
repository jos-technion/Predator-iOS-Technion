//
//  Utils.mm
//  majd marian
//
//  Created by admin on 10/15/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <opencv2/opencv.hpp>
#include "Utils.h"
#include "TldDetector.h"


IplImage *imageFromCharBlurred(const unsigned char* values, CvSize *frameSize, int frameWidth, int frameHeight) {
    
	int blur[5][5]={ 
		{ 1, 4, 7, 4, 1 },
		{ 4,16,26,16, 4 },
		{ 7,26,41,26, 7 },
		{ 4,16,26,16, 4 },
		{ 1, 4, 7, 4, 1 }};
    
	IplImage *image = cvCreateImage(*frameSize, IPL_DEPTH_8U, 1);
	for(int y=0; y<frameHeight-5; y++) {
		for(int x=0; x<frameWidth-5; x++) {
			int val=0;
			for(int dy=0; dy<5; dy++) {
				for(int dx=0; dx<5; dx++) {
					int pixel=values[(y+dy)*frameWidth + x+dx];
					val+=pixel*blur[dy][dx];
				}
			}
            image->imageData[y*frameWidth+x] = val/273;
		}
	}

	return image;
}

IplImage *imageFromChar(const unsigned char* values, CvSize *frameSize, int frameWidth, int frameHeight) {
    
    IplImage *image = cvCreateImage(*frameSize, IPL_DEPTH_8U, 1);
    int widthStep = image->widthStep;
    for (int i = 0; i < frameHeight; i++) {
        for (int j = 0; j < frameWidth; j++) {
            image->imageData[i * widthStep + j] = values[i * frameWidth + j];
        }
    }
    return image;
}

void bbWarpPatch(TldImage *frame,double *bb, Classifier *classifier) {
    
    float *m = new float[4];
    int count = 0;
    for (float r = -0.1f; r < 0.1f; r += 0.005f) {
        float sine = sin(r);
        float cosine = cos(r);
        
        for (float sx = -0.1f; sx < 0.1f; sx += 0.05f) {
            for (float sy = -0.1f; sy < 0.1f; sy += 0.05f) {
                /*  Rotation matrix * translation matrix =
                 
                 | cos r   sin r | * | 1   sx | = 
                 | -sin r  cos r |   | sy   1 |
                 
                 | cos r + sy * sin r   sx * cos r + sin r |
                 | sy * cos r - sin r   cos r - sx * sin r | */
                m[0] = cosine + sy * sine;
                m[1] = sx * cosine + sine;
                m[2] = sy * cosine - sine;
                m[3] = cosine - sx * sine;
                
                TldImage *warp = new TldImage();
                warp->createWarp(frame, bb, m);
                classifier->train(warp, 1, 1, (int)bb[2], (int)bb[3], 1);
                count++;
                delete warp;
            }
        }
    }
    
    delete m;
}


void trainClassifier(TldImage *frame, double *tbb, Classifier * classifier) {
    // Set the width and height that are used as 1 * scale.
    // If tbb is NULL, we are not tracking and use the first-frame
    // bounding-box size, otherwise we use the tracked bounding-box size
    float baseWidth, baseHeight;
    int width = frame->getWidth();
    int height = frame->getHeight();
    int countNeg = 0;
    int countPos = 0;
    baseWidth = (float)tbb[2];
    baseHeight = (float)tbb[3];

        
    
    // Using the sliding-window approach, find positive matches to our object
    // Vector of positive patch matches' bounding-boxes
    
    // Minimum and maximum scales for the bounding-box, the number of scale
    // iterations to make, and the amount to increment scale by each iteration
    float minScale = 0.5f;
    float maxScale = 1.5f;
    //  int iterationsScale = 6;
    int iterationsScale = 6;
    float scaleInc = (maxScale - minScale) / (iterationsScale - 1);
    
    // Loop through a range of bounding-box scales
    for (float scale = minScale; scale <= maxScale; scale += scaleInc) {
        int minX = 0;
        int currentWidth = (int)(scale * (float)baseWidth);
        if(currentWidth>=width)currentWidth=width-1;
        int maxX = width - currentWidth;
        //float iterationsX = 30.0;
        float iterationsX = 100;
        int incX = (int)floor((float)(maxX - minX) / (iterationsX - 1.0f));
        if(incX==0)incX=1;
        
        // Same for y
        int minY = 0;
        int currentHeight = (int)(scale * (float)baseHeight);
        if(currentHeight>=height)currentHeight=height-1;
        int maxY = height - currentHeight;
        //float iterationsY = 30.0;
        float iterationsY = 40;
        int incY = (int)floor((float)(maxY - minY) / (iterationsY - 1.0f));
        if(incY==0)incY=1;
        //float pmax = 0;
        // Loop through all bounding-box top-left x-positions
        for (int x = minX; x <= maxX; x += incX) {
            
            
            
            // Loop through all bounding-box top-left x-positions
            for (int y = minY; y <= maxY; y += incY) {
                // Classify the patch
                double *bb = new double[6];
                bb[0] = (double)x;
                bb[1] = (double)y;
                bb[2] = (double)currentWidth;
                bb[3] = (double)currentHeight;
                
                float overlap = TldDetector::bbOverlap(tbb, bb);
                if (overlap ==0) {
                    float p = classifier->classify(frame, x, y, currentWidth, currentHeight);
                    if(p > 0.15) {
                         classifier->train(frame,  x, y, currentWidth, currentHeight, 0);
                         countNeg++;
                    }
                } else if (overlap > MIN_LEARNING_OVERLAP) {
                    classifier->train(frame, x, y, currentWidth, currentHeight, 1);
                    countPos++;
                }
                
                delete [] bb;
            }
        }
    }
    countPos++;
    printf("%d\n",countPos-1);
}


int ntuples (double ***matrix,double *a,int size_a,double* b,int size_b) {
    int i,j,k;
    int size= size_a*size_b;
    *matrix= (double**)malloc(sizeof(double*)*size);
    for(i =0;i < size;i++) {
        (*matrix)[i] = (double*) malloc(2*sizeof(double));
    }
    k=0;
    for( i=0; i < size_a; i++) {
        for( j=0; j < size_b; j++) {
            (*matrix)[k][0]= a[i];
            (*matrix)[k][1]= b[j];
            k++;
        }
    }
    return size;
    
}
/* repmatrix:
 repeate matrix, repeates the given matrix m*n times, in the following order:\
 n
 [matrix  matrix  matrix .... matrix] 
 [matrix  matrix  matrix .... matrix]
 [ ....                             ] m
 [matrix matrix matrix   .... matrix] 
 
 returns the size of the new matrix
 */
int repmatrix(double ***rep_matrix,double **org_matrix,int size_m,int size_n,int m,int n) {
    int i,j,k,l;
    *rep_matrix= (double**) malloc(sizeof(double*) * m *size_m);
    for(i = 0; i < m *size_m; i++) {
        (*rep_matrix)[i] = (double*) malloc(sizeof(double) * size_n* n);
    }
    for( i=0; i < size_m; i++ ) { 
        for(j=0; j < size_n;j ++) {
            for(k=0;k < m; k++) {
                for(l = 0; l < n; l++) {
                    (*rep_matrix)[i+size_m*k][j+size_n*l] = org_matrix[i][j];
                }
            }
        }
    }
    return n*size_n*m*size_m;
}

/* create_array:
 puts in array, all the values starting from: from, to: to, increment by: by
 the function returns the size of the array.
 */

int create_array(double **array,double from,double to,double by){
    int i;
    double j;
    int size =(to-from)/by;
    *array= (double*)malloc(sizeof(double)*size);
    j=from;
    for( i=0; i < size; i++ ) {
        (*array)[i]= j;
        j+= by;
    }
    return size;
}

void free_matrix(double **array, int n) {
    for(int i=0; i< n; i++) {
        free(array[i]);
    }
}


// correlation normalized
double ccorr_normed(unsigned char *f1,unsigned char *f2,int numDim) {
	double corr = 0;
	double norm1 = 0;
	double norm2 = 0;
    
	for (int i = 0; i<numDim; i++) {
		corr += f1[i]*f2[i];
		norm1 += f1[i]*f1[i];
		norm2 += f2[i]*f2[i];
	}
	// normalization to <0,1>
	return (corr / sqrt(norm1*norm2) + 1) / 2.0;
}


