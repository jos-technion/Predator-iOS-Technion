//
//  TldImage.h
//  majd marian
//
//  Created by admin on 10/15/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#pragma once
#include <algorithm>

class TldImage {
    // Private ===============================================================
private:
   
    
    //! Dimensions of the image
    int width, height;
    
    
    // Public ================================================================
public:
    //! Image data as vector
    unsigned char *data;
    /*!  Constructor. */
    TldImage(void);
    
    //! Creates the image from unsigned char* data 
     /*!
      \param img the image data in single dimention array
      \param width image width
      \param height image height
      \param blur wether to blur the image or not
     */
    void createFromImage( unsigned char* img, int width, int height, bool blur = false);
    
    //! creates an image from warp
    /*!
     Instantiates this instance containing the contents of the bounding-box
     in the given image instance warped by the given matrix.
     \param image image to create warp from
     \param bb bounding-box [x, y, width, height]
     \param m 2x2 transformation matrix (translation is added inside)
    */
    void createWarp(TldImage *image, double *bb, float *m);
    
    
    //! returns the width of the image
    int getWidth(void);
    
    //! returns the height of the iamge
    int getHeight(void);
    
    //! returns the image data
    unsigned char *getData(void);
    
    //! Destructor
    ~TldImage(void);
    
    
    // Protected =============================================================
protected:
    //! sets the image data to the data, notice that the array isn't copied!
    void setData(unsigned char *d);
    
    
};
