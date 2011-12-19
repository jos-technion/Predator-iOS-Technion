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