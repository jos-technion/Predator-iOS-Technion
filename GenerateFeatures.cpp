//
//  GenerateFeatures.cpp
//  Predator
//
//  Created by admin on 11/3/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "GenerateFeatures.h"
#include "Utils.h"
#define BIG_NUMBER 1000000


int
tldGenerateFeatures(float features[][4], int number_of_features) {
    double SHI = 1/5.0;
    double OFF = SHI;
    int i,j,k;
    double **matrix_tuples,**rep_matrix;
    int size_tuples,size_rep;
    double *x;
    int array_size= create_array(&x,0,1,SHI);
    size_tuples= ntuples(&matrix_tuples,x,array_size,x,array_size);
    free(x);
    
    /*
     matrix_tuples : matrix 2xarray_size
     _________________________________________
     |__|__|__|__|__|__|__|__|__|__|__|__|__|__|
     |__|__|__|__|__|__|__|__|__|__|__|__|__|__|
     
     */ 
    size_rep= repmatrix(&rep_matrix ,matrix_tuples ,size_tuples, 2 ,4,2);    
    
    /*
     repmatrix : matrix 4 x size_rep
     _________________________________________
     |__|__|__|__|__|__|__|__|__|__|__|__|__|__|
     |__|__|__|__|__|__|__|__|__|__|__|__|__|__|
     |__|__|__|__|__|__|__|__|__|__|__|__|__|__|
     |__|__|__|__|__|__|__|__|__|__|__|__|__|__|
     
     basically the width of this matrix is 4*size_tuples, so we divide it to 4 sections,
     each section we do a different thing:
     1st: we go right add addition to the 3rd row.
     2nd: we go left, substract addition from the 3rd row.
     3rd: we go up (top), substract addition from 4th row.
     4th: we go down (buttom) add addition to the 4th row.
     */ 
    
    
    /* free the unused array */
    free_matrix(matrix_tuples,size_tuples);
    free(matrix_tuples);
    
    
    srand(0);
    float addition;
    for(i=0; i < 4*size_tuples; i++) {
        int op = rand() %4;
        addition = rand()/(float)RAND_MAX + OFF;
        while(!addition) 
            addition = rand()/(float)RAND_MAX + OFF;
        switch (op) {
            case 0: // we go right
                rep_matrix[i][3] += addition;
                break;
            case 1: // we go left
                rep_matrix[i][3] -= addition;
                break;
            case 2: // we go to the top
                rep_matrix[i][4] += addition;
                break;
            case 3: // we go to the buttom
                rep_matrix[i][4] -= addition;
                break;
        }
    }
    
    /* Now we filter the candidate features, to leave only features within 0 and 1.  */
    k = 0; // current inspected feature
    for(i=0; i < number_of_features; i++) {
        if(rep_matrix[k][0] >0 && rep_matrix[k][1] < 1) { // a good feature, that we can take
            features[i][0] = rep_matrix[k][0];
            features[i][1] = rep_matrix[k][1];
            features[i][2] = rep_matrix[k][2];
            features[i][3] = rep_matrix[k][3];
           // assert(features[i][0] >=0);
            //assert(features[i][0] <=1);
            if(features[i][0] > 1) { // if we went too much right, then we fix it to 1
                features[i][0] = 1;
            }
            if(features[i][0] < 0) { // if we went too much left, then we fix it to 0
                features[i][0] = 0;
            }
            if(features[i][1] > 1) { // if we went too much down, then we fix it to 1
                features[i][1] = 1;
            }
            if(features[i][1] < 0) { // if we went too much up, then we fix it to 0
                features[i][1] = 0;
            }
            if(features[i][2] > 1) { // if we went too much right, then we fix it to 1
                features[i][2] = 1;
            }
            if(features[i][2] < 0) { // if we went too much left, then we fix it to 0
                features[i][2] = 0;
            }
            if(features[i][3] > 1) { // if we went too much down, then we fix it to 1
                features[i][3] = 1;
            }
            if(features[i][3] < 0) { // if we went too much up, then we fix it to 0
                features[i][3] = 0;
            }


        } else {
            i--;
        }
        k++; // go to next feature
    }
   // free_matrix(rep_matrix,size_tuples*4);
    free(rep_matrix);
    //free(x);
	
    return 0;
    
}