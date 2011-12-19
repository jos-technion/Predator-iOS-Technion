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


#ifndef __TLD__
#define __TLD__
#include "Utils.h"







class Tld {
public:
    //! constructor
    /*!
     \param width frame width
     \param height frame height
     \param frame the frame
     \param bb the bbox of the image
     \param nTrees number of trees
     \param nFEAT number of the features
     \param MIN_FEATURES_SCALE the min features scale
     \param MAX_FEATURES_SCALE  the maximum features scale
     \param WIDTH_STEPS width step
     \param HEIGHT_STEPS height step
     */
    Tld(int width, int height, unsigned char* frame, double* bb, int nTREES, int nFEAT, double MIN_FEATURE_SCALE, double MAX_FEATURE_SCALE, int WIDTH_STEPS, int HEIGHT_STEPS, bool is_video = false , bool blur = false) ;
    
    
    //!
    /*!
     \param width frame width
     \param height frame height
     \param NewImage the new image
     \param ttbb the traking bbox of the image
     \param MIN_TRACKING_CONF the min tracking confidence
     \param MIN_REINIT_CONF minumum reinit confidence
     \param  MIN_LEARNING_CONF minimum learning confidence
     \param blur if to to do blur for the image
     */
    void tldProcessFrame(int width, int height, unsigned char* NewImage,double * ttbb,double * outPut, double MIN_TRACKING_CONF, double MIN_REINIT_CONF, double MIN_LEARNING_CONF, bool blur = false, bool learning = true);
};

#endif