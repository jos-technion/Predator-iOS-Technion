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

#include "Fern.h"
#define POWER 1


Fern::Fern(int nodeNum) {
    nodeCount = nodeNum;
    int leafNodes = (int)pow(2.0f * (float)POWER, nodeCount);

    p = new int[leafNodes];

    n = new int[leafNodes];

    posteriors = new float[leafNodes];

    nodes = new PointTest*[nodeCount];

    float SHF = 1/30.0;
    float OFF = SHF;

    for (int i = 0; i < nodeCount; i++) {
       
        float x0 = (rand()%29) * SHF + OFF/2;
        float y0 = (rand()%29) * SHF + OFF/2;
        
        float addition;
        int op = rand() %4;
            addition = rand()/(float)RAND_MAX + OFF;
        while(!addition) 
            addition = rand()/(float)RAND_MAX + OFF;
        float x1 = x0;
        float y1 = y0;
        switch (op) {
            case 0: // we go right
                x1+= addition;
                break;
            case 1: // we go left
                x1 -= addition;
                break;
            case 2: // we go to the top
                y1 += addition;
                break;
            case 3: // we go to the buttom
                y1 -= addition;
                break;
            }
        y1 = (y1 < 0 ? 0 : y1);
        y1 = (y1 > 1 ? 1 : y1);
        x1 = (x1 < 0 ? 0 : x1);
        x1 = (x1 > 1 ? 1 : x1);

        printf("Feature: (%lf,%lf,%lf,%lf)\n",x0,y0,x1,y1);
        nodes[i] = new PointTest(x0,y0,x1,y1);
    }
    
    // Initialise p, n, and posteriors
    for (int i = 0; i < leafNodes; i++) {
        p[i] = n[i] = 0;
        posteriors[i] = 0.0f;
    }
}


int Fern::getLeafIndex(TldImage *image, int patchX, int patchY, int patchW, int patchH) {
    // Limit patch to image area
    int width = image->getWidth();
    int height = image->getHeight();
    
    // Clamp x and y values between 0 and width and height respectively
    patchX = std::max(std::min(patchX, width - 2), 0);
    patchY = std::max(std::min(patchY, height - 2), 0);
    
    // Limit width and height values to (width - patchX) and (height - patchY)
    // respectively
    patchW = std::min(patchW, width - patchX);
    patchH = std::min(patchH, height - patchY);
    
    // Apply all tests to find the leaf index this patch falls into
    int leaf = 0;
    
    for (int i = 0; i < nodeCount; i++) {
        leaf = leaf | (nodes[i]->test(image, patchX, patchY, patchW, patchH) << i * (int)POWER);
    }
    
    return leaf;
}

void Fern::train(TldImage *image, int patchX, int patchY, int patchW, int patchH, int patchClass) {
    // Apply all tests to find the leaf index this patch falls into
    int leaf = getLeafIndex(image, patchX, patchY, patchW, patchH);
    
    // Increment the number of positive or negative patches that fell into
    // this leaf
    if (patchClass == 0) {
        n[leaf]++;
    }
    else {
        p[leaf]++;
    }
    
    // Compute the posterior likelihood of a positive class for this leaf
    if (p[leaf] > 0) {
        posteriors[leaf] = (float)p[leaf] / (float)(p[leaf] + n[leaf]);
    }
}


float Fern::classify(TldImage *image, int patchX, int patchY, int patchW, int patchH) {
    // Return the precomputed posterior likelihood of a positive class for
    // this leaf
    return posteriors[getLeafIndex(image, patchX, patchY, patchW, patchH)];
}


Fern::~Fern() {
    for (int i = 0; i < nodeCount; i++) {
        delete nodes[i];
    }

    delete [] nodes;
    delete [] p;
    delete [] n;
    delete [] posteriors;
}
