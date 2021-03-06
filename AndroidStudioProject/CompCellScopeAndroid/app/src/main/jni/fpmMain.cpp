/*
fpmMain.cpp
*/

#include <time.h>
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

#define IS_ANDROID 1

#ifdef IS_ANDROID
#include <jni.h>
// Android Log Print
#define DEBUG(x...) do { \
  char buf[1024]; \
  sprintf(buf, x); \
    __android_log_print(ANDROID_LOG_ERROR,"TAG", "%s | %s:%d", buf, __FILE__, __LINE__); \
} while (0)
#else
    #define DEBUG(x...) do { \
      char buf[1024]; \
      sprintf(buf, x); \
      printf( "%s | %s:%d", buf, __FILE__, __LINE__); \
    } while (0)
#endif


using namespace std;
using namespace cv;

#define FILENAME_LENGTH 129
#define FILE_HOLENUM_DIGITS 4


string filePrefix = "iLED_";

bool loadImgDebug = 0;
bool runDebug = 0;



extern "C" {


/*
int computeFPM(int iterations);

// Debug flags
bool runDebug = false;
bool loadImgDebug = false;

*/
int16_t loadFPMDataset(FPM_Dataset *dataset) {
    DIR *dir;
    struct dirent *ent;
    Mat fullImg;
    Mat fullImgComplex;
    FPMimg tmpFPMimg;
    tmpFPMimg.Image = Mat::zeros(dataset->Np, dataset->Np, CV_8UC1);

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

    __android_log_print(ANDROID_LOG_INFO, "FPMPP", "preload checkpoint2");

    // Generate Rotation Matrix
    double angle = dataset->arrayRotation;
    Mat rotationMatrixZ = (Mat_<double>(3,3) << cos(angle*M_PI/180), -sin(angle*M_PI/180), 0,sin(angle*M_PI/180), cos(angle*M_PI/180), 0, 0, 0, 1);

    __android_log_print(ANDROID_LOG_INFO, "FPMPP", "preload checkpoint3");

    __android_log_print(ANDROID_LOG_INFO, "FPMPP","datasetRoot: %s", dataset->datasetRoot.c_str());

    if ((dir = opendir(dataset->datasetRoot.c_str())) != NULL) {
        int16_t num_images = 0;
        DEBUG("Loading Images...");
        while ((ent = readdir(dir)) != NULL) {
            string fileName = ent->d_name;
            DEBUG("Filename is: %s",fileName.c_str());
            // Get data from file name, if name is right format.
            if (fileName.compare(".") != 0 && fileName.compare("..") != 0 &&
                (strcmp(dataset->fileExtension.c_str(),&(ent->d_name[strlen(ent->d_name) -dataset->fileExtension.length()])) == 0) &&(fileName.find(dataset->filePrefix) == 0)){
                string holeNum = fileName.substr(fileName.find(dataset->filePrefix) +
                                                 dataset->filePrefix.length(),
                                                 fileName.length()-dataset->fileExtension.length()-dataset->filePrefix.length());
                FPMimg currentImage;
                currentImage.led_num = atoi(holeNum.c_str());

                float posX = dataset->holeCoordinates[currentImage.led_num - 1][0].get("x",0).asFloat();
                float posY = dataset->holeCoordinates[currentImage.led_num - 1][1].get("y",0).asFloat();
                float posZ = dataset->holeCoordinates[currentImage.led_num - 1][2].get("z",0).asFloat();

                Mat holeCoordinatesIn = (Mat_<double>(1,3) << posX, posY, posZ);

                //float posX = dataset->holeCoordinates[currentImage.led_num-1].get("x",0).asFloat();
                //std::cout << posX <<std::endl;
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


                currentImage.illumination_na =
                        sqrt(currentImage.sinTheta_x * currentImage.sinTheta_x +
                             currentImage.sinTheta_y * currentImage.sinTheta_y);
                std::cout <<"NA:"<<sqrt(currentImage.sinTheta_x * currentImage.sinTheta_x + currentImage.sinTheta_y * currentImage.sinTheta_y)<<endl;
                if (sqrt(currentImage.illumination_na<dataset->maxIlluminationNA))
                {

                    if (dataset->color) {
                        fullImg = imread(dataset->datasetRoot + fileName,
                                         CV_LOAD_IMAGE_ANYDEPTH | CV_LOAD_IMAGE_COLOR);
                        Mat channels[3];
                        split(fullImg, channels);
                        cvtColor(fullImg, fullImg, CV_RGB2GRAY);
                        fullImg = channels[2]; // Green Channel

                    } else {
                        fullImg =
                                imread(dataset->datasetRoot + fileName, CV_LOAD_IMAGE_ANYDEPTH);
                    }

                    // Populate fields of FPMImage class
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

                    num_images++;
                    std::cout << "Loaded: " << fileName
                    << ", LED # is: " << currentImage.led_num << std::endl;

                    // Interface for printing all of these values for debugging
                    if (loadImgDebug) {
                        std::cout << "   sintheta_x is: " << currentImage.sinTheta_x
                        << ", sintheta_y is: " << currentImage.sinTheta_y
                        << std::endl;
                        std::cout << "   Image Size is: " << currentImage.Image.rows << " x "
                        << currentImage.Image.cols << std::endl;
                        std::cout << "   uled : " << currentImage.uled << std::endl;
                        std::cout << "   vled : " << currentImage.vled << std::endl;
                        std::cout << "   idx_u : " << currentImage.idx_u << std::endl;
                        std::cout << "   idx_v : " << currentImage.idx_v << std::endl;
                        std::cout << "   pupilShiftX : " << currentImage.pupilShiftX
                        << std::endl;
                        std::cout << "   pupilShiftY : " << currentImage.pupilShiftY
                        << std::endl;
                        std::cout << "   cropXStart : " << currentImage.cropXStart
                        << std::endl;
                        std::cout << "   cropXEnd : " << currentImage.cropXEnd << std::endl;
                        std::cout << "   cropYStart : " << currentImage.cropYStart
                        << std::endl;
                        std::cout << "   cropYEnd : " << currentImage.cropYEnd << std::endl;
                        std::cout << "   illumination na: " << currentImage.illumination_na
                        << std::endl;
                        std::cout << std::endl << std::endl;
                    }
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
        int16_t indexIncr = 1;
        for (auto i : sort_indexes(dataset->illuminationNAList)) {
            if (indexIncr <= dataset->ledUsedCount)
            {
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
        // could not open directory
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

    // Initialize pupil function
    planes[0] = Mat::zeros(dataset->Np, dataset->Np, CV_64F);
    planes[1] = Mat::zeros(dataset->Np, dataset->Np, CV_64F);
    cv::Point center(cvRound(dataset->Np / 2), cvRound(dataset->Np / 2));
    int16_t naRadius = (int16_t)ceil(dataset->objectiveNA * dataset->ps_eff *
                                     dataset->Np / dataset->lambda);
    cv::circle(planes[0], center, naRadius, cv::Scalar(1.0), -1, 8, 0);

    // FFTshift the pupil so it is consistant with object FT
    fftShift(planes[0], planes[0]);

    merge(planes, 2, dataset->pupil);
    dataset->pupilSupport = dataset->pupil.clone();

    // Initialize FT of reconstructed object with center led image
    planes[0] =
            Mat_<double>(dataset->imageStack.at(dataset->sortedIndicies.at(1)).Image);
    cv::sqrt(planes[0], planes[0]); // Convert to amplitude
    planes[1] = Mat::zeros(dataset->Np, dataset->Np, CV_64F); // No Phase information (yet)
    merge(planes, 2, complexI); // Complex matricies are stored in NxMx2 matricies

    // Compute the fft of the input image (amp only)
    fft2(complexI, complexI);
    complexMultiply(complexI, dataset->pupilSupport, complexI); // Filter to pupil support
    fftShift(complexI, complexI); // Shift to center

    // Initialize our final image spectrum
    planes[0] = Mat::zeros(dataset->Nlarge, dataset->Mlarge, CV_64F);
    planes[1] = Mat::zeros(dataset->Nlarge, dataset->Mlarge, CV_64F);
    merge(planes, 2, dataset->objF);

    // Place center LED image in the correct position in the large spectrum
    complexI.copyTo(
            cv::Mat(dataset->objF, cv::Rect((int16_t)round(dataset->Mlarge / 2) -
                                            (int16_t)round(dataset->Ncrop / 2),
                                            (int16_t)round(dataset->Mlarge / 2) -
                                            (int16_t)round(dataset->Ncrop / 2),
                                            dataset->Np, dataset->Np)));

    // Shift to un-fftshifted position
    fftShift(dataset->objF, dataset->objF);
    DEBUG("Iteration count is: %d",dataset->itrCount);
    for (int16_t itr = 1; itr <= dataset->itrCount; itr++)
    {
        DEBUG("Starting iteration %d",itr);
        t1 = clock();
        for (int16_t imgIdx = 0; imgIdx < dataset->ledUsedCount; imgIdx++) //
        {
            DEBUG("Starting LED %d",imgIdx);
            int16_t ledNum = dataset->sortedIndicies.at(imgIdx);

            currImg = &dataset->imageStack.at(ledNum);

            // Update Fourier space, multply by pupil (P * O)
            fftShift(dataset->objF,
                     objF_centered); // Shifted Object spectrum (at center)

            fftShift(objF_centered(cv::Rect(currImg->cropXStart, currImg->cropYStart,
                                            dataset->Np, dataset->Np)),currImg->Objfcrop); // Take ROI from shifted object spectrum

            complexMultiply(currImg->Objfcrop, dataset->pupil, currImg->ObjfcropP);
            ifft2(currImg->ObjfcropP, currImg->ObjcropP);

            // Replace Amplitude (using pointer iteration)
            for (int i = 0; i < dataset->Np; i++) // loop through y
            {
                const uint16_t *m_i = currImg->Image.ptr<uint16_t>(i); // Input
                double *o_i = objectAmp.ptr<double>(i);                // Output

                for (int j = 0; j < dataset->Np; j++) {
                    o_i[j * 2] = sqrt((double)m_i[j]); // Reala
                    o_i[j * 2 + 1] = 0.0;              // Imaginary
                }
            }

            // Update Object fourier transform (preserving phase)
            complexAbs(currImg->ObjcropP + dataset->eps, tmpMat3);
            complexDivide(currImg->ObjcropP, tmpMat3, tmpMat1);
            complexMultiply(tmpMat1, objectAmp, tmpMat3);
            fft2(tmpMat3, currImg->Objfup);

            ///////// Object Update Function///////////
            // Numerator
            complexAbs(dataset->pupil, pupil_abs);
            complexConj(dataset->pupil, pupil_conj);
            complexMultiply(pupil_abs, pupil_conj, tmpMat1);
            complexMultiply(currImg->Objfup - currImg->ObjfcropP, tmpMat1, numerator);

            // Denominator
            double p;
            double pupil_abs_max;
            cv::minMaxLoc(pupil_abs, &p, &pupil_abs_max);
            complexMultiply(pupil_abs, pupil_abs, pupil_abs_sq);
            denomSum = pupil_abs_sq + dataset->delta2;
            complexDivide(numerator, denomSum * pupil_abs_max, tmpMat2);

            fftShift(dataset->objF, objF_centered);

            Mat objF_cropped = cv::Mat(
                    objF_centered, cv::Rect(currImg->cropXStart, currImg->cropYStart,
                                            dataset->Np, dataset->Np));
            fftShift(tmpMat2, tmpMat2);
            tmpMat1 = tmpMat2 + objF_cropped;

            // Replace the region in objF
            tmpMat1.copyTo(cv::Mat(objF_centered,
                                   cv::Rect(currImg->cropXStart, currImg->cropYStart,
                                            dataset->Np, dataset->Np)));
            fftShift(objF_centered, dataset->objF);

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
        DEBUG("FP Iteration Completed (Time: %f sec)",diff);
        dft(dataset->objF, dataset->objCrop, DFT_INVERSE | DFT_SCALE);
    }

    t4 = clock();
    float diff(((float)t4 - (float)t3) / CLOCKS_PER_SEC);
    DEBUG("FP Processing Completed (Time: %f sec)",diff);

    fftShift(dataset->pupil, dataset->pupil);

    char obj_filename[FILENAME_LENGTH];
    char obji_filename[FILENAME_LENGTH];
    char pupil_filename[FILENAME_LENGTH];
    char pupili_filename[FILENAME_LENGTH];
    snprintf(obj_filename,sizeof(obj_filename), "/storage/emulated/0/CellScope/FPM_results/pupil3.jpg");
    snprintf(pupil_filename,sizeof(obj_filename), "/storage/emulated/0/CellScope/FPM_results/object3.jpg");
    DEBUG("Saving...");
    DEBUG("Object file header is: %s",obj_filename);
    DEBUG("Object file header is: %s",pupil_filename);

    Mat complexChannels[] = {Mat::zeros(dataset->Np, dataset->Np, CV_32FC1),
                             Mat::zeros(dataset->Np, dataset->Np, CV_32FC1)};
    Mat complexChannelsPupil[] = {Mat::zeros(dataset->Np, dataset->Np, CV_32FC1),
                                  Mat::zeros(dataset->Np, dataset->Np, CV_32FC1)};

    split(dataset->objCrop, complexChannels);
    split(dataset->pupil, complexChannelsPupil);


    cv::normalize(complexChannels[0], complexChannels[0], 0, 255, CV_MINMAX);
    complexChannels[0].convertTo(complexChannels[0], CV_16U);

    cv::normalize(complexChannelsPupil[0], complexChannelsPupil[0], 0, 255, CV_MINMAX);
    complexChannelsPupil[0].convertTo(complexChannelsPupil[0], CV_16U);


    //write files
    imwrite(obj_filename, complexChannels[0]);
    //imwrite(obji_filename, complexChannels[1]);
    imwrite(pupil_filename,complexChannelsPupil[0]);
    //imwrite(pupili_filename,complexChannelsPupil[1]);

    DEBUG("Finished Writing images");


}


JNIEXPORT jint JNICALL Java_com_wallerlab_compcellscope_ComputationalCellScopeMain_computeFPM(JNIEnv* env, jint iterations) {
  char const * json_filename = "/storage/emulated/0/CellScope/dataset_dogStomach/dataset_dogStomach.json";
  // The dataset object, which contains all images and experimental parameters
    // The dataset object, which contains all images and experimental parameters
    FPM_Dataset mDataset;


    // Load parameters from json file
    Json::Value datasetJson;
    Json::Reader reader;
    ifstream jsonFile(json_filename);
    reader.parse(jsonFile, datasetJson);

    mDataset.filePrefix = datasetJson.get("filePrefix", "iLED_").asString();
    mDataset.fileExtension = datasetJson.get("fileExtension", ".tif").asString();
    mDataset.Np = datasetJson.get("cropSizeX", 90).asInt();
    mDataset.datasetRoot = datasetJson.get("datasetRoot", ".").asString();
    mDataset.pixelSize = datasetJson.get("pixelSize", 6.5).asDouble();
    mDataset.objectiveMag = datasetJson.get("objectiveMag", 8).asDouble();
    mDataset.objectiveNA = datasetJson.get("objectiveNA", 0.2).asDouble();
    mDataset.maxIlluminationNA =
            datasetJson.get("maxIlluminationNA", 0.7604).asDouble();
    mDataset.color = datasetJson.get("isColor", false).asBool();
    mDataset.centerLED = datasetJson.get("centerLED", 249).asInt();
    mDataset.lambda = datasetJson.get("lambda", 0.5).asDouble();
    mDataset.ps_eff = mDataset.pixelSize / (float)mDataset.objectiveMag;
    mDataset.du = (1 / mDataset.ps_eff) / (float)mDataset.Np;
    mDataset.leadingZeros = datasetJson.get("leadingZeros", false).asBool();
    mDataset.cropX = datasetJson.get("cropX", 1).asInt();
    mDataset.cropY = datasetJson.get("cropY", 1).asInt();
    mDataset.arrayRotation = datasetJson.get("arrayRotation", 0).asInt();
    mDataset.bk1cropX = datasetJson.get("bk1cropX", 1).asInt();
    mDataset.bk1cropY = datasetJson.get("bk1cropY", 1).asInt();
    mDataset.bk2cropX = datasetJson.get("bk2cropX", 1).asInt();
    mDataset.bk2cropY = datasetJson.get("bk2cropY", 1).asInt();
    mDataset.holeNumberDigits = datasetJson.get("holeNumberDigits",4).asInt();

    DEBUG("Dataset Root: %s",mDataset.datasetRoot.c_str());
    char fileName[FILENAME_LENGTH];
    sprintf(fileName, "%s%04d%s", mDataset.filePrefix.c_str(), mDataset.centerLED,
            mDataset.fileExtension.c_str());


    DEBUG("Dataset center filename: %s",(mDataset.datasetRoot + fileName).c_str() );

    int16_t resImprovementFactor = 1+(int16_t)ceil(
            2 * mDataset.ps_eff *
            (mDataset.maxIlluminationNA + mDataset.objectiveNA) / mDataset.lambda);

    DEBUG("Resolution improvement factor: %d",resImprovementFactor );
    mDataset.bgThreshold = datasetJson.get("bgThresh", 1000).asInt();
    mDataset.Mcrop = mDataset.Np;
    mDataset.Ncrop = mDataset.Np;
    mDataset.Nlarge = mDataset.Ncrop * resImprovementFactor;
    mDataset.Mlarge = mDataset.Mcrop * resImprovementFactor;
    mDataset.ps = mDataset.ps_eff / (float)resImprovementFactor;
    mDataset.delta1 = datasetJson.get("delta1", 5).asInt();
    mDataset.delta2 = datasetJson.get("delta2", 10).asInt();
    mDataset.itrCount = 1;
    mDataset.ledCount = datasetJson.get("ledCount", 508).asInt();
    mDataset.flipIlluminationX = datasetJson.get("flipDatasetX", false).asBool();
    mDataset.flipIlluminationY = datasetJson.get("flipDatasetY", false).asBool();
    mDataset.darkfieldExpMultiplier = datasetJson.get("darkfieldExpMultiplier",1).asInt();
    runDebug = datasetJson.get("debug",false).asBool();
    mDataset.holeCoordinates = datasetJson.get("holeCoordinates",0);

    DEBUG("Iteration Count is: %d",iterations);

    // Reserve memory for imageStack
    mDataset.imageStack.reserve(mDataset.ledCount);


  // Load the datastack, and process if we find images

  DEBUG("Finished preloading.");


  if(loadFPMDataset(&mDataset) > 0)
    DEBUG("Finished loading data.");
    runFPM(&mDataset);
    DEBUG("Finished FPM.");
}

}
