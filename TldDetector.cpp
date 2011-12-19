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


#include "TldDetector.h"
//static int WIDTH_STEPS = 30;
//static int HEIGHT_STEPS = 30;

TldDetector::TldDetector(int frameWidth, int frameHeight, double *bb, Classifier *classifier, int WIDTH_STEPS, int HEIGHT_STEPS) {
    width = frameWidth;
    height = frameHeight;
    initBBWidth = (float)bb[2];
    initBBHeight = (float)bb[3];
    this->classifier = classifier;
    this->WIDTH_STEPS = WIDTH_STEPS;
    this->HEIGHT_STEPS = HEIGHT_STEPS;
}


double TldDetector::bbOverlap(double *bb1, double *bb2) {
    // Check whether the bounding-boxes overlap at all
    if (bb1[0] > bb2[0] + bb2[2]) {
        return 0;
    }
    else if (bb1[1] > bb2[1] + bb2[3]) {
        return 0;
    }
    else if (bb2[0] > bb1[0] + bb1[2]) {
        return 0;
    }
    else if (bb2[1] > bb1[1] + bb1[3]) {
        return 0;
    }
    // If we got this far, the bounding-boxes overlap
    double overlapWidth = min(bb1[0] + bb1[2], bb2[0] + bb2[2]) - max(bb1[0], bb2[0]);
    double overlapHeight = min(bb1[1] + bb1[3], bb2[1] + bb2[3]) - max(bb1[1], bb2[1]);
    double overlapArea = overlapWidth * overlapHeight;
    double bb1Area = bb1[2] * bb1[3];
    double bb2Area = bb2[2] * bb2[3];
    
    return overlapArea / (bb1Area + bb2Area - overlapArea);
}


vector<double *> *TldDetector::detect(TldImage *frame, double *tbb) {
    // Set the width and height that are used as 1 * scale.
    // If tbb is NULL, we are not tracking and use the first-frame
    // bounding-box size, otherwise we use the tracked bounding-box size
    float baseWidth, baseHeight;
    
    if (tbb != NULL) {
        baseWidth = (float)tbb[2];
        baseHeight = (float)tbb[3];
    } else {
        baseWidth = initBBWidth;
        baseHeight = initBBHeight;
    }
    
    if (baseWidth < 15 || baseHeight < 15) {
		cout<<"BB is too small"<<endl;
        return new vector<double *>();
    }
    
    
    /* Using the sliding-window approach, we find both:
        matches that are classified positive but do not overlaps. (if we tracked the object)
        matches that are classified positive and do overlap
        matches that are classified negative and do overlap
     
     
     */
    vector<double *> *bbs = new vector<double *>();
    
    // Minimum and maximum scales for the bounding-box
    float minScale = 0.5f;
    float maxScale = 1.5f;
    
    // number of iteration to make between minScale and maxScale
    int iterationsScale = 6;
    
    float scaleInc = (maxScale - minScale) / (iterationsScale - 1);
    
    
    // Loop through all scales
    for (float scale = minScale; scale <= maxScale; scale += scaleInc) {
        
        int minX = 0; //minimum x position 
        // We calculate the current width and make sure we are still in the image
        int currentWidth = (int)(scale * (float)baseWidth);
        if (currentWidth >= width) 
            currentWidth=width-1;
        
        // maximum x position
        int maxX = width - currentWidth;
        
        // number of iteration to make between minX and maxX
        float iterationsX  = WIDTH_STEPS;
        int incX = (int)floor((float)(maxX - minX) / (iterationsX - 1.0f));
        if (incX == 0)
            incX=1;
        
        // Same for y
        int minY = 0;
        int currentHeight = (int)(scale * (float)baseHeight);
        if (currentHeight >= height)
            currentHeight=height-1;
        int maxY = height - currentHeight;

        
        float iterationsY = HEIGHT_STEPS;
        int incY = (int)floor((float)(maxY - minY) / (iterationsY - 1.0f));
        if (incY == 0)
            incY=1;
        
        // Loop through all bounding-box top-left x-positions
        for (int x = minX; x <= maxX; x += incX) {
            
            
            
            // Loop through all bounding-box top-left x-positions
            for (int y = minY; y <= maxY; y += incY) {
                // Classify the patch
                float p = classifier->classify(frame,x, y, currentWidth, currentHeight);
               // printf("Classifing as: %lf\n",p);
                // Store the patch data in an array
                // [x, y, width, height, confidence, overlapping], where
                // overlapping is 1 if the bounding-box overlaps with the
                // tracked bounding box, otherwise 0
                double *bb = new double[6];
                bb[0] = (double)x;
                bb[1] = (double)y;
                bb[2] = (double)currentWidth;
                bb[3] = (double)currentHeight;
                bb[4] = (double)p;
                
                if (tbb != NULL && bbOverlap(bb, tbb) > MIN_LEARNING_OVERLAP) {
                    bb[5] = 1;
                } else {
                    bb[5] = 0;
                }
                
                // if it overlaps the bb or, the classifier has a confidence of 0.15 that it positive,
                // we return those patches for the learning process.

                if (p > 0.3f || bb[5] == 1) {
                    //if(p > 0.5f || bb[5] == 1) {
                    bbs->push_back(bb);
                } else {
                    delete [] bb;
                }
            }
        }
    }
    return bbs;
}


TldDetector::~TldDetector() {
    
}
