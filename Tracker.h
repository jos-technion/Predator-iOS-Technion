#pragma once
#include <opencv2/opencv.hpp>
#include "TldImage.h"
#include "Classifier.h"
#include <math.h>

using namespace cv;


// Constants -----------------------------------------------------------------
// The number of points in a single dimension on the bounding-box
#define DIM_POINTS 10

// Total number of points on the bounding-box
#define TOTAL_POINTS (DIM_POINTS * DIM_POINTS)

// Defines the size of the search window in cvCalcOpticalFlowPyrLK
#define WINDOW_SIZE 4

// Maximal pyramid level number
#define LEVEL 5


#define USE_BACKWARD_TRACKING 1

class Tracker {
    // Private ===============================================================
    private:
    //! the width of each frame
    int width;
    //! the height of each frame
    int height;
    
    //! Previous video stream frame
    IplImage *prevFrame;
    
    //! Buffers for the nprevious pyramids used by cvCalcOpticalFlowPyrLK
    IplImage *prevPyramid;
    //! Buffers for the next pyramids used by cvCalcOpticalFlowPyrLK
    IplImage *nextPyramid;
    
   
    //! These points are those that are tracked
     //! The previous coordinates of the points placed in the bounding-box
    CvPoint2D32f *prevPoints;
     //! The next coordinates of the points placed in the bounding-box
    CvPoint2D32f *nextPoints;
    //! predPoints = predicted 1st frame points from tracking backwards
    CvPoint2D32f *predPoints;
    
    //!Size of the search window of each pyramid level in cvCalcOpticalFlowPyrLK
    CvSize *windowSize;
    
    //! Status output array of cvCalcOpticalFlowPyrLK. Elements are set to 1 if
    //! the flow for the corresponding feature has been found, otherwise 0
    //! status is for forward tracking
    char *status;
    //! predStatus is for backward tracking
    char *predStatus;
    
    //! Specifies the termination criteria of the iterative search algorithm in cvCalcOpticalFlowPyrLK
    TermCriteria *termCriteria;
    
    //! Pointer to the classifier for the entire program
    Classifier *classifier;
    
    //! calculates the median
    /*!
        \param A the array
        \param length length of array A
        \return the median
    */
    float median(float *A, int length);
    
    
    // Public ================================================================
    public:
    //! Uses normalized cross correlation in order to test the correlation between the two supplied array of points on the two images.
    /*!
         \param imgI the first image
         \param imgJ the second image
         \param points0 the first set of points
         \param point1 the second set of points
         \param nPts number of points
         \param status status as got from LK
         \param match an output array will have the match percentage of each point
         \param winsize window size
         \param method method to do the NCC
     */
    void normCrossCorrelation(IplImage *imgI, IplImage *imgJ, CvPoint2D32f *points0, CvPoint2D32f *points1, int nPts, char *status, float *match,int winsize, int method);
    
    //! Uses euclidena distance in order to test the distance between the two supplied array of points.
    /*!
     \param points0 the first set of points
     \param point1 the second set of points
     \param nPts number of points
     \param match an output array will have the match percentage of each point
   */

    void euclideanDistance (CvPoint2D32f *point1, CvPoint2D32f *point2, float *match, int nPts);
    
    //!  Constructor. Initialises the tracker.
    /*!    
        \param frameWidth width of the video stream frames
        \param frameHeight height of the video stream frames
        \param frameSize size of the video frame as a CvSize object
        \param firstFrame the first video stream frame
        \param classifier pointer to the classifier for the program
     */
    Tracker(int frameWidth, int frameHeight, CvSize *frameSize, IplImage *firstFrame, Classifier *classifier);
    
    //!  Tracks the region of the input frame indicated by the given bounding-box.
    /*!
         1) Initialise uniformly distributed set of points over the previous
         frame's trajectory patch
         2) Track these points using the Lucas-Kanade algorithm.
         3) Set the bounding-box scale and position for the current frame based on
         the median scale and position changes of the remaining points
     
        \param nextFrameIpl IplImage of the next frame to track
        \param nextFrame next video stream frame
        \param bb array containing the trajectory bounding-box  [x, y, width, height] 
        \return  Returns the estimated new bounding box [x, y, width, height, confidence].
        
    */
    double *track(IplImage *nextFrameIpl, TldImage *nextFrame, double *bb);
    
    //!  Tracks the region of the input frame indicated by the given bounding-box.
    /*!
     1) Initialise uniformly distributed set of points over the previous
     frame's trajectory patch
     2) Track these points using the Lucas-Kanade algorithm
     3) Estimate the tracking error of each point
     4) Filter out the tracked points with greatest error (typically 50% of
     points using NCC and ED)
     5) Set the bounding-box scale and position for the current frame based on
     the median scale and position changes of the remaining points
     
     \param nextFrameIpl IplImage of the next frame to track
     \param nextFrame next video stream frame
     \param bb array containing the trajectory bounding-box  [x, y, width, height] 
     \return  Returns the estimated new bounding box [x, y, width, height, confidence].
     
     */
    double *trackWithBackward(IplImage *nextFrame, TldImage *nextFrameIntImg, double* bb);
    
    
    //!  Setter for prevFrame (also frees the current value of prevFrame). 
    void setPrevFrame(IplImage *frame);
    
    //!  Destructor. 
    ~Tracker();
};
