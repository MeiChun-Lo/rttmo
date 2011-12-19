#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>




using namespace std;
using namespace cv;


#define SLIDER_MAX 4
#ifndef CV_WINDOW_KEEPRATIO
#define CV_WINDOW_KEEPRATIO 0
#endif

#include "tmo.h"

int m_live_usb = 1;
int m_cap_hdr = 1;
int m_init = 0;
int m_mode = 3;
int m_test = 1;
int m_test2 = 1;


Mat runHDR(Mat& im1, Mat& im2, Mat& im3) {

    MSG("hdr...");

    Mat hdr(im1.rows, im1.cols, CV_32FC3);
    if (m_cap_hdr) { makehdr3log(&im1, &im2, &im3, &hdr); hdr *= 15;}
    else { im1.convertTo(hdr, CV_32FC3); }

    Mat hdr0(hdr);

    if (m_mode == 4) {

        // mantiuk
        Mat luv(hdr.rows, hdr.cols, CV_32FC3);
        cvtColor(hdr, luv, CV_BGR2YCrCb);

        vector<Mat> lplanes;
        split(luv, lplanes);
        Mat Y(hdr.rows, hdr.cols, CV_32FC1);
        lplanes[0].convertTo(Y, CV_32FC1);

        vector<Mat> cplanes;
        split(hdr, cplanes);
        Mat R(hdr.rows, hdr.cols, CV_32FC1);
        cplanes[0].convertTo(R, CV_32FC1);
        Mat G(hdr.rows, hdr.cols, CV_32FC1);
        cplanes[1].convertTo(G, CV_32FC1);
        Mat B(hdr.rows, hdr.cols, CV_32FC1);
        cplanes[2].convertTo(B, CV_32FC1);

        bool bcg = false;
        int itmax = 256 * 3;
        float tol = 5e-2;
        int cols = hdr.cols;
        int rows = hdr.rows;

        int m_contrast = 1;
        int m_saturation = 1;
        int m_detail = 1;
        float contrast = (m_contrast) ? 0.25 : -0.25 ;   /* neg = contrast eq | pos = contrast map */
        float saturation = (m_saturation) ? 1.25 : 0.85 ;
        float detail = (m_detail) ? 2.0 : 1.0 ;

        float* fR = (float*)R.data;
        float* fG = (float*)G.data;
        float* fB = (float*)B.data;
        float* fY = (float*)Y.data;

        tmo_mantiuk06_contmap(cols, rows,
                              fR,
                              fG,
                              fB,
                              fY,
                              contrast, saturation, detail, bcg, itmax, tol);

        Mat rgb[] = {R, G, B};
        merge(rgb, 3, hdr);
        hdr *= 255;

    } else if (m_mode == 3) {

        float cc = (float)m_test / 10.0;
        MSG("test %d %f", m_test, cc);

        // sharpen image using "unsharp mask" algorithm
        Mat img;
        Mat blurred; double sigma = 7, threshold = 0, amount = cc;//1.66f;
        Mat sharpened;
        Mat lowContrastMask;


        Mat xyz(hdr.rows, hdr.cols, CV_32FC3);
        cvtColor(hdr, xyz, CV_BGR2XYZ);

        vector<Mat> lplanes;
        split(xyz, lplanes);
        Mat Y(hdr.rows, hdr.cols, CV_32FC1);
        Mat X(hdr.rows, hdr.cols, CV_32FC1);
        Mat Z(hdr.rows, hdr.cols, CV_32FC1);
        lplanes[1].convertTo(Y, CV_32FC1);
        lplanes[0].convertTo(X, CV_32FC1);
        lplanes[2].convertTo(Z, CV_32FC1);
        Mat L(hdr.rows, hdr.cols, CV_32FC1);


        float cc2 = (float)m_test2 / 10.0;
        MSG("test2 %d %f", m_test2, cc2);
        // sharpen image using "unsharp mask" algorithm
        img = Y;
        sigma = 1; threshold = 0; amount = cc2;//1.66f;
        GaussianBlur(img, blurred, Size(), sigma, sigma);
        L = blurred / (1 + blurred);
        Mat scale(hdr.rows, hdr.cols, CV_32FC1);
        scale = 1 / L * cc2;
        multiply(Y, scale, Y);
        multiply(X, scale, X);
        multiply(Z, scale, Z);

        Mat rgb[] = {X, Y, Z};
        merge(rgb, 3, hdr);
        cvtColor(hdr, hdr, CV_XYZ2BGR);




        img = hdr;
        sigma = 7; threshold = 0; amount = cc;//1.66f;
        GaussianBlur(img, blurred, Size(), sigma, sigma);
        sharpened = img * (1 + amount) + blurred * (-amount);
        lowContrastMask = abs(img - blurred) < threshold;
        img.copyTo(sharpened, lowContrastMask);
        hdr = sharpened;
        // // clamp
        hdr = min(hdr, 255);
        hdr = max(hdr, 0);


    } else if (m_mode == 2) {
        float ccc = (float)m_test / 10.0;
        MSG("test %d %f", m_test, ccc);

        // drago
        Mat xyz(hdr.rows, hdr.cols, CV_32FC3);
        cvtColor(hdr, xyz, CV_BGR2XYZ);

        vector<Mat> lplanes;
        split(xyz, lplanes);
        Mat Y(hdr.rows, hdr.cols, CV_32FC1);
        Mat X(hdr.rows, hdr.cols, CV_32FC1);
        Mat Z(hdr.rows, hdr.cols, CV_32FC1);
        lplanes[1].convertTo(Y, CV_32FC1);
        lplanes[0].convertTo(X, CV_32FC1);
        lplanes[2].convertTo(Z, CV_32FC1);

        float bias = ccc;//1.15f;// 0.99;  // 0.85;
        int width = hdr.cols;
        int height = hdr.rows;
        Mat L(hdr.rows, hdr.cols, CV_32FC1);
        float* fY = (float*)Y.data;
        float* fL = (float*)L.data;

        tmo_drago03(width, height,
                    fY,
                    fL,
                    bias);

        Mat scale(hdr.rows, hdr.cols, CV_32FC1);
        divide(L, Y, scale);
        multiply(Y, scale, Y);
        multiply(X, scale, X);
        multiply(Z, scale, Z);

        Mat rgb[] = {X, Y, Z};
        merge(rgb, 3, hdr);
        cvtColor(hdr, hdr, CV_XYZ2BGR);
        hdr *= 255;

        // float cc = 1.1;
        // // gamma
        // Mat tmp = hdr/255.0f;
        // pow(tmp,cc,hdr);
        // hdr*=255;

    } else if (m_mode == 1) {

        float cc = (float)m_test / 5.0;
        MSG("test %d %f", m_test, cc);



        // sharpen image using "unsharp mask" algorithm
        Mat img = hdr;
        Mat blurred; double sigma = 7, threshold = 0, amount = 0.50;//1.66f;
        GaussianBlur(img, blurred, Size(), sigma, sigma);
        Mat sharpened = img * (1 + amount) + blurred * (-amount);
        Mat lowContrastMask = abs(img - blurred) < threshold;
        img.copyTo(sharpened, lowContrastMask);
        hdr = sharpened;

        // // float cc = (float)m_test/10.0;
        // // MSG("test %d %f",m_test, cc);
        // float cc = 1.1;
        // // gamma
        // Mat tmp = hdr/255.0f;
        // pow(tmp,cc,hdr);
        // hdr*=255;

    } else {

        float cc = (float)m_test / 10.0;
        MSG("test %d %f", m_test, cc);

        Mat tmp = hdr / 255.0f;
        pow(tmp, cc, hdr);
        hdr *= 255;

    }


    // Clamp
    hdr = min(hdr, 255);
    hdr = max(hdr, 0);

    // hdr.convertTo(hdr, CV_8UC3);
    // MSG("done.");
    // imshow("tmo", hdr); //cvWaitKey(0);
    return hdr;
}


int main(int argc, char** argv) {

    if (argc < 2)  {
        fprintf(stderr, "uh = live usb hdr | un = live usb raw | img1-3 | movie file \n usage:  %s uh \n         %s un \n         %s img1 img2 img3 \n         %s video-filename \n\n", argv[0], argv[0], argv[0], argv[0]);
        return 1;
    }
    MSG("loading...");

    if (argc == 4) {

        IplImage* img1 = cvLoadImage(argv[1], 1);
        Mat im1(img1);
        IplImage* img2 = cvLoadImage(argv[2], 1);
        Mat im2(img2);
        IplImage* img3 = cvLoadImage(argv[3], 1);
        Mat im3(img3);

        namedWindow("im1", CV_WINDOW_KEEPRATIO);
        imshow("im1", im1);
        namedWindow("im2", CV_WINDOW_KEEPRATIO);
        imshow("im2", im2);
        namedWindow("im3", CV_WINDOW_KEEPRATIO);
        imshow("im3", im3);

        if (0 == m_init)  { namedWindow("tmo", CV_WINDOW_KEEPRATIO); }

        runHDR(im1, im2, im3);

        while (1) {
            char key;
            key = (char) cvWaitKey(10);
            if (key == 27 || key == 'q' || key == 'Q') { break; }
        }

        return 0;

    } else {

        string s = argv[1];
        if (s.compare(string("uh")) == 0) { m_live_usb = 1; m_cap_hdr = 1; }
        else if (s.compare(string("un")) == 0) { m_live_usb = 1; m_cap_hdr = 0; }
        else { m_live_usb = 0; m_cap_hdr = 0; }

        namedWindow("trackbar", CV_WINDOW_KEEPRATIO); cvMoveWindow("trackbar", 10, 10);
        imshow("trackbar", Mat(70, 300, CV_8UC1));
        createTrackbar("mode", "trackbar", &m_mode, SLIDER_MAX, 0);
        createTrackbar("test", "trackbar", &m_test, 20, 0);
        createTrackbar("test2", "trackbar", &m_test2, 20, 0);

        // camera setup
        Mat cimage;
        VideoCapture  capture;

        if (!m_live_usb) {
            capture.open(argv[1]);
        } else {
            int w = 320;
            int h = 240;
            capture.set(CV_CAP_PROP_FRAME_WIDTH, w);
            capture.set(CV_CAP_PROP_FRAME_HEIGHT, h);
            capture.open(0); //try to open
            capture.set(CV_CAP_PROP_FRAME_WIDTH, w);
            capture.set(CV_CAP_PROP_FRAME_HEIGHT, h);
        }

        if (!capture.isOpened()) { //if this fails...
            cerr << "Failed to open a video device or video file!\n" << endl;
            return 1;
        }

        if (0 == m_init) { namedWindow("cam", CV_WINDOW_KEEPRATIO); cvMoveWindow("cam", 10, 200); }
        if (0 == m_init) { namedWindow("tmo", CV_WINDOW_KEEPRATIO); cvMoveWindow("tmo", 10, 500); }
        if (0 == m_init) { namedWindow("tmp", CV_WINDOW_KEEPRATIO); cvMoveWindow("tmp", 300, 500); }
        if (0 == m_init) { namedWindow("tmp2", CV_WINDOW_KEEPRATIO); cvMoveWindow("tmp2", 300, 500); }

        // camera process loop
        while (1) {

            // get camera image
            capture >> cimage;
            if (cimage.empty()) {
                cout << "Couldn't load " << endl;
                break;
            }
            imshow("cam", cimage);


            if (m_live_usb && m_cap_hdr) {

                // hdr capture
                capture.set(CV_CAP_PROP_BRIGHTNESS, 0.05);
                capture >> cimage;
                capture >> cimage;
                capture >> cimage;
                capture >> cimage;
                Mat img1;
                cimage.copyTo(img1);

                capture.set(CV_CAP_PROP_BRIGHTNESS, 0.45);
                capture >> cimage;
                capture >> cimage;
                capture >> cimage;
                capture >> cimage;
                Mat img2;
                cimage.copyTo(img2);

                capture.set(CV_CAP_PROP_BRIGHTNESS, 0.99);
                capture >> cimage;
                capture >> cimage;
                capture >> cimage;
                capture >> cimage;
                Mat img3;
                cimage.copyTo(img3);

                capture.set(CV_CAP_PROP_BRIGHTNESS, 0.45);
                capture >> cimage;
                capture >> cimage;
                capture >> cimage;
                capture >> cimage;

                runHDR(img1, img2, img3);

            } else if (m_live_usb && !m_cap_hdr) {

                // raw usb
                Mat img1, img2, img3;
                cimage.copyTo(img1);
                runHDR(img1, img2, img3);

            } else {

                // loaded video
                Mat img1, img2, img3;
                cimage.copyTo(img1);
                Mat hdr = runHDR(img1, img2, img3);
                Mat mtmp = Mat::zeros(hdr.rows, hdr.cols, CV_8UC3);


                // clamp
                hdr = min(hdr, 255);
                hdr = max(hdr, 0);

                hdr.convertTo(mtmp, CV_8UC3);
                imshow("tmo", mtmp);


            }


            // quit?
            char key;
//            key = (char) cvWaitKey(10); // for real-time
            key = (char) cvWaitKey(1000); // for testing
            if (key == 27 || key == 'q' || key == 'Q') { break; }
            m_init = 1;
        }

        return 0;

    } // live view

}

