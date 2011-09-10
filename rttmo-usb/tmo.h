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
int tmo_mantiuk06_contmap(int c, int r, float* R, float* G, float* B, float* Y,
                          float contrastFactor, float saturationFactor, float detailFactor, bool bcg,
                          int itmax = 50, float tol = 1e-2);






/**
 * @brief Frederic Drago Logmapping Algorithm
 *
 * Implementation obtained from source code provided
 * by Frederic Drago on 16 May 2003.
 *
 * @param width image width
 * @param height image height
 * @param Y [in] image luminance values
 * @param L [out] tone mapped values
 * @param maxLum maximum luminance in the image
 * @param avLum logarithmic average of luminance in the image
 * @param bias bias parameter of tone mapping algorithm (eg 0.85)
 */

void tmo_drago03(unsigned int width, unsigned int height,
                 const float* Y, float* L,
                 float bias);

/**
 * @brief Find average and maximum luminance in an image
 *
 * @param Y [in] image luminance values
 * @param avLum [out] average luminance
 * @param maxLum [out] maximum luminance
 */
void calculateLuminance(unsigned int width, unsigned int height, const float* Y, float& avLum, float& maxLum);

#endif // __TMO_H__
