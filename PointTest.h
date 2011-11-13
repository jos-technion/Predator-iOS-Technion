//
//  PointTest.h
//  Predator
//
//  Created by admin on 11/3/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//
#include "TldImage.h"


class PointTest {
public:
    //!constructor
    /*!
     \param x0 top-left x-position
     \param y0 top-left y-position
     \param x1 bottom-right x-position
     \param y1 bottom-right y-position
     */
    PointTest(float x0, float y0, float x1, float y1);
    
    
    //! test the feature on the supplied patch
    /*!
     \param img image to take patch from
     \param patchX patch top-left x-position
     \param patchY patch top-left y-position
     \param patchW patch width
     \param patchH patch height
     \return the value on the supplied patch
     */
    int test(TldImage *img, int patchX, int patchY, int patchW, int patchH);
    
private:
    //! top-left x-position
    float x0;
    
    //! top-left y-position
    float y0;
    
    //! bottom-right x-position
    float x1;
    
    //! bottom-right y-position
    float y1;
};