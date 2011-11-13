
#pragma once
#ifndef __TLD_DETECTOR__
#define __TLD_DETECTOR__

#include "Classifier.h"
#include "Tld.h"
#include <vector>


using namespace std;


#define MIN_LEARNING_OVERLAP 0.6


class TldDetector {
    private:
    //! Pointer to the classifier for the entire program
    Classifier *classifier;
    
    //! width of each frame
    int width;
    //! height of each frame
    int height;
    
    //! Initial size of the  width in bounding-box
    float initBBWidth;
     //! Initial size of the height in bounding-box
    float initBBHeight;
    
    //! number of iterations for width
    int WIDTH_STEPS ;
    //! number of iterations for height
    int HEIGHT_STEPS;

    
    // Public ================================================================
    public:
    //!  Constructor.
    /*!
        \param frameWidth width of the video stream frames
        \param frameHeight height of the video stream frames
        \param bb array containing the trajectory bounding-box
            [x, y, width, height]
        \param classifier pointer to the classifier for the program 
        \param WIDTH_STEPS number of iteration on the width
        \param HEIGHT_STEPS number of iteration on the height
     */
    TldDetector(int frameWidth, int frameHeight, double *bb, Classifier *classifier, int WIDTH_STEPS, int HEIGHT_STEPS);
    
    //!  Detects the object in the given frame.
    /*!
       \param frame current frame as an TldImage; this is NOT freed
        \param tbb tracked bounding-box this frame [x, y, width, height]
        \return a pointer to a vector of bounding-box arrays each
        containing [x, y, width, height, confidence, overlapping] that
        are either positive, or negative and overlapping with the
        trajectory bounding-box.
     */
    vector<double *> *detect(TldImage *frame, double *tbb);
    
    //!*  calculate the intersection between two bounding boxes as a percentage of their total area.
       /*!
        \param bb1 first bounding-box [x, y, width, height]
        \param bb2 second bounding-box [x, y, width, height] 
        \return the intersection between two bounding boxes as a percentage of their total area.
        */
    static double bbOverlap(double *bb1, double *bb2);
    
    //!  Destructor.
    ~TldDetector();
};
#endif