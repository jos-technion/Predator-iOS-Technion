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


#include "PointTest.h"

PointTest::PointTest(float x0, float y0, float x1, float y1) {
    this->x0 = x0;
    this->x1 = x1;
    this->y0 = y0;
    this->y1 = y1;

}
int PointTest::test(TldImage *img, int patchX, int patchY, int patchW, int patchH) {
    int featureX0 = (x0*patchW) + patchX;
    int featureY0 = (y0 *patchH) + patchY;
    int featureX1 = (x1*patchW) + patchX;
    int featureY1 = (y1 *patchH) + patchY;
    
    //printf("Feature: (%d,%d,%d,%d)\n",featureX0,featureY0,featureX1,featureY1);
    unsigned char p0 = (img->getData())[featureY0 * (img->getWidth()) + featureX0];
    unsigned char p1 = (img->getData())[featureY1 * (img->getWidth()) + featureX1];
   // printf("Value: %d - %d\n",p0,p1);
    return p0 > p1 ? 1 : 0;
}