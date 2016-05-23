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
//#include "opencv2/contrib/contrib.hpp"
#include "cvComplex.h"
#include <stdio.h>
#include <cmath>

int16_t gv_cMap = -1; // Global Colormap Setting

const int16_t SHOW_COMPLEX_MAG = 0;
const int16_t SHOW_COMPLEX_COMPONENTS = 1;
const int16_t SHOW_COMPLEX_REAL = 2;
const int16_t SHOW_COMPLEX_IMAGINARY = 3;
const int16_t SHOW_AMP_PHASE = 4;

const int16_t RW_MODE_AMP_PHASE = 1;
const int16_t RW_MODE_REAL_IMAG = 2;

const int16_t CMAP_MIN = 0;
const int16_t CMAP_MAX = 11;
const int16_t COLORMAP_NONE = -1;

const int16_t COLORIMAGE_REAL = 0;
const int16_t COLORIMAGE_COMPLEX = 1;

using namespace cv;
using namespace std;

void circularShift(const cv::Mat& input, cv::Mat& output, int16_t x, int16_t y)
{
    if (output.empty())
        output = cv::Mat::zeros(input.rows, input.cols, input.type());

    int16_t w = input.cols;
    int16_t h = input.rows;

    int16_t shiftR = x % w;
    int16_t shiftD = y % h;

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

    // Generate pointers
    cv::Mat shift1, shift2, shift3, shift4;
    if (input.data==output.data)
    {
        shift1 = input(gate1).clone();
        shift2 = input(gate2).clone();
        shift3 = input(gate3).clone();
        shift4 = input(gate4).clone();
    }
    else
    {
        shift1 = input(gate1);
        shift2 = input(gate2);
        shift3 = input(gate3);
        shift4 = input(gate4);
    }

    // Copy to result
    shift1.copyTo(cv::Mat(output, out1));
    shift2.copyTo(cv::Mat(output, out2));
    shift3.copyTo(cv::Mat(output, out3));
    shift4.copyTo(cv::Mat(output, out4));
}

void maxComplexReal(cv::Mat& m, std::string label)
{
    cv::Mat planes[] = {cv::Mat::zeros(m.rows, m.cols, m.type()), cv::Mat::zeros(m.rows, m.cols, m.type())};
    cv::split(m,planes);
    double minVal, maxVal;
    cv::minMaxLoc(planes[0], &minVal, &maxVal);
    std::cout << "Max/Min values of " <<label << " are: " << maxVal << ", " << minVal << std::endl;
}

/*
void complexConj(const cv::Mat& input, cv::Mat& output)
{
  if (output.empty())
    output = cv::Mat::zeros(input.rows, input.cols, CV_64FC2);

  double real, imag;
	for(int i = 0; i < input.rows; i++) // loop through y
	{
    const double* m_i = input.ptr<double>(i);  // Input
    double* o_i = output.ptr<double>(i);   // Output

    for(int j = 0; j < input.cols; j++)
    {
        real = (double) m_i[j*2];
        imag = (double) -1.0 * m_i[j*2+1];

        o_i[j*2] = real;
        o_i[j*2+1] = imag;
    }
	}
}*/

void complexConj(const cv::Mat& input, cv::Mat& output)
{
    if (output.empty())
        output = cv::Mat::zeros(input.rows, input.cols, CV_64FC2);
    output = input.mul(cv::Scalar(1,-1));
}


void complexAngle(const cv::Mat& input, cv::Mat& output)
{
    if (output.empty())
        output = cv::Mat::zeros(input.rows, input.cols, CV_64FC2);

    for(int i = 0; i < input.rows; i++) // loop through y
    {
        const double* m_i = input.ptr<double>(i);  // Input
        double* o_i = output.ptr<double>(i);   // Output

        for(int j = 0; j < input.cols; j++)
        {
            o_i[2*j] = (double) atan2(m_i[j*2+1],m_i[j*2]);
            o_i[j*2+1] = 0.0;
        }
    }
}

void complexAbs(const cv::Mat& input, cv::Mat& output)
{
    // Ensure output is not empty
    if (output.empty())
        output = cv::Mat::zeros(input.rows, input.cols, CV_64FC2);

    for(int i = 0; i < input.rows; i++) // loop through y
    {
        const double* m_i = input.ptr<double>(i);  // Input
        double* o_i = output.ptr<double>(i);   // Output

        for(int j = 0; j < input.cols; j++)
        {
            o_i[j*2] = (double) std::sqrt(m_i[j*2] * m_i[j*2] + m_i[j*2+1] * m_i[j*2+1]);
            o_i[j*2+1] = 0.0;
        }
    }
}

void complexAmpPhaseToRealImag(const cv::Mat& input, cv::Mat& output)
{

    if (output.empty())
        output = cv::Mat::zeros(input.rows, input.cols, CV_64FC2);

    double real,imag;
    for(int i = 0; i < input.rows; i++) // loop through y
    {
        const double* m_i = input.ptr<double>(i);  // Input
        double* o_i = output.ptr<double>(i);   // Output

        for(int j = 0; j < input.cols; j++)
        {
            real = (double)m_i[j*2]*sin((double)m_i[j*2+1]);
            imag = (double)m_i[j*2]*cos((double)m_i[j*2+1]);
            o_i[j*2]  = real; o_i[j*2+1] = imag;
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
 *   cv::Mat& output:    Complex Product of m1 and m2
 */
void complexMultiply(const cv::Mat& input1, const cv::Mat& input2, cv::Mat& output)
{
    // Check if matricies are of same size and type
    if (!((input1.size() == input2.size()) && (input1.type() == input2.type())))
    {
        std::cout << "ERROR - matricies are of different size!" << std::endl;
        return;
    }
    // Ensure output is not empty
    if (output.empty())
        output = cv::Mat::zeros(input1.rows, input1.cols, CV_64FC2);

    // (a + bi) * (c + di) = (ac - bd) + (ad + bc)i
    double real,imag;
    for(int i = 0; i < input1.rows; i++) // loop through y
    {
        const double* m1_i = input1.ptr<double>(i);   // Input 1
        const double* m2_i = input2.ptr<double>(i);   // Input 2
        double* o_i = output.ptr<double>(i);      // Output
        for(int j = 0; j < input1.cols; j++)
        {
            real = (m1_i[j*2] * m2_i[j*2]) - (m1_i[j*2+1] * m2_i[j*2+1]);    // Real
            imag = (m1_i[j*2] * m2_i[j*2+1]) + (m1_i[j*2+1] * m2_i[j*2]);  // Imaginary
            o_i[j*2]  = real; o_i[j*2+1] = imag;
        }
    }
}

/* complexScalarMultiply(const cv::Mat& m1, const cv::Mat& m2, cv::Mat& output)
 * Multiplies the real and imaginary parts of a complex matrix by a scalar.
 *
 * INPUTS:
 *   double scalar:      Scalar to multiply
 *   const cv::Mat& m1:  Complex matrix input
 * OUTPUT:
 *   cv::Mat& output:    Complex product of m1 and scalar
 */
void complexScalarMultiply(std::complex<double> scaler, cv::Mat& input, cv::Mat output)
{
if (output.empty())
output = cv::Mat::zeros(input.rows, input.cols, CV_64FC2);

// (a + bi) * (c + di) = (ac - bd) + (ad + bc)i
double real, imag;
for(int i = 0; i < input.rows; i++) // loop through y
{
const double* m_i = input.ptr<double>(i);   // Input 1
double* o_i = output.ptr<double>(i);      // Output
for(int j = 0; j < input.cols; j++)
{
real = scaler.real() * m_i[j*2] - scaler.imag() * m_i[j*2+1]; // Real
imag= scaler.imag() * m_i[j*2] + scaler.real() * m_i[j*2+1]; // Imaginary
o_i[j*2]  = real; o_i[j*2+1] = imag;
}
}
}

/* complexDivide(const cv::Mat& m1, const cv::Mat& m2, cv::Mat& output)
 * Divides one matrix by another where the first two color channels are the
 * real and imaginary coefficents, respectivly. Uses the equation:
 *   (a+bi) / (c+di) = (ac+bd) / (c^2+d^2) + (bc-ad) / (c^2+d^2) * i
 *
 * INPUTS:
 *   const cv::Mat& m1:  Complex Matrix 1
 *   const cv::Mat& m2:  Complex Matrix 2
 * OUTPUT:
 *   cv::Mat& output:    Complex Product of m1 and m2
 */
void complexDivide(const cv::Mat& input1, const cv::Mat& input2, cv::Mat& output)
{
    // Check if matricies are of same size and type
    if (!((input1.size() == input2.size()) && (input1.type() == input2.type())))
    {
        std::cout << "ERROR - matricies are of different size!" << std::endl;
        return;
    }

    // Ensure output is not empty
    if (output.empty())
        output = cv::Mat::zeros(input1.rows, input1.cols, CV_64FC2);

    // (a+bi) / (c+di) = (ac+bd) / (c^2+d^2) + (bc-ad) / (c^2+d^2) * i
    double real, imag;
    for(int i = 0; i < input1.rows; i++) // loop through y
    {
        const double* m1_i = input1.ptr<double>(i);   // Input 1
        const double* m2_i = input2.ptr<double>(i);   // Input 2
        double* o_i = output.ptr<double>(i);      // Output
        for(int j = 0; j < input1.cols; j++)
        {
            real = ((m1_i[j*2] * m2_i[j*2]) + (m1_i[j*2+1] * m2_i[j*2+1])) / (m2_i[j*2] * m2_i[j*2] + m2_i[j*2+1] * m2_i[j*2+1]); // Real
            imag = ((m1_i[j*2+1] * m2_i[j*2]) - (m1_i[j*2] * m2_i[j*2+1])) / (m2_i[j*2] * m2_i[j*2] + m2_i[j*2+1] * m2_i[j*2+1]);  // Imaginary
            o_i[j*2]  = real; o_i[j*2+1] = imag;
        }
    }
}

void complexInverse(const cv::Mat& input, cv::Mat& output)
{

    if (output.empty())
        output = cv::Mat::zeros(input.rows, input.cols, CV_64FC2);

    // (a+bi) / (c+di) = (ac+bd) / (c^2+d^2) + (bc-ad) / (c^2+d^2) * i
    double real, imag;
    for(int i = 0; i < input.rows; i++) // loop through y
    {
        //const double* m1_i = m1.ptr<double>(i);   // Input 1
        const double* m2_i = input.ptr<double>(i);   // Input 2
        double* o_i = output.ptr<double>(i);      // Output
        for(int j = 0; j < input.cols; j++)
        {
            real = ((m2_i[j*2]) + ( m2_i[j*2+1])) / (m2_i[j*2] * m2_i[j*2] + m2_i[j*2+1] * m2_i[j*2+1]); // Real
            imag = (( m2_i[j*2]) - (m2_i[j*2+1])) / (m2_i[j*2] * m2_i[j*2] + m2_i[j*2+1] * m2_i[j*2+1]);  // Imaginary
            o_i[j*2]  = real; o_i[j*2+1] = imag;
        }
    }
}

void fftShift(const cv::Mat& input, cv::Mat& output)
{
    circularShift(input, output, std::ceil((double) input.cols/2), std::ceil((double) input.rows/2));
}

void ifftShift(const cv::Mat& input, cv::Mat& output)
{
    circularShift(input, output, std::floor((double) input.cols/2), std::ceil((double) input.rows/2));
}
// Opencv fft implimentation
void fft2(cv::Mat& input, cv::Mat& output)
{
    //cv::Mat paddedInput;
    //int m = cv::getOptimalDFTSize( input.rows );
    //int n = cv::getOptimalDFTSize( input.cols );

    // Zero pad for Speed
    //cv::copyMakeBorder(input, paddedInput, 0, m - input.rows, 0, n - input.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
    cv::dft(input, output, cv::DFT_COMPLEX_OUTPUT);
}

// Inverse Fourier Transform
void ifft2(cv::Mat& input, cv::Mat& output)
{
    //cv::Mat paddedInput;
    //int m = cv::getOptimalDFTSize( input.rows );
    //int n = cv::getOptimalDFTSize( input.cols );

    // Zero pad for speed
    //cv::copyMakeBorder(input, paddedInput, 0, m - input.rows, 0, n - input.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
    cv::dft(input, output, cv::DFT_INVERSE | cv::DFT_COMPLEX_OUTPUT | cv::DFT_SCALE); // Real-space of object
}

// Write complex matrix to file
void complex_imwrite(cv::Mat& m1,std::string fname, int16_t rwMode)
{
    cv::Mat complexPlanes[] = {cv::Mat::zeros(m1.rows, m1.cols, m1.type()), cv::Mat::zeros(m1.rows, m1.cols, m1.type())};
    std::string typeStr1;
    std::string typeStr2;
    if (rwMode & RW_MODE_AMP_PHASE)
    {
        complexAbs(m1,complexPlanes[0]);
        complexAngle(m1,complexPlanes[1]);
        typeStr1 = "Amp"; typeStr2 = "Phase";
    }
    else if (rwMode & RW_MODE_REAL_IMAG)
    {
        cv::split(m1,complexPlanes);
        typeStr1 = "Real"; typeStr2 = "Imag";
    }
    cv::imwrite(fname+'_'+typeStr1+".tiff",complexPlanes[0]);
    cv::imwrite(fname+'_'+typeStr2+".tiff",complexPlanes[1]);
}

// Read a complex image from two matricies
void complex_imread(std::string fNameAmp, std::string fNamePhase, cv::Mat& output, int16_t rwMode)
{
    Mat m1 = cv::imread(fNameAmp,-1*CV_LOAD_IMAGE_ANYDEPTH); // Any depth, any type
    Mat m2 = cv::imread(fNamePhase,-1*CV_LOAD_IMAGE_ANYDEPTH); // Any depth, any type
    if (m1.rows ==0 || m2.rows==0)
    {
        std::cout << "ERROR - images not found!"<<std::endl;
        return;
    }
    cv::Mat complexPlanes[] = {cv::Mat::zeros(m1.rows, m1.cols, m1.type()), cv::Mat::zeros(m1.rows, m1.cols, m1.type())};
    complexPlanes[0] = m1; complexPlanes[1] = m2;
    cv::merge(complexPlanes,2,output);
    Mat tmpMat = Mat::zeros(output.rows,output.cols,output.type());
    if (rwMode & RW_MODE_AMP_PHASE)
    {
        complexAmpPhaseToRealImag(output,tmpMat);
        tmpMat.copyTo(output);
    }


}
// Mouse callback for showImg
void onMouse( int event, int x, int y, int, void* param )
{

    cv::Mat* imgPtr = (cv::Mat*) param;
    cv::Mat image;
    imgPtr->copyTo(image);
    image.convertTo(image,CV_64F); // to keep types consitant

    // Split image into channels
    cv::Mat *  planes= new cv::Mat[image.channels()];
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

// Compatability with previous versions
void showComplexImg(cv::Mat m, int16_t displayFlag, std::string windowTitle)
{
    showComplexImg(m, displayFlag, windowTitle,-1);
}

void showImg(cv::Mat m, std::string windowTitle)
{
    showImg(m,windowTitle,-1);
}

// Display a complex image
void showComplexImg(cv::Mat m, int16_t displayFlag, std::string windowTitle, int16_t gv_cMap)
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
                windowTitle = windowTitle + " Magnitude";
                //cv::log(planes[0],planes[0]);
                showImg(planes[0], windowTitle, gv_cMap);
                break;
            }
            case (SHOW_COMPLEX_REAL):
            {
                std::string reWindowTitle = windowTitle + " Real";
                showImg(planes[0], reWindowTitle, gv_cMap);
                break;
            }
            case (SHOW_COMPLEX_IMAGINARY):
            {
                std::string imWindowTitle = windowTitle + " Imaginary";
                showImg(planes[1], imWindowTitle, gv_cMap);
                break;
            }
            case (SHOW_AMP_PHASE):
            {
                Mat m2;
                cv::Mat planes2[] = {cv::Mat::zeros(m.rows, m.cols, m.type()), cv::Mat::zeros(m.rows, m.cols, m.type())};
                complexAngle(m,m2);
                cv::split(m2, planes2);

                std::string amWindowTitle = windowTitle + " Amplitude";
                std::string phWindowTitle = windowTitle + " Phase";
                complexAbs(m,m);
                cv::split(m, planes);

                showImg(planes[0], amWindowTitle, gv_cMap);
                showImg(planes2[0], phWindowTitle, gv_cMap);
                break;
            }
            default:
            {

                std::string reWindowTitle = windowTitle + " Real";
                std::string imWindowTitle = windowTitle + " Imaginary";

                showImg(planes[0], reWindowTitle, gv_cMap);
                showImg(planes[1], imWindowTitle, gv_cMap);

                break;
            }
        }
        //cv::waitKey();
        //cv::destroyAllWindows();
    }
    else
        std::cout << "ERROR ( cvComplex::shotComplexImg ) : Input Mat is not complex (m.channels() != 2)" << std::endl;
}

// Print information about a matrix
void printMat(cv::Mat m, std::string title)
{
    std::cout << "cv::Mat " << title<<" properties:"<<std::endl;
    std::cout << "  sz: " <<m.cols<< " x " <<m.rows<<std::endl;
    std::cout << "  depth: "<<m.depth()<<", channels: " <<m.channels()<<std::endl;
    std::cout << "  type: "<<m.type()<<std::endl;
}

// Show a single-channel image
void showImg(cv::Mat m, std::string windowTitle, int16_t gv_cMap)
{

    cv::Mat displayMat;

    if (gv_cMap >= cv::COLORMAP_AUTUMN && gv_cMap <= cv::COLORMAP_HOT)
    {
        cv::Mat scaledMat;
        cv::normalize(m, scaledMat, 0, 255, CV_MINMAX);
        scaledMat.convertTo(scaledMat, CV_8U);
        cv::applyColorMap(scaledMat, displayMat, gv_cMap);
    }
    else
    {
        cv::Mat scaledImg;
        cv::normalize(m, scaledImg, 0, 1, CV_MINMAX);
        scaledImg.convertTo(scaledImg,CV_32FC1);
        cvtColor(scaledImg, displayMat, CV_GRAY2RGB);
    }

    cv::startWindowThread();
    cv::namedWindow(windowTitle, cv::WINDOW_NORMAL);
    cv::setMouseCallback(windowTitle, onMouse, &m);;
    cv::imshow(windowTitle, displayMat);
    cv::waitKey();
    cv::destroyAllWindows();

}

void showImgC(cv::Mat* ImgC, std::string windowTitle, int16_t REAL_COMPLEX)
{
    cv::Mat displayMat, ImgCC;

    if(REAL_COMPLEX == COLORIMAGE_REAL)
    {
        cv::merge(ImgC,3,ImgCC);
    }
    else
    {
        cv::Mat ImgCC_buff[] = {cv::Mat::zeros(ImgC[0].rows, ImgC[0].cols, CV_64FC1),
                                cv::Mat::zeros(ImgC[0].rows, ImgC[0].cols, CV_64FC1),
                                cv::Mat::zeros(ImgC[0].rows, ImgC[0].cols, CV_64FC1)};
        cv::Mat ImgCC_buff2[] = {cv::Mat::zeros(ImgC[0].rows, ImgC[0].cols, CV_64FC1),
                                 cv::Mat::zeros(ImgC[0].rows, ImgC[0].cols, CV_64FC1)};

        cv::split(ImgC[0],ImgCC_buff2);
        ImgCC_buff[0] = ImgCC_buff2[0].clone();
        cv::split(ImgC[1],ImgCC_buff2);
        ImgCC_buff[1] = ImgCC_buff2[0].clone();
        cv::split(ImgC[2],ImgCC_buff2);
        ImgCC_buff[2] = ImgCC_buff2[0].clone();
        cv::merge(ImgCC_buff,3,ImgCC);
    }

    cv::normalize(ImgCC, displayMat, 0, 1, CV_MINMAX);
    displayMat.convertTo(displayMat,CV_64FC3);

    cv::startWindowThread();
    cv::namedWindow(windowTitle, cv::WINDOW_NORMAL);
    cv::setMouseCallback(windowTitle, onMouse, ImgC);;
    cv::imshow(windowTitle, displayMat);
    cv::waitKey();
    cv::destroyAllWindows();

}

/*void setColorMap(int16_t cMap)
{
	if (cMap >= CMAP_MIN && cMap <= CMAP_MAX)
		gv_cMap = cMap;
	else
		std::cout << "ERROR ( cvComplex::setColorMap )  : Invalid Color Map (Valid Values are between " << CMAP_MIN <<" and " << CMAP_MAX << std::endl;
}*/
