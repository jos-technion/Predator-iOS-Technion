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
#include "Fern.h"
#include "TldImage.h"
#include <iostream>
using namespace std;

class Classifier {
    public:
    //!  Constructor.
    /*!
        \param fernNum the number of ferns to create
        \param nodeNum number of nodes to create in each fern
    */
    Classifier(int fernNum, int nodeNum);
    
    //!  Trains all ferns in the forest with a single training patch.
    /*!
      \param image: image to take the training patch from
       \param  patchX patch top-left x-position
       \param  patchY patch top-left y-position
       \param  patchW patch width
       \param  patchH patch height
       \param  patchClass 0 if the patch is negative, 1 if the patch is positive
     */
    void train(TldImage *image, int patchX, int patchY, int patchW, int patchH, int patchClass);
    
    //!  Classifies a given patch.
    /*!
        
        \param  image image to take the test patch from
        \param   patchX patch top-left x-position
        \param   patchY patch top-left y-position
        \param   patchW patch width
        \param   patchH patch height 
         \return the posterior likelihood that the patch is positive.
    */
    float classify(TldImage *image,  int patchX, int patchY, int patchW, int patchH);
    
    //!  Destructor. 
    ~Classifier(void);
    
    private:
    //! Array of ferns
    Fern **ferns;
    
    //! Number of ferns
    int fernCount;

};
