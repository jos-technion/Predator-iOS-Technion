

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
