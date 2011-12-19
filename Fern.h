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
#include "TldImage.h"

#include "PointTest.h"
#include <algorithm>
#include <math.h>


/*  Implementation of a random fern. */
class Fern {
    // Private ===============================================================
    private:
    //! The 2^nFEAT array
    PointTest **nodes;
    //! Number of nodes
    int nodeCount;
    
    //! Array containing the number of positive patches that fell into each
    //! leaf node
    int *p;
    
    //! Array containing the number of negative patches that fell into each
    //! leaf node
    int *n;
    
    //! Array containing the precomputed posterior likelihoods that each leaf
    //! node is positive.
    float *posteriors;
    
    //! Computes the index of the leaf node a patch falls into.
    /*!
        \param image image to take patch from
        \param patchX patch top-left x-position
        \param patchY patch top-left y-position
        \param patchW patch width
        \param patchH patch height
        \return the index.
     */
    int getLeafIndex(TldImage *image, int patchX, int patchY, int patchW, int patchH);
    
    
    // Public ================================================================
    public:
    /*  Constructor.
        nodeNum: number of nodes to create
    */
    Fern(int nodeNum);
    
   //! Trains this fern with a single training patch.
     /*!
        \param image image to take the training patch from
        \param patchX patch top-left x-position
        \param patchX patch top-left y-position
        \param patchW patch width
        \param patchH patch height
        \param patchClass 0 if the patch is negative, 1 if the patch is positive
     */
    void train(TldImage *image, int patchX, int patchY, int patchW, int patchH, int patchClass);
    
   //! Classifies a given patch.
     /*!
        Returns the posterior liklihood that the patch is positive.
        image: image to take the test patch from
        \param patchX patch top-left x-position
        \param patchX patch top-left y-position
        \param patchW patch width
        \param patchH patch height
      */
    float classify(TldImage *image, int patchX, int patchY, int patchW, int patchH);
    
    /*  Destructor. */
    ~Fern(void);
};
