

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
