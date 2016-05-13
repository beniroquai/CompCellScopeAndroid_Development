/*
 * cvComplex.cpp
 * A set of functions for dealing with two channel complex matricies inside
 * the OpenCV 2.4.* Imaging processing library. Complex numbers are generally
 * dealt with using a 2 color channel Mat object rather than std::complex. 
 * This library provides several common functions for manipulating matricies
 * in this format.
 *
 * Maintained by Z. Phillips, Computational Imaging Lab, UC Berkeley
 * Report bugs directly to zkphil@berkeley.edu
 */
 
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <string>
#include <math.h>
#include <stdio.h>
#include <dirent.h>
#include <fstream>
#include <vector>
#include "cvComplex.h"

int16_t gv_cMap = -1; // Global Colormap Setting

const int16_t SHOW_COMPLEX_MAG = 0;
const int16_t SHOW_COMPLEX_COMPONENTS = 1;
const int16_t SHOW_COMPLEX_REAL = 2;
const int16_t SHOW_COMPLEX_IMAGINARY = 3;
const int16_t SHOW_AMP_PHASE = 4;

const int16_t CMAP_MIN = 0;
const int16_t CMAP_MAX = 11;
const int16_t COLORMAP_NONE = -1;

using namespace cv;
using namespace std;

void circularShift(cv::Mat img, cv::Mat result, int x, int y){
    int w = img.cols;
    int h = img.rows;

    int shiftR = x % w;
    int shiftD = y % h;
    
    if (shiftR < 0)//if want to shift in -x direction
        shiftR += w;
    
    if (shiftD < 0)//if want to shift in -y direction
        shiftD += h;

    cv::Rect gate1(0, 0, w-shiftR, h-shiftD);//rect(x, y, width, height)
    cv::Rect out1(shiftR, shiftD, w-shiftR, h-shiftD);
    
    cv::Rect gate2(w-shiftR, 0, shiftR, h-shiftD); 
    cv::Rect out2(0, shiftD, shiftR, h-shiftD);
    
    cv::Rect gate3(0, h-shiftD, w-shiftR, shiftD);
    cv::Rect out3(shiftR, 0, w-shiftR, shiftD);
    
    cv::Rect gate4(w-shiftR, h-shiftD, shiftR, shiftD);
    cv::Rect out4(0, 0, shiftR, shiftD);
   
    cv::Mat shift1 = img ( gate1 );
	 cv::Mat shift2 = img ( gate2 );
	 cv::Mat shift3 = img ( gate3 );
	 cv::Mat shift4 = img ( gate4 );
	 shift1.copyTo(cv::Mat(result, out1));
	 shift2.copyTo(cv::Mat(result, out2));
	 shift3.copyTo(cv::Mat(result, out3));
	 shift4.copyTo(cv::Mat(result, out4));
}



double fastSqrt(double x) {
    if (x <= 0)
        return 0;       // if negative number throw an exception?
    int exp = 0;
    x = frexp(x, &exp); // extract binary exponent from x
    if (exp & 1) {      // we want exponent to be even
        exp--;
        x *= 2;
    }
    double y = (1+x)/2; // first approximation
    double z = 0;
    while (y != z) {    // yes, we CAN compare doubles here!
        z = y;
        y = (y + x/y) / 2;
    }
    return ldexp(y, exp/2); // multiply answer by 2^(exp/2)
}

void maxComplexReal(cv::Mat& m, std::string label)
{
      cv::Mat planes[] = {cv::Mat::zeros(m.rows, m.cols, m.type()), cv::Mat::zeros(m.rows, m.cols, m.type())};
      cv::split(m,planes);
      double minVal, maxVal;
      cv::minMaxLoc(planes[0], &minVal, &maxVal);
      std::cout << "Max/Min values of " <<label << " are: " << maxVal << ", " << minVal << std::endl;
}

void complexConj(const cv::Mat& m, cv::Mat& output)
{
   if (output.empty())
   	output = cv::Mat::zeros(m.rows, m.cols, CV_64FC2);
   	
	for(int i = 0; i < m.rows; i++) // loop through y
	{
    const double* m_i = m.ptr<double>(i);  // Input
    double* o_i = output.ptr<double>(i);   // Output
    
    for(int j = 0; j < m.cols; j++)
    {
        o_i[j*2] = (double) m_i[j*2];
        o_i[j*2+1] = (double) -1.0 * m_i[j*2+1];
    }
	}
}

void complexAbs(const cv::Mat& m, cv::Mat& output)
{

   if (output.empty())
   	output = cv::Mat::zeros(m.rows, m.cols, CV_64FC2);

	for(int i = 0; i < m.rows; i++) // loop through y
	{
    const double* m_i = m.ptr<double>(i);  // Input
    double* o_i = output.ptr<double>(i);   // Output

    for(int j = 0; j < m.cols; j++)
    {

        o_i[j*2] = (double) std::sqrt(m_i[j*2] * m_i[j*2] + m_i[j*2+1] * m_i[j*2+1]);
        o_i[j*2+1] = 0.0;
    }
	}
}

void complexAbs2(const cv::Mat& m, cv::Mat& output)
{

   if (output.empty())
   	output = cv::Mat::zeros(m.rows, m.cols, CV_64FC1);

	for(int i = 0; i < m.rows; i++) // loop through y
	{
    const double* m_i = m.ptr<double>(i);  // Input
    double* o_i = output.ptr<double>(i);   // Output

    for(int j = 0; j < m.cols; j++)
    {

        o_i[j] = (double) std::sqrt(m_i[j*2] * m_i[j*2] + m_i[j*2+1] * m_i[j*2+1]);
    }
	}
}




void complexAngle(const cv::Mat& m, cv::Mat& output)
{

   if (output.empty())
   	output = cv::Mat::zeros(m.rows, m.cols, CV_64FC1);

	for(int i = 0; i < m.rows; i++) // loop through y
	{
    const double* m_i = m.ptr<double>(i);  // Input
    double* o_i = output.ptr<double>(i);   // Output

    for(int j = 0; j < m.cols; j++)
    {
        o_i[j] = (double) atan(m_i[j*2+1]/m_i[j*2]);
    }
	}
}

/* complexMultiply(const cv::Mat& m1, const cv::Mat& m2, cv::Mat& output)
 * Multiplies 2 complex matricies where the first two color channels are the
 * real and imaginary coefficents, respectivly. Uses the equation:
 *         (a + bi) * (c + di) = (ac - bd) + (ad + bc)i
 *
 * INPUTS:
 *   const cv::Mat& m1:  Complex Matrix 1
 *   const cv::Mat& m2:  Complex Matrix 2
 * OUTPUT:
 *   cv::Mat& outpit:    Complex Product of m1 and m2 
 */
void complexMultiply(const cv::Mat& m1, const cv::Mat& m2, cv::Mat& output)
{
   if (output.empty())
   	output = cv::Mat::zeros(m1.rows, m1.cols, CV_64FC2);
   // (a + bi) * (c + di) = (ac - bd) + (ad + bc)i
	for(int i = 0; i < m1.rows; i++) // loop through y
	{
    const double* m1_i = m1.ptr<double>(i);   // Input 1
    const double* m2_i = m2.ptr<double>(i);   // Input 2
    double* o_i = output.ptr<double>(i);      // Output
    for(int j = 0; j < m1.cols; j++)
    {
        o_i[j*2] = (m1_i[j*2] * m2_i[j*2]) - (m1_i[j*2+1] * m2_i[j*2+1]);    // Real
        o_i[j*2+1] = (m1_i[j*2] * m2_i[j*2+1]) + (m1_i[j*2+1] * m2_i[j*2]);  // Imaginary
    }
	}
}

void complexScalarMultiply(double scaler, cv::Mat& m, cv::Mat output)
{
   if (output.empty())
   	output = cv::Mat::zeros(m.rows, m.cols, CV_64FC2);
   // (a + bi) * (c + di) = (ac - bd) + (ad + bc)i
	for(int i = 0; i < m.rows; i++) // loop through y
	{
    const double* m_i = m.ptr<double>(i);   // Input 1
    double* o_i = output.ptr<double>(i);      // Output
    for(int j = 0; j < m.cols; j++)
    {
        o_i[j*2] = scaler * m_i[j*2]; // Real
        o_i[j*2+1] = scaler * m_i[j*2+1]; // Real
    }
	}
}

void complexDivide(const cv::Mat& m1, const cv::Mat& m2, cv::Mat& output)
{
   if (output.empty())
   	output = cv::Mat::zeros(m1.rows, m1.cols, CV_64FC2);
   // (a+bi) / (c+di) = (ac+bd) / (c^2+d^2) + (bc-ad) / (c^2+d^2) * i
	for(int i = 0; i < m1.rows; i++) // loop through y
	{
    const double* m1_i = m1.ptr<double>(i);   // Input 1
    const double* m2_i = m2.ptr<double>(i);   // Input 2
    double* o_i = output.ptr<double>(i);      // Output
    for(int j = 0; j < m1.cols; j++)
    {
        o_i[j*2] = ((m1_i[j*2] * m2_i[j*2]) + (m1_i[j*2+1] * m2_i[j*2+1])) / (m2_i[j*2] * m2_i[j*2] + m2_i[j*2+1] * m2_i[j*2+1]); // Real
        o_i[j*2+1] = ((m1_i[j*2+1] * m2_i[j*2]) - (m1_i[j*2] * m2_i[j*2+1])) / (m2_i[j*2] * m2_i[j*2] + m2_i[j*2+1] * m2_i[j*2+1]);  // Imaginary
    }
	}
}

void complexInverse(const cv::Mat& m1, const cv::Mat& m2, cv::Mat& output)
{
   if (output.empty())
   	output = cv::Mat::zeros(m1.rows, m1.cols, CV_64FC2);
   // (a+bi) / (c+di) = (ac+bd) / (c^2+d^2) + (bc-ad) / (c^2+d^2) * i
	for(int i = 0; i < m1.rows; i++) // loop through y
	{
    //const double* m1_i = m1.ptr<double>(i);   // Input 1
    const double* m2_i = m2.ptr<double>(i);   // Input 2
    double* o_i = output.ptr<double>(i);      // Output
    for(int j = 0; j < m1.cols; j++)
    {
        o_i[j*2] = ((m2_i[j*2]) + ( m2_i[j*2+1])) / (m2_i[j*2] * m2_i[j*2] + m2_i[j*2+1] * m2_i[j*2+1]); // Real
        o_i[j*2+1] = (( m2_i[j*2]) - (m2_i[j*2+1])) / (m2_i[j*2] * m2_i[j*2] + m2_i[j*2+1] * m2_i[j*2+1]);  // Imaginary
    }
	}
}

// Depreciated fftshift methods

/* 
cv::Mat fftShift(cv::Mat m)
{
      cv::Mat shifted = cv::Mat(m.cols,m.rows,m.type());
      circularShift(m, shifted, std::ceil((double) m.cols/2), std::ceil((double) m.rows/2));
      return shifted;
}

cv::Mat ifftShift(cv::Mat m)
{
      cv::Mat shifted = cv::Mat(m.cols,m.rows,m.type());
      circularShift(m, shifted, std::floor((double) m.cols/2), std::floor((double)m.rows/2));
      return shifted;
}
*/ //end depreciation

void fftShift(const cv::Mat& input, cv::Mat& output)
{
	if ((input.data == output.data) || output.empty())
	{
		cv::Mat shifted = cv::Mat::zeros(input.cols, input.rows, input.type());
		circularShift(input, shifted, std::ceil((double) input.cols/2), std::ceil((double) input.rows/2));
		output = shifted.clone();
	}
	else
	 	circularShift(input, output, std::ceil((double) input.cols/2), std::ceil((double) input.rows/2));
}

void ifftShift(const cv::Mat& input, cv::Mat& output)
{
      if ((input.data == output.data) || output.empty())
      {
      	cv::Mat shifted = cv::Mat::zeros(input.cols, input.rows, input.type());
         circularShift(input, shifted, std::floor((double) input.cols/2), std::floor((double) input.rows/2));
         output = shifted.clone();
      }
      else
       	circularShift(input, output, std::floor((double) input.cols/2), std::floor((double) input.rows/2));  
}
/*
void ifftShift2(const cv::Mat& input, cv::Mat& output)
{
	circularShift(input, output, std::floor((double) input.cols/2), std::floor((double) input.rows/2)); 
	showComplexImg(output,-1,"ifftshift2 Result");
}
*/
// Opencv fft implimentation
void fft2(cv::Mat& input, cv::Mat& output)
{
   cv::Mat paddedInput;
   int m = cv::getOptimalDFTSize( input.rows );
   int n = cv::getOptimalDFTSize( input.cols ); 
   
   // Zero pad for Speed
   cv::copyMakeBorder(input, paddedInput, 0, m - input.rows, 0, n - input.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
   cv::dft(paddedInput, output, cv::DFT_COMPLEX_OUTPUT);
}
 
// Opencv ifft implimentation
void ifft2(cv::Mat& input, cv::Mat& output)
{
   cv::Mat paddedInput;
   int m = cv::getOptimalDFTSize( input.rows );
   int n = cv::getOptimalDFTSize( input.cols ); 
   
   // Zero pad for speed
   cv::copyMakeBorder(input, paddedInput, 0, m - input.rows, 0, n - input.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
   cv::dft(paddedInput, output, cv::DFT_INVERSE | cv::DFT_COMPLEX_OUTPUT | cv::DFT_SCALE); // Real-space of object
}

void complex_imwrite(std::string fname, cv::Mat& m1)
{

//   cv::Mat planes[] = {cv::Mat::zeros(m1.rows, m1.cols, m1.type()),
//        cv::Mat::zeros(m1.rows, m1.cols, m1.type())};
//
//   cv::split(m1,planes);

   cv::Mat real_m;
   cv::Mat imaginary_m;
   cv::Mat real_scaled;
   cv::Mat imag_scaled;

   complexAbs2(m1, real_m);
   complexAngle(m1, imaginary_m);

   cv::normalize(real_m, real_scaled, 0,255, CV_MINMAX);
   real_scaled.convertTo(real_scaled, CV_8U);

   cv::normalize(imaginary_m, imag_scaled, 0,255, CV_MINMAX);
   imag_scaled.convertTo(imag_scaled, CV_8U);

   cv::imwrite("/storage/emulated/0/CellScope/FPMTest/real_complexNa_010.jpg", real_scaled);
   cv::imwrite("/storage/emulated/0/CellScope/FPMTest/imag_complexNa_010.jpg", imag_scaled);

   //outputPlanes[0] = inputPlanes[0];
   //outputPlanes[1] = inputPlanes[1];
   //cv::Mat outMat;
   //cv::merge(outputPlanes,3,outMat);
   //cv::imwrite(fname,outMat);
}

void onMouse( int event, int x, int y, int, void* param )
{

	cv::Mat* imgPtr = (cv::Mat*) param;
	cv::Mat image;
	imgPtr->copyTo(image);
	image.convertTo(image,CV_64F); // to keep types consitant

	// Split image into channels
	cv::Mat planes[image.channels()];
	for (int16_t ch=0; ch < image.channels(); ch++)
		planes[ch] = cv::Mat(image.rows, image.cols, image.type());
	split(image,planes);
	
	switch (event)
	{
		case ::CV_EVENT_LBUTTONDOWN:
		{
			// Pretty printing of complex matricies
			if (image.channels() ==2)
			{
				std::printf("x:%d y:%d: \n", x, y);
				std::printf("  %.4f + %.4fi\n",
				planes[0].at<double>(y,x), 
				planes[1].at<double>(y,x));
			}
			else //All other color channels
			{
				std::printf("x:%d y:%d: \n", x, y);
				for (int16_t ch=0; ch < image.channels(); ch++)
				{
					std::printf("  Channel %d: %.4f \n",ch+1,planes[ch].at<double>(y,x));
				}
			}
			std::cout<<std::endl;
			break;
		}
		case ::CV_EVENT_RBUTTONDOWN:
		{ 
			double minVal, maxVal;
			std::printf("x:%d y:%d: \n", x, y);
			for (int16_t ch=0; ch < image.channels(); ch++)
			{
				cv::minMaxLoc(planes[ch], &minVal, &maxVal);
				std::printf("  Channel %d: min: %.4f, max: %.4f \n",ch+1,minVal,maxVal);
			}
			std::cout<<std::endl;
			break;
		}
	default:
		return;
	}
}

void showComplexImg(cv::Mat m, int16_t displayFlag, std::string windowTitle)
{
   if (m.channels() == 2) // Ensure Complex Matrix
   {
		cv::Mat planes[] = {cv::Mat::zeros(m.rows, m.cols, m.type()), cv::Mat::zeros(m.rows, m.cols, m.type())};
		cv::split(m, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
		
		switch(displayFlag)
		{
			case (SHOW_COMPLEX_MAG):
			{
				cv::magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
            std::string windowTitle = windowTitle + " Magnitude";
            showImg(planes[0], windowTitle);
				break;
			}
			case (SHOW_COMPLEX_REAL):
			{
			   std::string reWindowTitle = windowTitle + " Real";
            showImg(planes[0], reWindowTitle);
				break;
			}
			case (SHOW_COMPLEX_IMAGINARY):
			{
				std::string imWindowTitle = windowTitle + " Imaginary";
            showImg(planes[1], imWindowTitle);
				break;
			}
			default:
			{

				std::string reWindowTitle = windowTitle + " Real";
				std::string imWindowTitle = windowTitle + " Imaginary";
				
				showImg(planes[0], reWindowTitle);
				showImg(planes[1], imWindowTitle);
				
				break;
			}
		}
		cv::waitKey();
		cv::destroyAllWindows();
	}
	else
		std::cout << "ERROR ( cvComplex::shotComplexImg ) : Input Mat is not complex (m.channels() != 2)" << std::endl;	
}

void printMat(cv::Mat m, std::string title)
{
	std::cout << "cv::Mat " << title<<" properties:"<<std::endl;
	std::cout << "  sz: " <<m.cols<< " x " <<m.rows<<std::endl;
	std::cout << "  depth: "<<m.depth()<<", channels: " <<m.channels()<<std::endl;
	std::cout << "  type: "<<m.type()<<std::endl;
}

void showImg(cv::Mat m, std::string windowTitle)
{
   cv::Mat scaledMat, displayMat;
   /*
	cv::normalize(m, scaledMat, 0,255, CV_MINMAX);
	scaledMat.convertTo(scaledMat, CV_8U);
	
	if (gv_cMap >= 0)
		cv::applyColorMap(scaledMat, displayMat, gv_cMap);
   else
   	displayMat = scaledMat;
   	*/
   	
   cv::namedWindow(windowTitle, cv::WINDOW_NORMAL);
	cv::setMouseCallback(windowTitle, onMouse, &m);;
	cv::imshow(windowTitle, displayMat);
		
	cv::waitKey();
	cv::destroyAllWindows();	
}
/*
    COLORMAP_AUTUMN = 0,
    COLORMAP_BONE = 1,
    COLORMAP_JET = 2,
    COLORMAP_WINTER = 3,
    COLORMAP_RAINBOW = 4,
    COLORMAP_OCEAN = 5,
    COLORMAP_SUMMER = 6,
    COLORMAP_SPRING = 7,
    COLORMAP_COOL = 8,
    COLORMAP_HSV = 9,
    COLORMAP_PINK = 10,
    COLORMAP_HOT = 11
    */
void setColorMap(int16_t cMap)
{
	if (cMap >= CMAP_MIN && cMap <= CMAP_MAX)
		gv_cMap = cMap;
	else
		std::cout << "ERROR ( cvComplex::setColorMap )  : Invalid Color Map (Valid Values are between " << CMAP_MIN <<" and " << CMAP_MAX << std::endl;
}
