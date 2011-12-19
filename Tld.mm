  //
//  Tld.mm
//  Predator
//
//  Created by admin on 10/25/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//


#include "Tld.h"
#include "Utils.h"
#include "TldImage.h"
#include "Classifier.h"
#include "TldDetector.h"
#include "Tracker.h"
#include <stdlib.h>
#include <math.h>




static bool is_initialized = 0;

static float initBBWidth;
static float initBBHeight;
static IplImage *img2 = 0;
int frameHeight;
int frameWidth;

CvSize *frameSize;

double confidence;

static Classifier *classifier;
static Tracker *tracker;
static TldDetector *detector;
IplImage *firstPatch;
bool video = false;
/*
 size: is the size of the frame
 frame: is the first frame
 box: is the initiale box data:
 [x, y, width, height]
 
 */


Tld::Tld(int width, int height, unsigned char* frame, double* bb, int nTREES, int nFEAT, double MIN_FEATURE_SCALE, double MAX_FEATURE_SCALE, int WIDTH_STEPS, int HEIGHT_STEPS, bool is_video, bool blur) {
    video = is_video;
    srand(0);
    printf("Trees: %d, Features: %d, Min scale: %lf, Max scale: %lf, Width steps: %d, Height steps: %d\n",nTREES, nFEAT, MIN_FEATURE_SCALE, MAX_FEATURE_SCALE, WIDTH_STEPS, HEIGHT_STEPS);
    frameWidth = width;
    frameHeight = height;
    frameSize = (CvSize *)malloc(sizeof(CvSize));
    *frameSize = cvSize(frameWidth, frameHeight);
    TldImage *firstFrame = new TldImage();
    firstFrame->createFromImage(frame, width, height,blur);
    IplImage *firstFrameIplImage = imageFromChar(frame, frameSize, frameWidth, frameHeight);
    cvSetImageROI(firstFrameIplImage, cvRect(bb[0], bb[1], bb[2],bb[3]));
    /* create destination image
     Note that cvGetSize will return the width and the height of ROI */
    img2 = cvCreateImage(cvGetSize(firstFrameIplImage),
                                   firstFrameIplImage->depth,
                                   firstFrameIplImage->nChannels);
    
    /* copy subimage */
    cvCopy(firstFrameIplImage, img2, NULL);
    /* always reset the Region of Interest */
    cvResetImageROI(firstFrameIplImage);
    IplImage *firstFrameIplImgPatch = cvCreateImage(cvSize(15,15),  
                                                    firstFrameIplImage->depth,
                                                    firstFrameIplImage->nChannels);
    cvResize(img2, firstFrameIplImgPatch);
    firstPatch = firstFrameIplImgPatch;
    cvReleaseImage(&img2);
   // cvReleaseImage(&firstFrameIplImage);
    
    initBBWidth = (float)bb[2];
    initBBHeight = (float)bb[3];
    confidence = 1.0f;
    
    // Initialise classifier, Tracker and detector
    srand((unsigned int)time(0));
    classifier = new Classifier(nTREES, nFEAT);
    tracker = new Tracker(frameWidth, frameHeight, frameSize, firstFrameIplImage, classifier);
    detector = new TldDetector(frameWidth, frameHeight, bb, classifier, WIDTH_STEPS, HEIGHT_STEPS);
    
    // Train the classifier on the bounding-box patch and warps of it
    classifier->train(firstFrame,  (int)bb[0], (int)bb[1], (int)initBBWidth, (int)initBBHeight, 1);
    bbWarpPatch(firstFrame, bb, classifier);
    //trainNegative(firstFrame,  bb, initBBWidth, initBBHeight, classifier);
    // trainNegative(firstFrame,  bb, frameWidth, frameHeight, classifier);
    trainClassifier(firstFrame, bb, classifier);
    // Free memory and set initialised
    float p = classifier->classify(firstFrame,  bb[0],bb[1],bb[2],bb[3]);
    printf("Classifier: %lf\n",p);
    delete firstFrame;
    is_initialized = true;
    
    return;
}

void Tld::tldProcessFrame(int width, int height, unsigned char* NewImage,double * ttbb,double * outPut, double MIN_TRACKING_CONF, double MIN_REINIT_CONF, double MIN_LEARNING_CONF, bool blur, bool learning) {
    
    
    IplImage *nextFrame = imageFromChar(NewImage, frameSize, frameWidth, frameHeight);
    TldImage *nextFrameIntImg = new TldImage();
    nextFrameIntImg->createFromImage(NewImage,width,height,blur);
    double *bb = ttbb;
    
    
    double *tbb;
    vector<double *> *dbbs;
   // printf("Tracking: %lf, Reinit: %lf, Learning: %lf\n",MIN_TRACKING_CONF, MIN_REINIT_CONF, MIN_LEARNING_CONF);
    
    if (confidence > MIN_TRACKING_CONF) {
        tbb = tracker->track(nextFrame, nextFrameIntImg, bb);
        float p = classifier->classify(nextFrameIntImg,  tbb[0],tbb[1],tbb[2],tbb[3]);
        printf("Classifier: %lf\n",p);
        if (tbb[4] > MIN_TRACKING_CONF)
		{
			dbbs = detector->detect(nextFrameIntImg, tbb);
            
		} 
		else
		{
			dbbs = detector->detect(nextFrameIntImg, NULL);
            
		}
        //  learn = true;
    } else {
        dbbs = detector->detect(nextFrameIntImg, NULL);
        //dbbs = detector->detect(nextFrameIntImg, bb);
        tracker->setPrevFrame(nextFrame);
        tbb = new double[5];
        tbb[0] = 0;
        tbb[1] = 0;
        tbb[2] = 0;
        tbb[3] = 0;
        tbb[4] = MIN_TRACKING_CONF;
    }
    // dbb
    
    double dbbMaxConf = 0.0f;
    int dbbMaxConfIndex = -1;
    float dbb_conf = 0;
    float *nccs;
    if (dbbs->size() != 0) {
        nccs = (float*)malloc(sizeof(float)* dbbs->size());
    }
    for (int i = 0; i < dbbs->size(); i++) {
        double dbbConf = dbbs->at(i)[4];
        cvSetImageROI(nextFrame, cvRect(dbbs->at(i)[0], dbbs->at(i)[1], dbbs->at(i)[2],dbbs->at(i)[3]));
        IplImage *dbb_img;
        dbb_img = cvCreateImage(cvGetSize(nextFrame),
                                nextFrame->depth,
                                nextFrame->nChannels);
        
        cvCopy(nextFrame, dbb_img, NULL);
        cvResetImageROI(nextFrame);
        IplImage *dbb_patch = cvCreateImage(cvSize(15,15),  
                                            nextFrame->depth,
                                            nextFrame->nChannels);
        cvResize(dbb_img, dbb_patch);
        
        cvReleaseImage(&dbb_img);
        
        dbb_conf = ccorr_normed((unsigned char*)firstPatch->imageData, (unsigned char*)dbb_patch->imageData,15*15);
        nccs[i] = dbb_conf;
        cvReleaseImage(&dbb_patch);
        cvSetImageROI(nextFrame, cvRect(tbb[0], tbb[1], tbb[2],tbb[3]));

        if (dbb_conf > dbbMaxConf && dbbConf > 0.6) {
            dbbMaxConf = dbb_conf;
            dbbMaxConfIndex = i;
        }
     }
    
    float tbb_conf;
    if ( tbb[4] != 0 && tbb[2] >=15 && tbb[3] >=15) {
        IplImage *tbb_img = cvCreateImage(cvGetSize(nextFrame),
                             nextFrame->depth,
                             nextFrame->nChannels);
        cvCopy(nextFrame, tbb_img, NULL);
        IplImage *tbb_patch = cvCreateImage(cvSize(15,15),  
                                              nextFrame->depth,
                                              nextFrame->nChannels);
        cvResize(tbb_img, tbb_patch);
        cvReleaseImage(&tbb_img);
        
        //printf("My best confidence: %lf - NCC: %lf\n",dbbMaxConf,dbb_conf);
        tbb_conf = ccorr_normed((unsigned char*)firstPatch->imageData, (unsigned char*)tbb_patch->imageData,15*15);
        cvReleaseImage(&tbb_patch);
       // printf("My tbb confidence: %lf\n",tbb_conf);
    } else {
        tbb_conf = 0;
    }
    printf("Detector best confidence: %lf - NCC: %lf\n",dbbMaxConf,dbb_conf);
    printf("Tracker best confidence: %lf - NCC: %lf\n",tbb[4],tbb_conf);
    if (dbbMaxConf > MIN_REINIT_CONF && dbbMaxConf > tbb[4] && dbb_conf > (video ? 0.943 :0.95) && dbb_conf > tbb_conf) {
        delete tbb;
        tbb = new double[5];
        double *dbb = dbbs->at(dbbMaxConfIndex);
        tbb[0] = dbb[0];
        tbb[1] = dbb[1];
        tbb[2] = dbb[2];
        tbb[3] = dbb[3];
        tbb[4] = dbb[4];
    } else  if (tbb[4] > MIN_TRACKING_CONF && confidence > MIN_LEARNING_CONF && tbb_conf > (video ? 0.94 :0.94)) {
        for (int i = 0; i < dbbs->size(); i++) {
            
            double *dbb = dbbs->at(i);
            
            if (dbb[5] == 1 && nccs[i] > 0.91) {
                
                if (learning) {
                    printf("Learning positive patch!\n");
                
                classifier->train(nextFrameIntImg,  (int)dbb[0], (int)dbb[1], (int)dbb[2], (int)dbb[3], 1);
                }
            }
            else if (dbb[5] == 0 && nccs[i] < 0.999 ) {
                // printf("Learning negative patch :(\n");
                if (learning)
                classifier->train(nextFrameIntImg,  (int)dbb[0], (int)dbb[1], (int)dbb[2], (int)dbb[3], 0);
            }
        }
        if (learning)
        classifier->train(nextFrameIntImg,  (int)tbb[0], (int)tbb[1], (int)tbb[2], (int)tbb[3], 1);
        
    } else {
        tbb = new double[5];
        tbb[0] = 0;
        tbb[1] = 0;
        tbb[2] = 0;
        tbb[3] = 0;
        tbb[4] = MIN_TRACKING_CONF;
    }
    confidence = tbb[4];
    
    
	outPut[0] = tbb[0];
	outPut[1] = tbb[1];
	outPut[2] = tbb[2];
	outPut[3] = tbb[3];
    
    
    
    
    free(tbb);
    dbbs->clear();
    delete nextFrameIntImg;
}

