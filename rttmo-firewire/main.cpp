#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>

#include "GuppyCam.h"
GuppyCam camera_firewire;

using namespace std;
using namespace cv;

#define LIVE_VIEW 1

#ifndef CV_WINDOW_KEEPRATIO
#define CV_WINDOW_KEEPRATIO 0
#endif

#include "tmo.h"

int m_init = 0;
int m_contrast = 0;
int m_saturation = 0;
int m_detail = 0;
#define SLIDER_MAX 1

void runHDR(Mat& im1, Mat& im2, Mat& im3) {

	MSG("hdr...");

	Mat hdr(im2.rows, im2.cols, CV_32FC3);
	makehdr3log(&im1, &im2, &im3, &hdr);
//	makehdr2log(&im1, &im3, &hdr);

//	imshow("hdr", hdr);

	/////////////////////////

	Mat luv( hdr.rows, hdr.cols, CV_32FC3 );
	cvtColor(hdr, luv, CV_BGR2YCrCb);

	vector<Mat> lplanes;
	split(luv, lplanes);
	Mat Y(hdr.rows, hdr.cols, CV_32FC1);
	lplanes[0].convertTo(Y, CV_32FC1);


	/////////////////////////

	vector<Mat> cplanes;
	split(hdr, cplanes);
	Mat R(hdr.rows, hdr.cols, CV_32FC1);
	cplanes[0].convertTo(R, CV_32FC1);
	Mat G(hdr.rows, hdr.cols, CV_32FC1);
	cplanes[1].convertTo(G, CV_32FC1);
	Mat B(hdr.rows, hdr.cols, CV_32FC1);
	cplanes[2].convertTo(B, CV_32FC1);

	/////////////////////////

	bool bcg = false;
	int itmax = 256;
	float tol = 1e-2;
	int cols = hdr.cols;
	int rows = hdr.rows;

//	float contrast = -0.133; /* neg = contrast eq | pos = contrast map */
//	float saturation = 1.133;
//	float detail = 1.133;

	float contrast = (m_contrast) ? 0.11 : -0.11 ;
	float saturation = (m_saturation) ? 1.5 : 1.0 ;
	float detail = (m_detail) ? 2.0 : 1.0 ;

    MSG("cont %f",contrast);
    MSG("sat %f",saturation);
    MSG("detail %f",detail);

	float* fR = (float*)R.data;
	float* fG = (float*)G.data;
	float* fB = (float*)B.data;
	float* fY = (float*)Y.data;

	MSG("tonemapping...");

	tmo_mantiuk06_contmap(cols, rows,
						  fR,
						  fG,
						  fB,
						  fY,
						  contrast, saturation, detail, bcg, itmax, tol);

	Mat rgb[] = {R, G, B};
	merge(rgb, 3, hdr);

	MSG("done.");

	/////////////////////////

	imshow("tmo", hdr);

	m_init = 1;
}


int main(int argc, char** argv) {

#if (LIVE_VIEW==0)

	if (argc != 4)  {
		fprintf(stderr, "usage: %s img1 img2 img3 \n", argv[0]);
		return 1;
	}

	MSG("loading...");

	IplImage* img1 = cvLoadImage(argv[1], 1);
	Mat im1(img1);
	IplImage* img2 = cvLoadImage(argv[2], 1);
	Mat im2(img2);
	IplImage* img3 = cvLoadImage(argv[3], 1);
	Mat im3(img3);

	/////////////////////////

	namedWindow("im1", CV_WINDOW_KEEPRATIO);
	imshow("im1", im1);
	namedWindow("im2", CV_WINDOW_KEEPRATIO);
	imshow("im2", im2);
	namedWindow("im3", CV_WINDOW_KEEPRATIO);
	imshow("im3", im3);

	/////////////////////////
//	if (0==m_init)  { namedWindow("hdr", CV_WINDOW_KEEPRATIO); }
	if (0==m_init)  { namedWindow("tmo", CV_WINDOW_KEEPRATIO); }

	runHDR(im1, im2, im3);

	/////////////////////////

	while (1) {
		char key;
		key = (char) cvWaitKey(10);
		if ( key == 27 || key == 'q' || key == 'Q' ) { break; }
	}

	/////////////////////////

	return 0;

#else

	namedWindow("trackbar", CV_WINDOW_KEEPRATIO);
	createTrackbar( "contrast", "trackbar", &m_contrast, SLIDER_MAX, 0 );
	createTrackbar( "saturation", "trackbar", &m_saturation, SLIDER_MAX, 0 );
	createTrackbar( "detail", "trackbar", &m_detail, SLIDER_MAX, 0 );


	// views
	IplImage* rawi =  cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 3);
	int div = 4;
	Mat img1;
	Mat img2;
	Mat img3;

	if (!camera_firewire.connect()) {
		printf("Camera connect failure \n");
		printf("Try using sudo... \n");
		exit(-1);
	} else {
		camera_firewire.loadSettings();
		printf("Camera settings loaded \n");
	}

	if (0==m_init)	{ namedWindow("im1", CV_WINDOW_KEEPRATIO); }
	if (0==m_init)	{ namedWindow("im2", CV_WINDOW_KEEPRATIO); }
	if (0==m_init)	{ namedWindow("im3", CV_WINDOW_KEEPRATIO); }
//	if (0==m_init)  { namedWindow("hdr", CV_WINDOW_KEEPRATIO); }
	if (0==m_init)  { namedWindow("tmo", CV_WINDOW_KEEPRATIO); }


	camera_firewire.setAllAuto(false);
	camera_firewire.setAutoWhiteBalance(true);
	camera_firewire.setGain(300);
    camera_firewire.setExposure(1023);

	// camera process loop
	while (1) {


		printf(" capture \n");
		camera_firewire.GrabCvImage(rawi);
		camera_firewire.GrabCvImage(rawi);
		Mat cimage(rawi);

		/////////////////////////

#if 1
		//camera_firewire.setExposure(1023);
		camera_firewire.setShutter(225);
		camera_firewire.GrabCvImage(rawi);

		Mat _img1(rawi);
		resize(_img1, img1, Size(cimage.cols / div, cimage.rows / div));
		imshow("im1", img1);

		//camera_firewire.setExposure(1023);
		camera_firewire.setShutter(340);
		camera_firewire.GrabCvImage(rawi);

		Mat _img2(rawi);
		resize(_img2, img2, Size(cimage.cols / div, cimage.rows / div));
		imshow("im2", img2);

		//camera_firewire.setExposure(1023);
		camera_firewire.setShutter(700);
		camera_firewire.GrabCvImage(rawi);
		camera_firewire.GrabCvImage(rawi);

		Mat _img3(rawi);
		resize(_img3, img3, Size(cimage.cols / div, cimage.rows / div));
		imshow("im3", img3);

		/////////////////////////

		runHDR(img1, img2, img3);

		/////////////////////////

#else
	imshow("cam", cimage);
#endif


		// quit?
		char key;
		key = (char) cvWaitKey(10);
		if ( key == 27 || key == 'q' || key == 'Q' ) { break; }


		m_init = 1;
	}

	return 0;

#endif // live view

}

