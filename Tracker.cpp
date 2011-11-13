

#include "Tracker.h"

#include <opencv2/legacy/legacy.hpp>

#include <opencv2/core/internal.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/video/background_segm.hpp>

#include <opencv2/legacy/blobtrack.hpp>
#include <opencv2/legacy/compat.hpp>

//#include <_matrix.h>

typedef unsigned short ushort;

CV_INLINE bool operator == (CvSize size1, CvSize size2 );
CV_INLINE bool operator == (CvSize size1, CvSize size2 )
{
    return size1.width == size2.width && size1.height == size2.height;
}

CV_INLINE bool operator != (CvSize size1, CvSize size2 );
CV_INLINE bool operator != (CvSize size1, CvSize size2 )
{
    return size1.width != size2.width || size1.height != size2.height;
}



namespace cv {
#undef CV_8TO32F
#define CV_8TO32F(a) (a)
    static const void*
    icvAdjustRect( const void* srcptr, int src_step, int pix_size,
                  CvSize src_size, CvSize win_size,
                  CvPoint ip, CvRect* pRect )
    {
        CvRect rect;
        const char* src = (const char*)srcptr;
        
        if( ip.x >= 0 )
        {
            src += ip.x*pix_size;
            rect.x = 0;
        }
        else
        {
            rect.x = -ip.x;
            if( rect.x > win_size.width )
                rect.x = win_size.width;
        }
        
        if( ip.x + win_size.width < src_size.width )
            rect.width = win_size.width;
        else
        {
            rect.width = src_size.width - ip.x - 1;
            if( rect.width < 0 )
            {
                src += rect.width*pix_size;
                rect.width = 0;
            }
            assert( rect.width <= win_size.width );
        }
        
        if( ip.y >= 0 )
        {
            src += ip.y * src_step;
            rect.y = 0;
        }
        else
            rect.y = -ip.y;
        
        if( ip.y + win_size.height < src_size.height )
            rect.height = win_size.height;
        else
        {
            rect.height = src_size.height - ip.y - 1;
            if( rect.height < 0 )
            {
                src += rect.height*src_step;
                rect.height = 0;
            }
        }
        
        *pRect = rect;
        return src - rect.x*pix_size;
    }
    

    static void
    icvCalcIxIy_32f( const float* src, int src_step, float* dstX, float* dstY, int dst_step,
                    CvSize src_size, const float* smooth_k, float* buffer0 )
    {
        int src_width = src_size.width, dst_width = src_size.width-2;
        int x, height = src_size.height - 2;
        float* buffer1 = buffer0 + src_width;
        
        src_step /= sizeof(src[0]);
        dst_step /= sizeof(dstX[0]);
        
        for( ; height--; src += src_step, dstX += dst_step, dstY += dst_step )
        {
            const float* src2 = src + src_step;
            const float* src3 = src + src_step*2;
            
            for( x = 0; x < src_width; x++ )
            {
                float t0 = (src3[x] + src[x])*smooth_k[0] + src2[x]*smooth_k[1];
                float t1 = src3[x] - src[x];
                buffer0[x] = t0; buffer1[x] = t1;
            }
            
            for( x = 0; x < dst_width; x++ )
            {
                float t0 = buffer0[x+2] - buffer0[x];
                float t1 = (buffer1[x] + buffer1[x+2])*smooth_k[0] + buffer1[x+1]*smooth_k[1];
                dstX[x] = t0; dstY[x] = t1;
            }
        }
    }
    
    static void
    intersect( CvPoint2D32f pt, CvSize win_size, CvSize imgSize,
              CvPoint* min_pt, CvPoint* max_pt )
    {
        CvPoint ipt;
        
        ipt.x = cvFloor( pt.x );
        ipt.y = cvFloor( pt.y );
        
        ipt.x -= win_size.width;
        ipt.y -= win_size.height;
        
        win_size.width = win_size.width * 2 + 1;
        win_size.height = win_size.height * 2 + 1;
        
        min_pt->x = MAX( 0, -ipt.x );
        min_pt->y = MAX( 0, -ipt.y );
        max_pt->x = MIN( win_size.width, imgSize.width - ipt.x );
        max_pt->y = MIN( win_size.height, imgSize.height - ipt.y );
    }
    static int icvMinimalPyramidSize( CvSize imgSize )
    {
        return cvAlign(imgSize.width,8) * imgSize.height / 3;
    }
    
    
    static CvStatus CV_STDCALL icvGetRectSubPix_8u32f_C1R
    ( const uchar* src, int src_step, CvSize src_size,
     float* dst, int dst_step, CvSize win_size, CvPoint2D32f center )
    {
        CvPoint ip;
        float  a12, a22, b1, b2;
        float a, b;
        double s = 0;
        int i, j;
        
        center.x -= (win_size.width-1)*0.5f;
        center.y -= (win_size.height-1)*0.5f;
        
        ip.x = cvFloor( center.x );
        ip.y = cvFloor( center.y );
        
        if( win_size.width <= 0 || win_size.height <= 0 )
            return CV_BADRANGE_ERR;
        
        a = center.x - ip.x;
        b = center.y - ip.y;
        a = MAX(a,0.0001f);
        a12 = a*(1.f-b);
        a22 = a*b;
        b1 = 1.f - b;
        b2 = b;
        s = (1. - a)/a;
        
        src_step /= sizeof(src[0]);
        dst_step /= sizeof(dst[0]);
        
        if( 0 <= ip.x && ip.x + win_size.width < src_size.width &&
           0 <= ip.y && ip.y + win_size.height < src_size.height )
        {
            // extracted rectangle is totally inside the image
            src += ip.y * src_step + ip.x;
            
#if 0
            if( icvCopySubpix_8u32f_C1R_p &&
               icvCopySubpix_8u32f_C1R_p( src, src_step, dst,
                                         dst_step*sizeof(dst[0]), win_size, a, b ) >= 0 )
                return CV_OK;
#endif
            
            for( ; win_size.height--; src += src_step, dst += dst_step )
            {
                float prev = (1 - a)*(b1*CV_8TO32F(src[0]) + b2*CV_8TO32F(src[src_step]));
                for( j = 0; j < win_size.width; j++ )
                {
                    float t = a12*CV_8TO32F(src[j+1]) + a22*CV_8TO32F(src[j+1+src_step]);
                    dst[j] = prev + t;
                    prev = (float)(t*s);
                }
            }
        }
        else
        {
            CvRect r;
            
            src = (const uchar*)icvAdjustRect( src, src_step*sizeof(*src),
                                              sizeof(*src), src_size, win_size,ip, &r);
            
            for( i = 0; i < win_size.height; i++, dst += dst_step )
            {
                const uchar *src2 = src + src_step;
                
                if( i < r.y || i >= r.height )
                    src2 -= src_step;
                
                for( j = 0; j < r.x; j++ )
                {
                    float s0 = CV_8TO32F(src[r.x])*b1 +
                    CV_8TO32F(src2[r.x])*b2;
                    
                    dst[j] = (float)(s0);
                }
                
                if( j < r.width )
                {
                    float prev = (1 - a)*(b1*CV_8TO32F(src[j]) + b2*CV_8TO32F(src2[j]));
                    
                    for( ; j < r.width; j++ )
                    {
                        float t = a12*CV_8TO32F(src[j+1]) + a22*CV_8TO32F(src2[j+1]);
                        dst[j] = prev + t;
                        prev = (float)(t*s);
                    }
                }
                
                for( ; j < win_size.width; j++ )
                {
                    float s0 = CV_8TO32F(src[r.width])*b1 +
                    CV_8TO32F(src2[r.width])*b2;
                    
                    dst[j] = (float)(s0);
                }
                
                if( i < r.height )
                    src = src2;
            }
        }
        
        return CV_OK;
    }
    

    
    
    static void
    icvInitPyramidalAlgorithm( const CvMat* imgA, const CvMat* imgB,
                              CvMat* pyrA, CvMat* pyrB,
                              int level, CvTermCriteria * criteria,
                              int max_iters, int flags,
                              uchar *** imgI, uchar *** imgJ,
                              int **step, CvSize** size,
                              double **scale, cv::AutoBuffer<uchar>* buffer )
    {
        const int ALIGN = 8;
        int pyrBytes, bufferBytes = 0, elem_size;
        int level1 = level + 1;
        
        int i;
        CvSize imgSize, levelSize;
        
        *imgI = *imgJ = 0;
        *step = 0;
        *scale = 0;
        *size = 0;
        
        /* check input arguments */
        if( ((flags & CV_LKFLOW_PYR_A_READY) != 0 && !pyrA) ||
           ((flags & CV_LKFLOW_PYR_B_READY) != 0 && !pyrB) )
            CV_Error( CV_StsNullPtr, "Some of the precomputed pyramids are missing" );
        
        if( level < 0 )
            CV_Error( CV_StsOutOfRange, "The number of pyramid levels is negative" );
        
        switch( criteria->type )
        {
            case CV_TERMCRIT_ITER:
                criteria->epsilon = 0.f;
                break;
            case CV_TERMCRIT_EPS:
                criteria->max_iter = max_iters;
                break;
            case CV_TERMCRIT_ITER | CV_TERMCRIT_EPS:
                break;
            default:
                assert( 0 );
                CV_Error( CV_StsBadArg, "Invalid termination criteria" );
        }
        
        /* compare squared values */
        criteria->epsilon *= criteria->epsilon;
        
        /* set pointers and step for every level */
        pyrBytes = 0;
        
        imgSize = cvGetSize(imgA);
        elem_size = CV_ELEM_SIZE(imgA->type);
        levelSize = imgSize;
        
        for( i = 1; i < level1; i++ )
        {
            levelSize.width = (levelSize.width + 1) >> 1;
            levelSize.height = (levelSize.height + 1) >> 1;
            
            int tstep = cvAlign(levelSize.width,ALIGN) * elem_size;
            pyrBytes += tstep * levelSize.height;
        }
        
        assert( pyrBytes <= imgSize.width * imgSize.height * elem_size * 4 / 3 );
        
        /* buffer_size = <size for patches> + <size for pyramids> */
        bufferBytes = (int)((level1 >= 0) * ((pyrA->data.ptr == 0) +
                                             (pyrB->data.ptr == 0)) * pyrBytes +
                            (sizeof(imgI[0][0]) * 2 + sizeof(step[0][0]) +
                             sizeof(size[0][0]) + sizeof(scale[0][0])) * level1);
        
        buffer->allocate( bufferBytes );
        
        *imgI = (uchar **) (uchar*)(*buffer);
        *imgJ = *imgI + level1;
        *step = (int *) (*imgJ + level1);
        *scale = (double *) (*step + level1);
        *size = (CvSize *)(*scale + level1);
        
        imgI[0][0] = imgA->data.ptr;
        imgJ[0][0] = imgB->data.ptr;
        step[0][0] = imgA->step;
        scale[0][0] = 1;
        size[0][0] = imgSize;
        
        if( level > 0 )
        {
            uchar *bufPtr = (uchar *) (*size + level1);
            uchar *ptrA = pyrA->data.ptr;
            uchar *ptrB = pyrB->data.ptr;
            
            if( !ptrA )
            {
                ptrA = bufPtr;
                bufPtr += pyrBytes;
            }
            
            if( !ptrB )
                ptrB = bufPtr;
            
            levelSize = imgSize;
            
            /* build pyramids for both frames */
            for( i = 1; i <= level; i++ )
            {
                int levelBytes;
                CvMat prev_level, next_level;
                
                levelSize.width = (levelSize.width + 1) >> 1;
                levelSize.height = (levelSize.height + 1) >> 1;
                
                size[0][i] = levelSize;
                step[0][i] = cvAlign( levelSize.width, ALIGN ) * elem_size;
                scale[0][i] = scale[0][i - 1] * 0.5;
                
                levelBytes = step[0][i] * levelSize.height;
                imgI[0][i] = (uchar *) ptrA;
                ptrA += levelBytes;
                
                if( !(flags & CV_LKFLOW_PYR_A_READY) )
                {
                    prev_level = cvMat( size[0][i-1].height, size[0][i-1].width, CV_8UC1 );
                    next_level = cvMat( size[0][i].height, size[0][i].width, CV_8UC1 );
                    cvSetData( &prev_level, imgI[0][i-1], step[0][i-1] );
                    cvSetData( &next_level, imgI[0][i], step[0][i] );
                    cvPyrDown( &prev_level, &next_level );
                }
                
                imgJ[0][i] = (uchar *) ptrB;
                ptrB += levelBytes;
                
                if( !(flags & CV_LKFLOW_PYR_B_READY) )
                {
                    prev_level = cvMat( size[0][i-1].height, size[0][i-1].width, CV_8UC1 );
                    next_level = cvMat( size[0][i].height, size[0][i].width, CV_8UC1 );
                    cvSetData( &prev_level, imgJ[0][i-1], step[0][i-1] );
                    cvSetData( &next_level, imgJ[0][i], step[0][i] );
                    cvPyrDown( &prev_level, &next_level );
                }
            }
        }
    }

    struct LKTrackerInvoker
    {
        LKTrackerInvoker( const CvMat* _imgI, const CvMat* _imgJ,
                         const CvPoint2D32f* _featuresA,
                         CvPoint2D32f* _featuresB,
                         char* _status, float* _error,
                         CvTermCriteria _criteria,
                         CvSize _winSize, int _level, int _flags )
        {
            imgI = _imgI;
            imgJ = _imgJ;
            featuresA = _featuresA;
            featuresB = _featuresB;
            status = _status;
            error = _error;
            criteria = _criteria;
            winSize = _winSize;
            level = _level;
            flags = _flags;
        }
        
        void operator()(const BlockedRange& range) const
        {
            static const float smoothKernel[] = { 0.09375, 0.3125, 0.09375 };  // 3/32, 10/32, 3/32
            
            int i, i1 = range.begin(), i2 = range.end();
            
            CvSize patchSize = cvSize( winSize.width * 2 + 1, winSize.height * 2 + 1 );
            int patchLen = patchSize.width * patchSize.height;
            int srcPatchLen = (patchSize.width + 2)*(patchSize.height + 2);
            
            AutoBuffer<float> buf(patchLen*3 + srcPatchLen);
            float* patchI = buf;
            float* patchJ = patchI + srcPatchLen;
            float* Ix = patchJ + patchLen;
            float* Iy = Ix + patchLen;
            float scaleL = 1.f/(1 << level);
            CvSize levelSize = cvGetMatSize(imgI);
            
            // find flow for each given point
            for( i = i1; i < i2; i++ )
            {
                CvPoint2D32f v;
                CvPoint minI, maxI, minJ, maxJ;
                CvSize isz, jsz;
                int pt_status;
                CvPoint2D32f u;
                CvPoint prev_minJ = { -1, -1 }, prev_maxJ = { -1, -1 };
                double Gxx = 0, Gxy = 0, Gyy = 0, D = 0, minEig = 0;
                float prev_mx = 0, prev_my = 0;
                int j, x, y;
                
                v.x = featuresB[i].x*2;
                v.y = featuresB[i].y*2;
                
                pt_status = status[i];
                if( !pt_status )
                    continue;
                
                minI = maxI = minJ = maxJ = cvPoint(0, 0);
                
                u.x = featuresA[i].x * scaleL;
                u.y = featuresA[i].y * scaleL;
                
                intersect( u, winSize, levelSize, &minI, &maxI );
                isz = jsz = cvSize(maxI.x - minI.x + 2, maxI.y - minI.y + 2);
                u.x += (minI.x - (patchSize.width - maxI.x + 1))*0.5f;
                u.y += (minI.y - (patchSize.height - maxI.y + 1))*0.5f;
                
                if( isz.width < 3 || isz.height < 3 ||
                   icvGetRectSubPix_8u32f_C1R( imgI->data.ptr, imgI->step, levelSize,
                                              patchI, isz.width*sizeof(patchI[0]), isz, u ) < 0 )
                {
                    // point is outside the first image. take the next
                    status[i] = 0;
                    continue;
                }
                
                icvCalcIxIy_32f( patchI, isz.width*sizeof(patchI[0]), Ix, Iy,
                                (isz.width-2)*sizeof(patchI[0]), isz, smoothKernel, patchJ );
                
                for( j = 0; j < criteria.max_iter; j++ )
                {
                    double bx = 0, by = 0;
                    float mx, my;
                    CvPoint2D32f _v;
                    
                    intersect( v, winSize, levelSize, &minJ, &maxJ );
                    
                    minJ.x = MAX( minJ.x, minI.x );
                    minJ.y = MAX( minJ.y, minI.y );
                    
                    maxJ.x = MIN( maxJ.x, maxI.x );
                    maxJ.y = MIN( maxJ.y, maxI.y );
                    
                    jsz = cvSize(maxJ.x - minJ.x, maxJ.y - minJ.y);
                    
                    _v.x = v.x + (minJ.x - (patchSize.width - maxJ.x + 1))*0.5f;
                    _v.y = v.y + (minJ.y - (patchSize.height - maxJ.y + 1))*0.5f;
                    
                    if( jsz.width < 1 || jsz.height < 1 ||
                       icvGetRectSubPix_8u32f_C1R( imgJ->data.ptr, imgJ->step, levelSize, patchJ,
                                                  jsz.width*sizeof(patchJ[0]), jsz, _v ) < 0 )
                    {
                        // point is outside of the second image. take the next
                        pt_status = 0;
                        break;
                    }
                    
                    if( maxJ.x == prev_maxJ.x && maxJ.y == prev_maxJ.y &&
                       minJ.x == prev_minJ.x && minJ.y == prev_minJ.y )
                    {
                        for( y = 0; y < jsz.height; y++ )
                        {
                            const float* pi = patchI +
                            (y + minJ.y - minI.y + 1)*isz.width + minJ.x - minI.x + 1;
                            const float* pj = patchJ + y*jsz.width;
                            const float* ix = Ix +
                            (y + minJ.y - minI.y)*(isz.width-2) + minJ.x - minI.x;
                            const float* iy = Iy + (ix - Ix);
                            
                            for( x = 0; x < jsz.width; x++ )
                            {
                                double t0 = pi[x] - pj[x];
                                bx += t0 * ix[x];
                                by += t0 * iy[x];
                            }
                        }
                    }
                    else
                    {
                        Gxx = Gyy = Gxy = 0;
                        for( y = 0; y < jsz.height; y++ )
                        {
                            const float* pi = patchI +
                            (y + minJ.y - minI.y + 1)*isz.width + minJ.x - minI.x + 1;
                            const float* pj = patchJ + y*jsz.width;
                            const float* ix = Ix +
                            (y + minJ.y - minI.y)*(isz.width-2) + minJ.x - minI.x;
                            const float* iy = Iy + (ix - Ix);
                            
                            for( x = 0; x < jsz.width; x++ )
                            {
                                double t = pi[x] - pj[x];
                                bx += (double) (t * ix[x]);
                                by += (double) (t * iy[x]);
                                Gxx += ix[x] * ix[x];
                                Gxy += ix[x] * iy[x];
                                Gyy += iy[x] * iy[x];
                            }
                        }
                        
                        D = Gxx * Gyy - Gxy * Gxy;
                        if( D < DBL_EPSILON )
                        {
                            pt_status = 0;
                            break;
                        }
                        
                        // Adi Shavit - 2008.05
                        if( flags & CV_LKFLOW_GET_MIN_EIGENVALS )
                            minEig = (Gyy + Gxx - sqrt((Gxx-Gyy)*(Gxx-Gyy) + 4.*Gxy*Gxy))/(2*jsz.height*jsz.width);
                        
                        D = 1. / D;
                        
                        prev_minJ = minJ;
                        prev_maxJ = maxJ;
                    }
                    
                    mx = (float) ((Gyy * bx - Gxy * by) * D);
                    my = (float) ((Gxx * by - Gxy * bx) * D);
                    
                    v.x += mx;
                    v.y += my;
                    
                    if( mx * mx + my * my < criteria.epsilon )
                        break;
                    
                    if( j > 0 && fabs(mx + prev_mx) < 0.01 && fabs(my + prev_my) < 0.01 )
                    {
                        v.x -= mx*0.5f;
                        v.y -= my*0.5f;
                        break;
                    }
                    prev_mx = mx;
                    prev_my = my;
                }
                
                featuresB[i] = v;
                status[i] = (char)pt_status;
                if( level == 0 && error && pt_status )
                {
                    // calc error
                    double err = 0;
                    if( flags & CV_LKFLOW_GET_MIN_EIGENVALS )
                        err = minEig;
                    else
                    {
                        for( y = 0; y < jsz.height; y++ )
                        {
                            const float* pi = patchI +
                            (y + minJ.y - minI.y + 1)*isz.width + minJ.x - minI.x + 1;
                            const float* pj = patchJ + y*jsz.width;
                            
                            for( x = 0; x < jsz.width; x++ )
                            {
                                double t = pi[x] - pj[x];
                                err += t * t;
                            }
                        }
                        err = sqrt(err);
                    }
                    error[i] = (float)err;
                }
            } // end of point processing loop (i)
        }
        
        const CvMat* imgI;
        const CvMat* imgJ;
        const CvPoint2D32f* featuresA;
        CvPoint2D32f* featuresB;
        char* status;
        float* error;
        CvTermCriteria criteria;
        CvSize winSize;
        int level;
        int flags;
    };
    
    
    void
    majd( const void* arrA, const void* arrB,
                           void* pyrarrA, void* pyrarrB,
                           const CvPoint2D32f * featuresA,
                           CvPoint2D32f * featuresB,
                           int count, CvSize winSize, int level,
                           char *status, float *error,
                           CvTermCriteria criteria, int flags )
    {
        cv::AutoBuffer<uchar> pyrBuffer;
        cv::AutoBuffer<uchar> buffer;
        cv::AutoBuffer<char> _status;
        
        const int MAX_ITERS = 100;
        
        CvMat stubA, *imgA = (CvMat*)arrA;
        CvMat stubB, *imgB = (CvMat*)arrB;
        CvMat pstubA, *pyrA = (CvMat*)pyrarrA;
        CvMat pstubB, *pyrB = (CvMat*)pyrarrB;
        CvSize imgSize;
        
        uchar **imgI = 0;
        uchar **imgJ = 0;
        int *step = 0;
        double *scale = 0;
        CvSize* size = 0;
        
        int i, l;
        
        imgA = cvGetMat( imgA, &stubA );
        imgB = cvGetMat( imgB, &stubB );
        
        if( CV_MAT_TYPE( imgA->type ) != CV_8UC1 )
            CV_Error( CV_StsUnsupportedFormat, "" );
        
        if( !CV_ARE_TYPES_EQ( imgA, imgB ))
            CV_Error( CV_StsUnmatchedFormats, "" );
        
        if( !CV_ARE_SIZES_EQ( imgA, imgB ))
            CV_Error( CV_StsUnmatchedSizes, "" );
        
        if( imgA->step != imgB->step )
            CV_Error( CV_StsUnmatchedSizes, "imgA and imgB must have equal steps" );
        
        imgSize = cvGetMatSize( imgA );
        
        if( pyrA )
        {
            pyrA = cvGetMat( pyrA, &pstubA );
            
            if( pyrA->step*pyrA->height < icvMinimalPyramidSize( imgSize ) )
                CV_Error( CV_StsBadArg, "pyramid A has insufficient size" );
        }
        else
        {
            pyrA = &pstubA;
            pyrA->data.ptr = 0;
        }
        
        if( pyrB )
        {
            pyrB = cvGetMat( pyrB, &pstubB );
            
            if( pyrB->step*pyrB->height < icvMinimalPyramidSize( imgSize ) )
                CV_Error( CV_StsBadArg, "pyramid B has insufficient size" );
        }
        else
        {
            pyrB = &pstubB;
            pyrB->data.ptr = 0;
        }
        
        if( count == 0 )
            return;
        
        if( !featuresA || !featuresB )
            CV_Error( CV_StsNullPtr, "Some of arrays of point coordinates are missing" );
        
        if( count < 0 )
            CV_Error( CV_StsOutOfRange, "The number of tracked points is negative or zero" );
        
        if( winSize.width <= 1 || winSize.height <= 1 )
            CV_Error( CV_StsBadSize, "Invalid search window size" );
        
        icvInitPyramidalAlgorithm( imgA, imgB, pyrA, pyrB,
                                  level, &criteria, MAX_ITERS, flags,
                                  &imgI, &imgJ, &step, &size, &scale, &pyrBuffer );
        
        if( !status )
        {
            _status.allocate(count);
            status = _status;
        }
        
        memset( status, 1, count );
        if( error )
            memset( error, 0, count*sizeof(error[0]) );
        
        if( !(flags & CV_LKFLOW_INITIAL_GUESSES) )
            memcpy( featuresB, featuresA, count*sizeof(featuresA[0]));
        
        for( i = 0; i < count; i++ )
        {
            featuresB[i].x = (float)(featuresB[i].x * scale[level] * 0.5);
            featuresB[i].y = (float)(featuresB[i].y * scale[level] * 0.5);
        }
        
        /* do processing from top pyramid level (smallest image)
         to the bottom (original image) */
        for( l = level; l >= 0; l-- )
        {
            CvMat imgI_l, imgJ_l;        
            cvInitMatHeader(&imgI_l, size[l].height, size[l].width, imgA->type, imgI[l], step[l]);
            cvInitMatHeader(&imgJ_l, size[l].height, size[l].width, imgB->type, imgJ[l], step[l]);
            
            cv::parallel_for(cv::BlockedRange(0, count),
                             cv::LKTrackerInvoker(&imgI_l, &imgJ_l, featuresA,
                                                  featuresB, status, error,
                                                  criteria, winSize, l, flags));
        } // end of pyramid levels loop (l)
    }

}

static void loadImageFromMatlab(unsigned char* values, IplImage *image) {
    
	int widthStep = image->widthStep;
	int N = 470; // width
	int M = 310; // height
    
	if (N == 0 || M == 0) {
		//printf("Input image error\n");
		return;
	}
    
	for(int i=0;i<N;i++)
		for(int j=0;j<M;j++) {
			image->imageData[j*widthStep+i] = values[j+i*M];
           
        }
  //  printf("\n");
}

Tracker::Tracker(int frameWidth, int frameHeight, CvSize *frameSize, IplImage *firstFrame, Classifier *classifier) {
    width = frameWidth;
    height = frameHeight;
    prevFrame = cvCloneImage(firstFrame);
    prevPyramid = cvCreateImage(*frameSize, IPL_DEPTH_8U, 1);
    nextPyramid = cvCreateImage(*frameSize, IPL_DEPTH_8U, 1);
    prevPoints = (CvPoint2D32f *)cvAlloc(TOTAL_POINTS * sizeof(CvPoint2D32f));
    nextPoints = (CvPoint2D32f *)cvAlloc(TOTAL_POINTS * sizeof(CvPoint2D32f));
    if(USE_BACKWARD_TRACKING) {
        predPoints = (CvPoint2D32f *)cvAlloc(TOTAL_POINTS * sizeof(CvPoint2D32f));
        predStatus = (char *)cvAlloc(TOTAL_POINTS);
    }
    windowSize = (CvSize *)malloc(sizeof(CvSize));
    *windowSize = cvSize(WINDOW_SIZE, WINDOW_SIZE);
    status = (char *)cvAlloc(TOTAL_POINTS);
    termCriteria = (TermCriteria *)malloc(sizeof(TermCriteria));
    *termCriteria = TermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03);
    this->classifier = classifier;
}


float Tracker::median(float *A, int length) {
    int index = (int)(length / 2);
    std::sort(A, A + length);
    
    if (length % 2 == 0) {
        return (A[index] + A[index + 1]) / 2;
    }
    else {
        return A[index];
    }
}


double *Tracker::trackWithBackward(IplImage *nextFrame, TldImage *nextFrameIntImg, double* bb) {
    
    // Perform Lucas-Kanade Tracking -----------------------------------------
    // Distribute points to track uniformly over the bounding-box
    double bbWidth = bb[2]-10;
    double bbHeight = bb[3]-10;
    double stepX = bbWidth / (DIM_POINTS - 1);
    double stepY = bbHeight / (DIM_POINTS - 1);
    int i, x, y;
    for (i = 0, x = 0; x < DIM_POINTS; x++) {
        for (y = 0; y < DIM_POINTS; y++, i++) {
            prevPoints[i].x = (float)(bb[0] +5 + x * stepX);
            prevPoints[i].y = (float)(bb[1]+ 5 + y * stepY);
            nextPoints[i].x = prevPoints[i].x;
            nextPoints[i].y = prevPoints[i].y;
            predPoints[i].x = prevPoints[i].x;
            predPoints[i].y = prevPoints[i].y;
        }
    }
    
   
    // Forward tracking using LK
    majd(prevFrame, nextFrame, prevPyramid, nextPyramid, prevPoints, nextPoints, TOTAL_POINTS, *windowSize, LEVEL, status, 0, *termCriteria, CV_LKFLOW_INITIAL_GUESSES);
    
    
    
     //Backward tracking using LK
     majd(nextFrame, prevFrame, nextPyramid, prevPyramid, nextPoints, predPoints, TOTAL_POINTS, *windowSize, LEVEL, 0, 0, *termCriteria, CV_LKFLOW_INITIAL_GUESSES | CV_LKFLOW_PYR_A_READY | CV_LKFLOW_PYR_B_READY);
     
     // Normalize Cross Correlation
     float *ncc    = (float*) malloc(TOTAL_POINTS*sizeof(float));
     float *fb     = (float*) malloc(TOTAL_POINTS*sizeof(float));
     // normCrossCorrelation(prevFrame,nextFrame,prevPoints,nextPoints,TOTAL_POINTS, status, ncc, 10,CV_TM_CCOEFF_NORMED);
     // CV_TM_CCORR_NORMED
     normCrossCorrelation(prevFrame,nextFrame,prevPoints,nextPoints,TOTAL_POINTS, status, ncc, 10,CV_TM_CCOEFF_NORMED);
     
     euclideanDistance( prevPoints,predPoints,fb,TOTAL_POINTS);
     
     //filtering dots
     int *good_points_part_1 = (int*) malloc(TOTAL_POINTS*sizeof(int));
     double ncc_median = median(ncc,TOTAL_POINTS);
     int good_points_1_idx = 0;
     for(i=0; i < TOTAL_POINTS; i++) {
         if(ncc[i] > ncc_median && status[i]==1) {
             good_points_part_1[good_points_1_idx] = i;
             good_points_1_idx++;
         }
     }
     
     // double fb_median = median(fb, TOTAL_POINTS);
     float *filtered_fb = (float*) malloc(sizeof(float) * good_points_1_idx);
     for(i =0; i < good_points_1_idx; i++)  {
         filtered_fb[i] = ncc[good_points_part_1[i]];
     }
     
     int good_points_2_idx = 0;
     int *good_points_part_2 = (int*) malloc(good_points_1_idx*sizeof(int));
     double fb_median = median(filtered_fb, good_points_1_idx);
     
     free(filtered_fb);
     for(i=0; i < good_points_1_idx; i++) {
         if(fb[good_points_part_1[i]] < fb_median) {
         
             good_points_part_2[good_points_2_idx] = good_points_part_1[i];
             good_points_2_idx++;
         }
     }
     
    // Calculate Bounding-Box Displacement -----------------------------------
    // Calculate the median displacement of the bounding-box in each dimension
    // We individually calculate the displacement of each point that was
    // successfully tracked and then find the median values in each dimension
    float *dxs = (float *)malloc(good_points_2_idx * sizeof(float));
    float *dys = (float *)malloc(good_points_2_idx * sizeof(float));
    int successful = 0;
    
    for(i = 0; i < good_points_2_idx; i++) {
        int f = good_points_part_2[i];
        if (status[f] == 1) {
            dxs[successful] = nextPoints[f].x - prevPoints[f].x;
            dys[successful] = nextPoints[f].y - prevPoints[f].y;
            successful++;
        } else {
            printf("mnyke\n");
        }
    }
     
     // Free unused arrays
     free(ncc);
     free(fb);
    
    
    // Get the median displacements
    
    double dispX = (double)median(dxs, successful);
    double dispY = (double)median(dys, successful);
    
    
    // Calculate Bounding-Box Scale Change -----------------------------------
    // For each possible pair of points we compute the ratio of distance apart
    // at time t + 1 to distance apart at time t
    // We then take the median ratio as the scale factor for the bounding-box
    // First, compute the maximum number of pairs in order to allocate our
    // ratios array
    
    
    int comparisons = 0;
    

    for(i = 1; i < good_points_2_idx; i++) {
        comparisons += i;
    }
    
    
    
    // Allocate the ratios array for storing each ratio and reset comparisons
    // to zero so we can count the number of successful comparisons
    
    
    
    float *scales = (float *)malloc(comparisons * sizeof(float));
    comparisons = 0;
    
     for (i = 0; i < good_points_2_idx; i++) {
         for (int j = i + 1; j < good_points_2_idx; j++) {

             int f = good_points_part_2[i];
             int f2 = good_points_part_2[j];
             
             if (status[f] == 1 && status[f2] == 1) {
             float dxPrev = prevPoints[f2].x - prevPoints[f].x;
             float dyPrev = prevPoints[f2].y - prevPoints[f].y;
             float dxNext = nextPoints[f2].x - nextPoints[f].x;
             float dyNext = nextPoints[f2].y - nextPoints[f].y;
             float sPrev = sqrt(dxPrev * dxPrev + dyPrev * dyPrev);
             float sNext = sqrt(dxNext * dxNext + dyNext * dyNext);
             
             if (sPrev != 0 && sNext != 0) {
                 scales[comparisons] = sNext / sPrev;
                 comparisons++;
             }
             } else {
                 printf("error!!\n");
             }
         }
     }
     free(good_points_part_1);
     free(good_points_part_2);
     
    
    
    
    // Get the median scale factor
    
    double scale = (double)median(scales, comparisons);
    //  double scale = 1.0;
    // Calculate the offset of the bounidng-box in x and y directions
    // We half the result because the bounding-box is made to expand about its
    // centre
    double offsetX = 0.5f * bbWidth * (scale - 1);
    double offsetY = 0.5f * bbHeight * (scale - 1);
    
    // Free memory
    cvReleaseImage(&prevFrame);
    prevFrame = cvCloneImage(nextFrame);
    free(dxs);
    free(dys);
    free(scales);
    
    
    // Set output ------------------------------------------------------------
    // We output the estimated new top-left and bottom-right coordinates of
    // the bounding-box. [top-left x, top-left y, width, height]
    double *bbNew = new double[5];
    bbNew[0] = bb[0] - offsetX + dispX;
    bbNew[1] = bb[1] - offsetY + dispY;
    bbNew[2] = bb[2] + offsetX * 2;
    bbNew[3] = bb[3] + offsetY * 2;
    
    
	if((bbNew[0] < 0)||(bbNew[1] < 0)||((bbNew[0]+bbNew[2]) > nextFrame->width)||((bbNew[1]+bbNew[3]) > nextFrame->height) || bbNew[2] <0 || bbNew[3] < 0)
	{
		bbNew[0] = 0;
		bbNew[1] = 0;
		bbNew[2] = 0;
		bbNew[3] = 0;
		bbNew[4] = 0;
	}
	else
	{
        printf("%lf,%lf,%lf,%lf\n",bbNew[0],bbNew[1],bbNew[2],bbNew[3]);
		bbNew[4] = (double)classifier->classify(nextFrameIntImg, (int)bbNew[0], (int)bbNew[1], (int)bbNew[2], (int)bbNew[3]);
	}
    
	
	cout<<"bbNew[0] = "<<bbNew[0]<<endl;
	cout<<"bbNew[1] = "<<bbNew[1]<<endl;
	cout<<"bbNew[2] = "<<bbNew[2]<<endl;
	cout<<"bbNew[3] = "<<bbNew[3]<<endl;
    cout<<"bbNew[4] = "<<bbNew[4]<<endl;
    
	return bbNew;

    
}
double *Tracker::track(IplImage *nextFrame, TldImage *nextFrameIntImg, double *bb) {
    if(USE_BACKWARD_TRACKING)
        return trackWithBackward(nextFrame,nextFrameIntImg, bb);
    
    // Perform Lucas-Kanade Tracking -----------------------------------------
    // Distribute points to track uniformly over the bounding-box
    double bbWidth = bb[2];
    double bbHeight = bb[3];
    double stepX = bbWidth / (DIM_POINTS + 1);
    double stepY = bbHeight / (DIM_POINTS + 1);
    int i, x, y;
    
    for (i = 0, x = 1; x <= DIM_POINTS; x++) {
        for (y = 1; y <= DIM_POINTS; y++, i++) {
            prevPoints[i].x = (float)(bb[0] + x * stepX);
            prevPoints[i].y = (float)(bb[1] + y * stepY);
            nextPoints[i].x = prevPoints[i].x;
            nextPoints[i].y = prevPoints[i].y;
        }
    }
    
    // Calculate optical flow with the iterative Lucas-Kanade method in pyramids
    // Last parameter flag meanings:
    // CV_LKFLOW_PYR_A_READY: pyramid A is precalculated before the call
    // CV_LKFLOW_PYR_B_READY: pyramid B is precalculated before the call
    // CV_LKFLOW_INITIAL_GUESSES: array B contains initial coordinates of features before the function call
    
    // Forward tracking using LK
    cvCalcOpticalFlowPyrLK(prevFrame, nextFrame, prevPyramid, nextPyramid, prevPoints, nextPoints, TOTAL_POINTS, *windowSize, LEVEL, status, 0, *termCriteria, CV_LKFLOW_INITIAL_GUESSES);
    
    
    // Calculate Bounding-Box Displacement -----------------------------------
    // Calculate the median displacement of the bounding-box in each dimension
    // We individually calculate the displacement of each point that was
    // successfully tracked and then find the median values in each dimension
    float *dxs = (float *)malloc(TOTAL_POINTS * sizeof(float));
    float *dys = (float *)malloc(TOTAL_POINTS * sizeof(float)); 

    int successful = 0;
    
    for (i = 0, x = 0; x < DIM_POINTS; x++) {
        for (y = 0; y < DIM_POINTS; y++, i++) {
            if (status[i] == 1) {
                dxs[successful] = nextPoints[i].x - prevPoints[i].x;
                dys[successful] = nextPoints[i].y - prevPoints[i].y;
                successful++;
            } 
        }
    }
    // Get the median displacements
    
    double dispX = (double)median(dxs, successful);
    double dispY = (double)median(dys, successful);
    
    
    // Calculate Bounding-Box Scale Change -----------------------------------
    // For each possible pair of points we compute the ratio of distance apart
    // at time t + 1 to distance apart at time t
    // We then take the median ratio as the scale factor for the bounding-box
    // First, compute the maximum number of pairs in order to allocate our
    // ratios array
    
    
    int comparisons = 0;
    
    for (i = 1; i < TOTAL_POINTS; i++) {
        comparisons += i;
    }
     
     
    
    // Allocate the ratios array for storing each ratio and reset comparisons
    // to zero so we can count the number of successful comparisons
    
    
    
    float *scales = (float *)malloc(comparisons * sizeof(float));
    comparisons = 0;
    
    for (i = 0; i < TOTAL_POINTS; i++) {
        for (int j = i + 1; j < TOTAL_POINTS; j++) {
            if (status[i] == 1 && status[j] == 1) {
                float dxPrev = prevPoints[j].x - prevPoints[i].x;
                float dyPrev = prevPoints[j].y - prevPoints[i].y;
                float dxNext = nextPoints[j].x - nextPoints[i].x;
                float dyNext = nextPoints[j].y - nextPoints[i].y;
                float sPrev = sqrt(dxPrev * dxPrev + dyPrev * dyPrev);
                float sNext = sqrt(dxNext * dxNext + dyNext * dyNext);
                
                if (sPrev != 0 && sNext != 0) {
                    scales[comparisons] = sNext / sPrev;
                    comparisons++;
                }
            }
        }
    }

    // Get the median scale factor
  
    double scale = (double)median(scales, comparisons);
    
    // Calculate the offset of the bounidng-box in x and y directions
    // We half the result because the bounding-box is made to expand about its
    // centre
    double offsetX = 0.5f * bbWidth * (scale - 1);
    double offsetY = 0.5f * bbHeight * (scale - 1);
    
    // Free memory
    cvReleaseImage(&prevFrame);
    prevFrame = cvCloneImage(nextFrame);
    free(dxs);
    free(dys);
    free(scales);
    
    
    // Set output ------------------------------------------------------------
    // We output the estimated new top-left and bottom-right coordinates of
    // the bounding-box. [top-left x, top-left y, width, height]
    double *bbNew = new double[5];
    bbNew[0] = bb[0] - offsetX + dispX;
    bbNew[1] = bb[1] - offsetY + dispY;
    bbNew[2] = bb[2] + offsetX * 2;
    bbNew[3] = bb[3] + offsetY * 2;
   

	if((bbNew[0] < 0)||(bbNew[1] < 0)||((bbNew[0]+bbNew[2]) > nextFrame->width)||((bbNew[1]+bbNew[3]) > nextFrame->height))
	{
		bbNew[0] = 0;
		bbNew[1] = 0;
		bbNew[2] = 0;
		bbNew[3] = 0;
		bbNew[4] = 0;
	}
	else
	{
		bbNew[4] = (double)classifier->classify(nextFrameIntImg, (int)bbNew[0], (int)bbNew[1], (int)bbNew[2], (int)bbNew[3]);
	}
    
	
	cout<<"bbNew[0] = "<<bbNew[0]<<endl;
	cout<<"bbNew[1] = "<<bbNew[1]<<endl;
	cout<<"bbNew[2] = "<<bbNew[2]<<endl;
	cout<<"bbNew[3] = "<<bbNew[3]<<endl;
    cout<<"bbNew[4] = "<<bbNew[4]<<endl;
    
	return bbNew;
}


void Tracker::setPrevFrame(IplImage *frame) {
    cvReleaseImage(&prevFrame);
    prevFrame = cvCloneImage(frame);
}
void Tracker::euclideanDistance (CvPoint2D32f *point1, CvPoint2D32f *point2, float *match, int nPts) {
    
	for (int i = 0; i < nPts; i++) {
        
		match[i] = sqrt((point1[i].x - point2[i].x)*(point1[i].x - point2[i].x) + 
                        (point1[i].y - point2[i].y)*(point1[i].y - point2[i].y) );
        
	}
}

void Tracker::normCrossCorrelation(IplImage *imgI, IplImage *imgJ, CvPoint2D32f *points0, CvPoint2D32f *points1, int nPts, char *status, float *match,int winsize, int method) {
    
    
	IplImage *rec0 = cvCreateImage( cvSize(winsize, winsize), 8, 1 );
	IplImage *rec1 = cvCreateImage( cvSize(winsize, winsize), 8, 1 );
	IplImage *res  = cvCreateImage( cvSize( 1, 1 ), IPL_DEPTH_32F, 1 );
    
	for (int i = 0; i < nPts; i++) {
		if (status[i] == 1) {
			cvGetRectSubPix( imgI, rec0, points0[i] );
			cvGetRectSubPix( imgJ, rec1, points1[i] );
			cvMatchTemplate( rec0,rec1, res, method );
			match[i] = ((float *)(res->imageData))[0]; 
            
		} else {
			match[i] = 0.0;
		}
	}
	cvReleaseImage( &rec0 );
	cvReleaseImage( &rec1 );
	cvReleaseImage( &res );
    
}

Tracker::~Tracker() {
    free(prevFrame);
    cvReleaseImage(&prevPyramid);
    cvReleaseImage(&nextPyramid);
    cvFree(&prevPoints);
    cvFree(&nextPoints);
    //cvFree(&predPoints);
    free(windowSize);
    cvFree(&status);
    //cvFree(&predStatus);
    free(termCriteria);
}
