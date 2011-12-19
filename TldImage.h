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
