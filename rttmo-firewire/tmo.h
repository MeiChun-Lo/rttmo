#ifndef __TMO_H__
#define __TMO_H__

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

using namespace std;
using namespace cv;

#define MSG(msg,...) do {                                   \
	        fprintf(stdout,__FILE__":%d(%s) " msg "\n",     \
	                __LINE__, __FUNCTION__, ##__VA_ARGS__); \
	        fflush(stdout);                                 \
	    } while (0)


void makehdr3(Mat* im1, Mat* im2, Mat* im3, Mat* hdr);
void makehdr2(Mat* im1, Mat* im3, Mat* hdr);
void makehdr3log(Mat* im1, Mat* im2, Mat* im3, Mat* hdr);
void makehdr2log(Mat* im1, Mat* im3, Mat* hdr);

/**
 * @brief: Tone mapping algorithm [Mantiuk2006]
 *
 * @param R red channel
 * @param G green channel
 * @param B blue channel
 * @param Y luminance channel
 * @param contrastFactor contrast scaling factor (in 0-1 range)
 * @param saturationFactor color desaturation (in 0-1 range)
 * @param bcg true if to use BiConjugate Gradients, false if to use Conjugate Gradients
 * @param itmax maximum number of iterations for convergence (typically 50)
 * @param tol tolerence to get within for convergence (typically 1e-3)
 * @return PFSTMO_OK if tone-mapping was sucessful, PFSTMO_ABORTED if
 * it was stopped from a callback function and PFSTMO_ERROR if an
 * error was encountered.
 */
int tmo_mantiuk06_contmap( int c, int r, float* R, float* G, float* B, float* Y,
						   float contrastFactor, float saturationFactor, float detailFactor, bool bcg,
						   int itmax = 50, float tol = 1e-2);

#endif // __TMO_H__
