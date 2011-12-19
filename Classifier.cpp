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


#include "Classifier.h"


Classifier::Classifier(int fernNum, int nodeNum) {
    // Initialise the ferns
    ferns = new Fern*[fernNum];
    fernCount = fernNum;

    for (int i = 0; i < fernCount; i++) {
        ferns[i] = new Fern(nodeNum);
    }
}


void Classifier::train(TldImage *image,  int patchX, int patchY, int patchW, int patchH, int patchClass) {
    // Train all the ferns
    if(patchClass == 0)
        return;
    for (int i = 0; i < fernCount; i++) {
        ferns[i]->train(image, patchX, patchY, patchW, patchH, patchClass);
    }
}


float Classifier::classify(TldImage *image, int patchX, int patchY, int patchW, int patchH) {
    // Calcualte the average fern posterior likelihood
    float sum = 0.0f;
    
    for (int i = 0; i < fernCount; i++) {
        sum += ferns[i]->classify(image, patchX, patchY, patchW, patchH);
    }
    
    return sum / fernCount;
}


Classifier::~Classifier() {
    for (int i = 0; i < fernCount; i++) {
        delete ferns[i];
    }
    
    delete [] ferns;
}
