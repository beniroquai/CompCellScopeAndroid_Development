/*
fpmMain.cpp
*/

#include <jni.h>
#include <time.h>
///#include <opencv2/contrib/contrib.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <string>
#include <stdio.h>
#include <dirent.h>
#include <fstream>
#include <vector>
#include <android/log.h>
#include <cmath>

#include "cvComplex.h"
#include "fpmMain.h"
#include "json.h"

//#include "include/rapidjson"
#include "domeHoleCoordinates.h"

using namespace std;
using namespace cv;

#define FILENAME_LENGTH 129
#define FILE_HOLENUM_DIGITS 4

string filePrefix = "iLED_";


extern "C" {


int computeFPM(int iterations);

// Debug flags
bool runDebug = false;
bool loadImgDebug = false;




int16_t loadFPMDataset(FPM_Dataset *dataset) {

  DIR *dir;
  struct dirent *ent;
  Mat fullImg;
  Mat fullImgComplex;
  FPMimg tmpFPMimg;
  tmpFPMimg.Image = Mat::zeros(dataset->Np, dataset->Np, CV_8UC1);

  //cout << rotationMatrixZ <<endl;
  clock_t t1, t2;
  if (loadImgDebug) {
    t1 = clock();
  }

  // Initialize array of image objects, since we don't know exactly what order
  // imread will read in images. First (0th) element is a dummy so we can access
  // these using the led # directly
  for (int16_t ledIdx = 0; ledIdx <= dataset->ledCount; ledIdx++) {
    dataset->imageStack.push_back(tmpFPMimg);
    dataset->illuminationNAList.push_back(99.0);
    dataset->NALedPatternStackX.push_back(-1.0);
    dataset->NALedPatternStackY.push_back(-1.0);
  }

  // Generate Rotation Matrix
  double angle = dataset->arrayRotation;

//  Mat rotationMatrixZ = (Mat_<double>(3,3) << cos (angle*M_PI/180), -sin (angle*M_PI/180), 0,sin (angle*M_PI/180), cos (angle*M_PI/180), 0, 0, 0, 1);
  Mat rotationMatrixZ = (Mat_<double>(3,3) << cos (angle), -sin (angle), 0,sin (angle), cos (angle), 0, 0, 0, 1);

  __android_log_print(ANDROID_LOG_INFO, "FPMPP", "directory is %s:", dataset->datasetRoot.c_str());
  if ((dir = opendir(dataset->datasetRoot.c_str())) != NULL) {
    int16_t num_images = 0;
    int imageCounter = 0;
    std::cout << "Loading Images..." << std::endl;
    ent = readdir(dir);
    __android_log_print(ANDROID_LOG_INFO, "FPMPP", "filename is %s:", ent->d_name);
    while ((ent = readdir(dir)) != NULL) {
     string fileName = ent->d_name;
     __android_log_print(ANDROID_LOG_INFO, "FPMPP", "filename inside the whileloop is %s", fileName.c_str());
     __android_log_print(ANDROID_LOG_INFO, "FPMPP", "strcmp1 is %d", strcmp(dataset->fileExtension.c_str(),
                                                                            &(ent->d_name[strlen(ent->d_name) -
                                                                                          dataset->fileExtension.length()])));
     __android_log_print(ANDROID_LOG_INFO, "FPMPP", "prefix is %s", (dataset->filePrefix).c_str());
     __android_log_print(ANDROID_LOG_INFO, "FPMPP", "strcmp 2 is %d", fileName.find(dataset->filePrefix));


      /* Get data from file name, if name is right format.*/
      if (fileName.compare(".") != 0 && fileName.compare("..") != 0 &&
          (strcmp(dataset->fileExtension.c_str(),
                  &(ent->d_name[strlen(ent->d_name) -
                                dataset->fileExtension.length()])) == 0) &&(
                              fileName.find(dataset->filePrefix) == 0))
                              {
        string holeNum = fileName.substr(fileName.find(dataset->filePrefix) +
                                             dataset->filePrefix.length(),
                                         FILE_HOLENUM_DIGITS);
        __android_log_print(ANDROID_LOG_INFO, "FPMPP", "holenum 1 is %s", holeNum.c_str());

       FPMimg currentImage;
       currentImage.led_num = atoi(holeNum.c_str());

       Mat holeCoordinatesIn = (Mat_<double>(1,3) << domeHoleCoordinates[currentImage.led_num - 1][0], domeHoleCoordinates[currentImage.led_num - 1][1], domeHoleCoordinates[currentImage.led_num - 1][2]);
       Mat holeCoordinates = holeCoordinatesIn * rotationMatrixZ;

       // Flip coordinates if desired
       Mat flipMat = (Mat_<double>(1,3) << 1, 1, 1);
       if (dataset->flipIlluminationX)
         flipMat = (Mat_<double>(1,3) << -1, 1, 1);
       if (dataset->flipIlluminationY)
         flipMat = (Mat_<double>(1,3) << -1, 1, 1);
       holeCoordinates = holeCoordinates.mul(flipMat);


       currentImage.sinTheta_x = sin(atan2(holeCoordinates.at<double>(0, 0),
                     holeCoordinates.at<double>(0, 2)));

       currentImage.sinTheta_y = sin(atan2(holeCoordinates.at<double>(0, 1),
                 holeCoordinates.at<double>(0, 2)));

       //std::cout << "SinThetaX: " << currentImage.sinTheta_x <<", SinThetaY: " << currentImage.sinTheta_y <<std::endl;

       currentImage.illumination_na =
                     sqrt(currentImage.sinTheta_x * currentImage.sinTheta_x +
                          currentImage.sinTheta_y * currentImage.sinTheta_y);
        std::cout <<"NA:"<<sqrt(currentImage.sinTheta_x * currentImage.sinTheta_x + currentImage.sinTheta_y * currentImage.sinTheta_y)<<endl;

      __android_log_print(ANDROID_LOG_INFO, "FPMPP", "curr ill na: %f, max ill na: %f", currentImage.illumination_na, dataset->maxIlluminationNA);
         if (sqrt(currentImage.illumination_na<dataset->maxIlluminationNA))
         {

        __android_log_print(ANDROID_LOG_INFO, "FPMPP", "filename is %s:", fileName.c_str());

        fullImg = imread(dataset->datasetRoot + fileName, CV_16UC1);
        __android_log_print(ANDROID_LOG_INFO, "FPMPP", "pix values are %d", fullImg.at<int>(50, 50));
        if (holeNum == "249" || holeNum == "0249"){
            char test[FILENAME_LENGTH];
            snprintf(test,sizeof(test), "/storage/emulated/0/CellScope/FPM_results/test249.tiff");
            imwrite(test, fullImg);
            __android_log_print(ANDROID_LOG_INFO, "FPMPP", "saved test img");
        }


        // Populate fields of FPMImage class
        currentImage.Image = fullImg(cv::Rect(dataset->cropX, dataset->cropY,
            dataset->Np, dataset->Np)).clone();

        currentImage.Image = fullImg(cv::Rect(dataset->cropX, dataset->cropY,
                                              dataset->Np, dataset->Np)).clone();


        // If darkfield, account for exposure multiplication (if any)
        if (dataset->darkfieldExpMultiplier != 1 && sqrt(currentImage.illumination_na > dataset->objectiveNA))
          currentImage.Image = currentImage.Image / dataset->darkfieldExpMultiplier;

        cv::Scalar bk1 = cv::mean(fullImg(cv::Rect(
            dataset->bk1cropX, dataset->bk1cropY, dataset->Np, dataset->Np)));
        cv::Scalar bk2 = cv::mean(fullImg(cv::Rect(
            dataset->bk2cropX, dataset->bk2cropY, dataset->Np, dataset->Np)));

        double bg_val = ((double)bk2[0] + (double)bk1[0]) / 2;
        if (bg_val > dataset->bgThreshold)
          bg_val = dataset->bgThreshold;

        currentImage.bg_val = (int16_t)round(bg_val);

        // Perform Background Subtraction
        cv::subtract(currentImage.Image, cv::Scalar(currentImage.bg_val, 0, 0),
                     currentImage.Image);

        currentImage.uled = currentImage.sinTheta_x / dataset->lambda;
        currentImage.vled = currentImage.sinTheta_y / dataset->lambda;

        // Fourier shift of off-axis led
        currentImage.idx_u = (int16_t)round(currentImage.uled / dataset->du);
        currentImage.idx_v = (int16_t)round(currentImage.vled / dataset->du);

        currentImage.pupilShiftX = currentImage.idx_u;
        currentImage.pupilShiftY = currentImage.idx_v;

        // Region to crop in Fourier Domain
        currentImage.cropXStart = (int16_t)round(dataset->Nlarge / 2) +
                                  currentImage.pupilShiftX -
                                  (int16_t)round(dataset->Ncrop / 2);
        currentImage.cropXEnd = (int16_t)round(dataset->Nlarge / 2) +
                                currentImage.pupilShiftX +
                                (int16_t)round(dataset->Ncrop / 2) - 1;
        currentImage.cropYStart = (int16_t)round(dataset->Mlarge / 2) +
                                  currentImage.pupilShiftY -
                                  (int16_t)round(dataset->Ncrop / 2);
        currentImage.cropYEnd = (int16_t)round(dataset->Mlarge / 2) +
                                currentImage.pupilShiftY +
                                (int16_t)round(dataset->Ncrop / 2) - 1;

        // Assign Image object to FPMDataset class
        dataset->imageStack.at(currentImage.led_num) = currentImage;
        dataset->illuminationNAList.at(currentImage.led_num) =
            currentImage.illumination_na;
        dataset->NALedPatternStackX.at(currentImage.led_num) =
            currentImage.sinTheta_x;
        dataset->NALedPatternStackY.at(currentImage.led_num) =
            currentImage.sinTheta_y;

        __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp6");
        __android_log_print(ANDROID_LOG_INFO, "loading2", "sorted_indices at %d is %f", currentImage.led_num,
        dataset->illuminationNAList.at(currentImage.led_num));

        num_images++;
        std::cout << "Loaded: " << fileName
                  << ", LED # is: " << currentImage.led_num << std::endl;
        loadImgDebug = true;

      }else
        std::cout << "Skipped LED# " << holeNum <<std::endl;
      }
      dataset->ledUsedCount = num_images;
    }
    closedir(dir);
    if (num_images <= 0) {
      std::cout << "ERROR - No images found in given directory." << std::endl;
      return -1;
    }

    // Sort the Images into the correct order
    std::vector<size_t> sorted_indices = sort_indexes(dataset->illuminationNAList);

    for (int i = 0; i != sorted_indices.size(); i++)
        __android_log_print(ANDROID_LOG_INFO, "loading11", "%d: %d", i, sorted_indices[i]);


    int16_t indexIncr = 1;
    for (auto i : sort_indexes(dataset->illuminationNAList)) {
      if (indexIncr <= dataset->ledUsedCount)
      {
        __android_log_print(ANDROID_LOG_INFO, "loading22", "%d is %d", indexIncr, i);
        dataset->sortedIndicies.push_back(i);
        dataset->sortedNALedPatternStackX.push_back(
            dataset->NALedPatternStackX[i]);
        dataset->sortedNALedPatternStackY.push_back(
            dataset->NALedPatternStackY[i]);
        indexIncr++;
      }
    }
    if (loadImgDebug) {
      t2 = clock();
      float diff(((float)t2 - (float)t1) / CLOCKS_PER_SEC);
      cout << "Image loading Completed (Time: " << diff << " sec)" << endl;
      return num_images;
    }
  return 1;
  } else {
    /* could not open directory */
    std::cout << "ERROR: Could not Open Directory.\n";
    return -1;
  }
}

void runFPM(FPM_Dataset *dataset) {

  clock_t t1, t2, t3, t4;
  t3 = clock();
  // Make dummy pointers to save space
  Mat *objF = &dataset->objF;

  // Initilize Matricies
  Mat tmpMat1, tmpMat2, tmpMat3;
  Mat objF_centered;
  Mat complexI, pupilAbs, pupilConj, objfcrop_abs, objfcrop_conj;
  Mat Objfcrop_abs;
  Mat Objfcrop_abs_sq;
  Mat Objf_abs;
  Mat Objfcrop_conj;
  Mat Objfcrop_abs_conj;
  Mat planes[] = {Mat::zeros(dataset->Np, dataset->Np, CV_64F),
                  Mat::zeros(dataset->Np, dataset->Np, CV_64F)};
  Mat objectAmp = Mat::zeros(dataset->Np, dataset->Np, CV_64FC2);
  Mat pupil_abs;
  Mat pupil_abs_sq;
  Mat pupil_conj;
  Mat numerator;
  Mat denomSum;
  double q, pupilMax, p, objf_max, Objf_abs_max;
  FPMimg *currImg;

  __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 100");


  // Initialize pupil function
  planes[0] = Mat::zeros(dataset->Np, dataset->Np, CV_64F);

  __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 101");


  planes[1] = Mat::zeros(dataset->Np, dataset->Np, CV_64F);
  cv::Point center(cvRound(dataset->Np / 2), cvRound(dataset->Np / 2));
  int16_t naRadius = (int16_t)ceil(dataset->objectiveNA * dataset->ps_eff *
                                   dataset->Np / dataset->lambda);
  cv::circle(planes[0], center, naRadius, cv::Scalar(1.0), -1, 8, 0);

  __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 102");


  // FFTshift the pupil so it is consistant with object FT
  fftShift(planes[0], planes[0]);

  merge(planes, 2, dataset->pupil);
  dataset->pupilSupport = dataset->pupil.clone();

  __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 103");


  // Initialize FT of reconstructed object with center led image
  planes[0] =
      Mat_<double>(dataset->imageStack.at(dataset->sortedIndicies.at(1)).Image);
  cv::sqrt(planes[0], planes[0]); // Convert to amplitude
  planes[1] = Mat::zeros(dataset->Np, dataset->Np, CV_64F);
  merge(planes, 2, complexI);

  __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 104");


  fft2(complexI, complexI);
  complexMultiply(complexI, dataset->pupilSupport, complexI);
  fftShift(complexI, complexI); // Shift to center

  __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 105");


  planes[0] = Mat::zeros(dataset->Nlarge, dataset->Mlarge, CV_64F);
  planes[1] = Mat::zeros(dataset->Nlarge, dataset->Mlarge, CV_64F);
  merge(planes, 2, dataset->objF);

  __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 106");


  complexI.copyTo(
      cv::Mat(dataset->objF, cv::Rect((int16_t)round(dataset->Mlarge / 2) -
                                          (int16_t)round(dataset->Ncrop / 2),
                                      (int16_t)round(dataset->Mlarge / 2) -
                                          (int16_t)round(dataset->Ncrop / 2),
                                      dataset->Np, dataset->Np)));

  // Shift to un-fftshifted position
  fftShift(dataset->objF, dataset->objF);

  __android_log_print(ANDROID_LOG_INFO, "FPMPP rotation", "rotation is: %f", dataset->arrayRotation);
  __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 107");


  for (int16_t itr = 1; itr <= dataset->itrCount; itr++)
  {
    __android_log_print(ANDROID_LOG_INFO, "FPMPP2", "itrCount: %d, itr: %d", itr, dataset->itrCount);

    t1 = clock();
    for (int16_t imgIdx = 0; imgIdx < dataset->ledUsedCount; imgIdx++) //
    {
      __android_log_print(ANDROID_LOG_INFO, "FPMPP2", "ledUsedCount: %d, imgIdx: %d", dataset->ledUsedCount, imgIdx);

      __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 108");

      int16_t ledNum = dataset->sortedIndicies.at(imgIdx);

      __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 108.1");
      __android_log_print(ANDROID_LOG_INFO, "lednum", "input for 1 lednum: %d", dataset->sortedIndicies.at(imgIdx));

      __android_log_print(ANDROID_LOG_INFO, "lednum", "input lednum: %d", ledNum);


      if (runDebug)
        cout << "Starting LED# " << ledNum << endl;

      __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 108.2");

      currImg = &dataset->imageStack.at(ledNum);

      __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 108.3");

      __android_log_print(ANDROID_LOG_INFO, "lednum", "currImg lednum: %d", currImg->led_num);

      // Update Fourier space, multply by pupil (P * O)
      fftShift(dataset->objF,
               objF_centered); // Shifted Object spectrum (at center)

      __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 108.4");


      if (currImg->led_num == 249) {
        __android_log_print(ANDROID_LOG_INFO, "FPMPP debug 0", "cropXStart: %d", currImg->cropXStart);
      }

      // CHECK objF_centered dimensions
      __android_log_print(ANDROID_LOG_INFO, "FPMPP", "objF_centered rows: %d", objF_centered.rows);
      __android_log_print(ANDROID_LOG_INFO, "FPMPP", "objF_centered cols: %d", objF_centered.cols);
      __android_log_print(ANDROID_LOG_INFO, "FPMPP debug", "currImg cropXStart: %d", (int16_t) currImg->cropXStart);
      __android_log_print(ANDROID_LOG_INFO, "FPMPP debug", "currImg cropYStart: %d", (int16_t) currImg->cropYStart);
      __android_log_print(ANDROID_LOG_INFO, "FPMPP", "dataset Np: %d", dataset->Np);
      __android_log_print(ANDROID_LOG_INFO, "FPMPP", "currImg->Objfcrop rows: %d", (currImg->Objfcrop).rows);
      __android_log_print(ANDROID_LOG_INFO, "FPMPP", "currImg->Objfcrop cols: %d", (currImg->Objfcrop).cols);



      fftShift(objF_centered(cv::Rect(currImg->cropXStart, currImg->cropYStart,
                                      dataset->Np, dataset->Np)),currImg->Objfcrop); // Take ROI from shifted object spectrum

      __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 109");


      complexMultiply(currImg->Objfcrop, dataset->pupil, currImg->ObjfcropP);
      ifft2(currImg->ObjfcropP, currImg->ObjcropP);
      if (runDebug) {
        std::cout << "NEW LED" << std::endl;
        showComplexImg((currImg->Objfcrop), SHOW_COMPLEX_MAG,
                       "currImg->Objfcrop");
        showComplexImg((currImg->ObjfcropP), SHOW_COMPLEX_MAG,
                       "currImg->ObjfcropP");
        showComplexImg((currImg->ObjcropP), SHOW_COMPLEX_COMPONENTS,
                       "currImg->ObjcropP");
        showComplexImg((dataset->objF), SHOW_COMPLEX_MAG,"objF");
      }

      __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 110");


      // Replace Amplitude (using pointer iteration)
      for (int i = 0; i < dataset->Np; i++) // loop through y
      {
        const uint16_t *m_i = currImg->Image.ptr<uint16_t>(i); // Input
        double *o_i = objectAmp.ptr<double>(i);                // Output

        for (int j = 0; j < dataset->Np; j++) {
          o_i[j * 2] = sqrt((double)m_i[j]); // Real
          o_i[j * 2 + 1] = 0.0;              // Imaginary
//          __android_log_print(ANDROID_LOG_INFO, "FPMPP", "FPM IS RUNNING");
        }
      }

      __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 111");


      // Update Object fourier transform (preserving phase)
      complexAbs(currImg->ObjcropP + dataset->eps, tmpMat3);
      complexDivide(currImg->ObjcropP, tmpMat3, tmpMat1);
      complexMultiply(tmpMat1, objectAmp, tmpMat3);
      fft2(tmpMat3, currImg->Objfup);

      if (runDebug) {
        showComplexImg(objectAmp, SHOW_COMPLEX_REAL,
                       "Amplitude of Input Image");
        showComplexImg(tmpMat3, SHOW_COMPLEX_COMPONENTS,
                       "Image with amplitude   replaced");
        showComplexImg((currImg->Objfup), SHOW_COMPLEX_MAG, "currImg->Objfup");
      }

      __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 112");


      ///////// Alternating Projection Method - Object ///////////
      // Numerator
      complexAbs(dataset->pupil, pupil_abs);
      complexConj(dataset->pupil, pupil_conj);
      complexMultiply(pupil_abs, pupil_conj, tmpMat1);
      complexMultiply(currImg->Objfup - currImg->ObjfcropP, tmpMat1, numerator);

      __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 113");


      // Denominator
      double p;
      double pupil_abs_max;
      cv::minMaxLoc(pupil_abs, &p, &pupil_abs_max);
      complexMultiply(pupil_abs, pupil_abs, pupil_abs_sq);
      denomSum = pupil_abs_sq + dataset->delta2;
      complexDivide(numerator, denomSum * pupil_abs_max, tmpMat2);

      __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 114");


      if (runDebug) {
        showComplexImg((numerator), SHOW_COMPLEX_MAG, "Object update Numerator");
        showComplexImg((tmpMat2), SHOW_COMPLEX_MAG, "Object update Denominator");
      }

      fftShift(dataset->objF, objF_centered);

      Mat objF_cropped = cv::Mat(
          objF_centered, cv::Rect(currImg->cropXStart, currImg->cropYStart,
                                  dataset->Np, dataset->Np));
      fftShift(tmpMat2, tmpMat2);
      tmpMat1 = tmpMat2 + objF_cropped;

      __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 115");



      if (runDebug) {
        showComplexImg((objF_cropped), SHOW_COMPLEX_MAG,
                       "Origional Object Spectrum to be updated");
        fftShift(tmpMat2, tmpMat2);
        showComplexImg((tmpMat2), SHOW_COMPLEX_MAG,
                       "Object spectrum update incriment");
      }



      tmpMat1.copyTo(cv::Mat(objF_centered,
                             cv::Rect(currImg->cropXStart, currImg->cropYStart,
                                      dataset->Np, dataset->Np)));
      fftShift(objF_centered, dataset->objF);

      __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 116");



      if (runDebug) {
        fftShift(tmpMat1, tmpMat1);
        showComplexImg((tmpMat1), SHOW_COMPLEX_MAG,
                       "Cropped updated object spectrum");
        showComplexImg((dataset->objF), SHOW_COMPLEX_MAG,
                       "Full updated object spectrum");
      }

      __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 117");


      ////// PUPIL UPDATE ///////
      // Numerator
      complexAbs(currImg->Objfcrop, Objfcrop_abs);
      complexAbs(dataset->objF, Objf_abs);
      complexConj(currImg->Objfcrop, Objfcrop_conj);
      complexMultiply(Objfcrop_abs, Objfcrop_conj, tmpMat1);
      complexMultiply(currImg->Objfup - currImg->ObjfcropP, tmpMat1, numerator);

      // Denominator
      cv::minMaxLoc(Objf_abs, &p, &Objf_abs_max);
      complexMultiply(Objfcrop_abs, Objfcrop_abs, Objfcrop_abs_sq);
      denomSum = Objfcrop_abs_sq + dataset->delta1;
      complexDivide(numerator, denomSum * Objf_abs_max, tmpMat2);
      complexMultiply(tmpMat2, dataset->pupilSupport, tmpMat2);

      dataset->pupil += tmpMat2;
    }
    t2 = clock();
    float diff(((float)t2 - (float)t1) / CLOCKS_PER_SEC);
    cout << "Iteration " << itr << " Completed (Time: " << diff << " sec)"
         << endl;
    dft(dataset->objF, dataset->objCrop, DFT_INVERSE | DFT_SCALE);
  }
  // showImgObject((dataset->objCrop), "Object");
  // showImgFourier((dataset->objF),"Object Spectrum");
  // showImgObject(fftShift(dataset->pupil),"Pupil");

  __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 118");


  char obj_filename[FILENAME_LENGTH];
  char obji_filename[FILENAME_LENGTH];
  snprintf(obj_filename,sizeof(obj_filename), "/storage/emulated/0/CellScope/FPM_results/object_real.tiff");
  snprintf(obji_filename,sizeof(obj_filename), "/storage/emulated/0/CellScope/FPM_results/object_imaginary.tiff");

  vector<Mat> complexChannels;
  split(dataset->objCrop, complexChannels);


  //write files
  __android_log_print(ANDROID_LOG_INFO, "FPMPP", "complexChannels value at (50,50) is: %f", complexChannels[0].at<float>(50, 50));
  __android_log_print(ANDROID_LOG_INFO, "FPMPP", "complexChannels types is %d", complexChannels[0].type());
  imwrite(obj_filename, complexChannels[0]);
  imwrite(obji_filename, complexChannels[1]);
  complex_imwrite("/storage/emulated/0/CellScope/FPM_results/complexImage.tiff", dataset->objCrop);




//  imwrite(obj_filename, dataset->objCrop);
//
//
//  __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 119");

//  __android_log_print(ANDROID_LOG_INFO, "FPMPP", "type: %d", (dataset->objF).type());
//
//
//  Mat dst;
//  Mat src = dataset->objF;
//  src.convertTo(dst, CV_64FC1);

//  __android_log_print(ANDROID_LOG_INFO, "FPMPP", "type: %d", (dataset->objF).type());

//  imwrite(objF_filename, dataset->objF);

  __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 120");

 __android_log_print(ANDROID_LOG_INFO, "FPMPP", "channels: %d", (dataset->pupil).channels());

//  imwrite(pupil_filename, dataset->pupil);

//  __android_log_print(ANDROID_LOG_INFO, "FPMPP", "cp 121");


  t4 = clock();
  float diff(((float)t4 - (float)t3) / CLOCKS_PER_SEC);
  cout << "FP Processing Completed (Time: " << diff << " sec)" << endl;

  // showComplexImg(dataset->objF, SHOW_COMPLEX_MAG, "Object Spectrum");
  //fftShift(dataset->objF,dataset->objF);
  //showComplexImg(dataset->objF, SHOW_COMPLEX_COMPONENTS, "Object Fourier Spectrum");

//  showComplexImg(dataset->objCrop, SHOW_AMP_PHASE, "Object");
//  fftShift(dataset->pupil, dataset->pupil);
//  showComplexImg(dataset->pupil, SHOW_AMP_PHASE, "Pupil");
}


JNIEXPORT jint JNICALL Java_com_wallerlab_compcellscope_ComputationalCellScopeMain_computeFPM(JNIEnv* env, jint iterations) {
  char const * json_filename = "dataset_cellScope.json";
  // The dataset object, which contains all images and experimental parameters
  FPM_Dataset mDataset;

  // Load parameters from json file
  Json::Value datasetJson;
  Json::Reader reader;
  ifstream jsonFile(json_filename);
  reader.parse(jsonFile, datasetJson);

  __android_log_print(ANDROID_LOG_INFO, "FPMPP", "json name: %s", json_filename);

  // for cellscope dataset json (manually inserted)
  mDataset.datasetRoot = datasetJson.get("datasetRoot", "/storage/emulated/0/CellScope/FPM_Sample/").asString();
  mDataset.debug = datasetJson.get("debug", false).asBool();
  mDataset.color = datasetJson.get("isColor", true).asBool();
  mDataset.ledCount = datasetJson.get("ledCount", 508).asInt();
  mDataset.pixelSize = datasetJson.get("pixelSize", 8).asDouble();
  mDataset.objectiveMag = datasetJson.get("objectiveMag", 4).asDouble();
  mDataset.objectiveNA = datasetJson.get("objectiveNA", 0.1).asDouble();
  mDataset.maxIlluminationNA = datasetJson.get("maxIlluminationNA", 0.77).asDouble();
  mDataset.centerLED = datasetJson.get("centerLED", 249).asInt();
  mDataset.lambda = datasetJson.get("lambda", 0.48).asDouble();
  mDataset.cropSizeX = datasetJson.get("cropSizeX", 100).asInt();
  mDataset.cropSizeY = datasetJson.get("cropSizeY", 100).asInt();
  mDataset.filePrefix = datasetJson.get("filePrefix", "scanning_blue_cropped_").asString();
  mDataset.fileExtension = datasetJson.get("fileExtension", ".tiff").asString();
  mDataset.holeNumberDigits = datasetJson.get("holeNumberDigits",4).asInt();
  mDataset.cropX = datasetJson.get("cropX", 0).asInt();
  mDataset.cropY = datasetJson.get("cropY", 0).asInt();
  mDataset.bk1cropX = datasetJson.get("bk1cropX", 0).asInt();
  mDataset.bk1cropY = datasetJson.get("bk1cropY", 0).asInt();
  mDataset.bk2cropX = datasetJson.get("bk2cropX", 0).asInt();
  mDataset.bk2cropY = datasetJson.get("bk2cropY", 0).asInt();
  mDataset.bgThreshold = datasetJson.get("bgThresh", 1000).asInt();
  mDataset.delta1 = datasetJson.get("delta1", 1000).asDouble();
  mDataset.delta2 = datasetJson.get("delta2", 70).asDouble();
  mDataset.arrayRotation = datasetJson.get("arrayRotation", 126).asInt();
  mDataset.flipDatasetX = datasetJson.get("flipDatasetX", true).asBool();
  mDataset.flipDatasetY = datasetJson.get("flipDatasetY", false).asBool();
  mDataset.darkfieldExpMultiplier = datasetJson.get("darkfieldExpMultiplier", 1).asInt();

  __android_log_print(ANDROID_LOG_INFO, "FPMPP", "set all fields");




  //other fields
  mDataset.Np = datasetJson.get("cropSizeX", 100).asInt();
  mDataset.ps_eff = mDataset.pixelSize / (float)mDataset.objectiveMag;
  mDataset.du = (1 / mDataset.ps_eff) / (float)mDataset.Np;
  mDataset.leadingZeros = datasetJson.get("leadingZeros", false).asBool();


  std::cout << "Dataset Root: " << mDataset.datasetRoot << std::endl;
  char fileName[FILENAME_LENGTH];
  sprintf(fileName, "%s%04d%s", mDataset.filePrefix.c_str(), mDataset.centerLED,
          mDataset.fileExtension.c_str());

  cout << mDataset.datasetRoot + fileName << endl;

  if (loadImgDebug)
  {
    Mat img = imread(mDataset.datasetRoot + fileName, CV_LOAD_IMAGE_ANYDEPTH);
    std::cout << img.size();
    showImg(
    img(cv::Rect(mDataset.cropX, mDataset.cropY, mDataset.Np, mDataset.Np)),
      "Cropped Center Image");
  }
  int16_t resImprovementFactor = 1+(int16_t)ceil(
      2 * mDataset.ps_eff *
      (mDataset.maxIlluminationNA + mDataset.objectiveNA) / mDataset.lambda);

  std::cout << "resImprovementFactor: " << resImprovementFactor <<std::endl;
  mDataset.Mcrop = mDataset.Np;
  mDataset.Ncrop = mDataset.Np;
  mDataset.Nlarge = mDataset.Ncrop * resImprovementFactor;
  mDataset.Mlarge = mDataset.Mcrop * resImprovementFactor;
  mDataset.ps = mDataset.ps_eff / (float)resImprovementFactor;
  mDataset.itrCount = 1;
  runDebug = datasetJson.get("debug",false).asBool();
  mDataset.flipIlluminationX = datasetJson.get("flipDatasetX", false).asBool();
  mDataset.flipIlluminationY = datasetJson.get("flipDatasetY", false).asBool();

  // Reserve memory for imageStack
  mDataset.imageStack.reserve(mDataset.ledCount);

  // Load the datastack, and process if we find images

  __android_log_print(ANDROID_LOG_INFO, "FPMPP", "preload checkpoint");

  if(loadFPMDataset(&mDataset) > 0)
    __android_log_print(ANDROID_LOG_INFO, "FPMPP", "loaded dataset");
    runFPM(&mDataset);
    __android_log_print(ANDROID_LOG_INFO, "FPMPP", "run dataset");
}

}
