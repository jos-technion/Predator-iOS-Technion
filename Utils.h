// Copyright 2011 Zdenek Kalal
//
// This file is part of TLD.
// 
// TLD is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// TLD is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with TLD.  If not, see <http://www.gnu.org/licenses/>.


#ifndef __TLD_UTILS
#define __TLD_UTILS
#include <opencv2/opencv.hpp>
#include "Classifier.h"
#include "TldImage.h"
#include "Parameters.h"
#include <stdlib.h>
#include <math.h>



//! bbWarpPatch creating warps from the initiale patch and teach the classifier
/*!
 \param frame the first frame
 \param bb the bounding-box
 \param classifier pointer to the classifier
 */
void bbWarpPatch(TldImage *frame,double *bb, Classifier *classifier);

//! imageFromChar  create an IplImage from char*
/*!
 \param values the data of the image
 \param frameSize frame size
 \param frameWidth frame width
 \param frameHeight frame height
  \return the image
 */
IplImage *imageFromChar(const unsigned char* values, CvSize *frameSize, int frameWidth, int frameHeight);

//! ntuples create tuples from array a and array b 
/*!
 \param  matrix the returned array that contain the tuples
 \param a first array 
 \param size_a size of a
 \param b second array 
 \param size_b size of b
  \return the number of the tuples
 */
int ntuples (double ***matrix,double *a,int size_a,double* b,int size_b);

//! repmatrix repeat matrix mxn times
/*!
 \param rep_matrix the repeated matrix
 \param org_matrix the original matrix
 \param size_m size_m of the original matrix
 \param size_n size_n of the original matrix
 \param m  width
 \param n height
 \return
 */
int repmatrix(double ***rep_matrix,double **org_matrix,int size_m,int size_n,int m,int n);

//! create_array
/*!create an array and initaite it all the values starting from: from, to: to, increment by: by
 \param array a pointer to create the array
 \param from start the initiation from
 \param  to end the initiated in
 \param  by every time you step by
  \return an initiated array 
 */
int create_array(double **array,double from,double to,double by);

//! free_matrix free the given matrix
/*!
 \param array the array to be free
 \param n the array size
 */
void free_matrix(double **array,  int n);

//! trainClassifier trains the classifier 
/*!
 \param frame the frame 
 \param tbb the bounding box 
 \param classifier the classifier
 */
void trainClassifier(TldImage *frame, double *tbb, Classifier * classifier);

//!
/*!
 \param f1
 \param f2
 \param numDim
 \return
 */
double ccorr_normed(unsigned char *f1,unsigned char *f2,int numDim) ;

 #endif





