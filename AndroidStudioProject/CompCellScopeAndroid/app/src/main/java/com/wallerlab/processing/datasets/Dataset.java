/*
 * Developed as part of the Computational CellScope Project
 * Waller Lab, EECS Dept., The University of California at Berkeley
 *
 * Licensed under the 3-Clause BSD License:
 *
 * Copyright © 2015 Regents of the University of California
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the owner nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

package com.wallerlab.processing.datasets;

import java.io.File;
import java.io.FileFilter;
import java.io.Serializable;
import java.util.Arrays;
import java.util.List;

import android.util.Log;

// Constants for current image dataset
public class Dataset implements Serializable {
	private String TAG = "Dataset";

	private static final long serialVersionUID = 1L;
	
	public static final String UNITS = "mm";
    public static final int SIZE = 7;
    public static final int LED_DISTANCE = 4;
    public static final int F_CONDENSER = 60; //mm
    public static final double F1 = 16.5;
    public static final double F2 = 25.4 * 4;
    public static final double M = F2/F1;
    public static final double DX0 = 0.0053;
    public static final double DX = DX0/M; //spatial resolution

    public static final float MAX_DEPTH = 0.1f;
    public static final float DEPTH_INC = 0.01f;

    public String DATASET_PATH = "";
    public String DATASET_NAME = "";
    public String DATASET_TYPE = ""; // Can be multimode, brightfield_scan, full_scan
    public String DATASET_HEADER = "";
    
    public boolean USE_COLOR_DPC = false;
    
    public float ZMIN = -100;
    public float ZINC = 10;
    public float ZMAX = 100;
    
    public int XCROP = 507;
    public int YCROP = 96;
    
    public int WIDTH  = 3264;   // 3264
    public int HEIGHT = 2448;  //2448
    
    public File[] fileList = null;
    public int fileCount = 0;
    
    public String ARRAY_TYPE = "domeA";

    public  String getRawImagePath(int idx, int deleteMe) {
    	return String.format("%s%s%d.jpeg", DATASET_PATH, DATASET_HEADER,idx);
        //return DATASET_PATH + DATASET_NAME + "/image" + String.format("%d%d", row, col) + ".bmp";
    }

    public  String getResultImagePath(String name, float depth) {
        return DATASET_PATH + DATASET_NAME + "/" + name + String.format("%d", (int)(depth*100)) + ".png";
    }

    public  String getResultImagePath(String name) {
        return DATASET_PATH + DATASET_NAME + "/" + name + ".png";
    }
    
    public void buildFileListFromPath(String path) {

        Log.d(TAG,String.format("FilePath is: %s", path));
        // Extract directory:
        DATASET_PATH = path;
        Log.d(TAG,String.format("Path is: %s", DATASET_PATH));
        File fileList2 = new File(DATASET_PATH);
        
        fileList = fileList2.listFiles(new FileFilter() {
            @Override
            public boolean accept(File pathname) {
                String name = pathname.getName().toLowerCase();
                return name.endsWith(".jpeg") && pathname.isFile();
            }
        });
        /*
        for (int i = 0; i<mDataset.fileList.length; i++)
        {
      	  Log.d(TAG,mDataset.fileList[i].toString());
        }
        */
        
        fileCount = fileList.length;
        String firstFileName = fileList[0].getAbsoluteFile().toString();
        // Define the dataset type
        if (firstFileName.contains("Brightfield_Scan"))
        {
        	  DATASET_TYPE = "brightfield";
            DATASET_HEADER = firstFileName.substring(path.lastIndexOf(File.separator)+1,firstFileName.lastIndexOf("_scanning_"))+"_scanning_";
            Log.d(TAG,String.format("BF Scan Header is: %s", DATASET_HEADER));
        }
        
    	  else if (firstFileName.contains("multimode"))
    	  {
    		  DATASET_TYPE = "multimode";
              DATASET_HEADER = "milti_";
              Log.d(TAG,String.format("Header is: %s", DATASET_HEADER));
    	  }
        
    	  else if (firstFileName.contains("Full_Scan"))
    	  {
    		  DATASET_TYPE = "full_scan";
              DATASET_HEADER = firstFileName.substring(firstFileName.lastIndexOf(File.separator)+1,firstFileName.lastIndexOf("_scanning_"));
            Log.d(TAG,String.format("Full Scan Header is: %s", DATASET_HEADER));
    	  }
        
        // Name the Dataset after the directory
        DATASET_NAME = DATASET_PATH.substring(0, DATASET_PATH.lastIndexOf(File.separator));
    }
    
    public List<Integer> leftList = Arrays.asList(159,160,180,181,182,201,202,203,223,224,225,226,246,247,248,249,270,271,272,292,293,294,295,315,316,317,338,339,340,361);
    public List<Integer> rightList = Arrays.asList(161,162,183,184,204,205,206,227,228,229,250,251,252,253,273,274,275,276,296,297,298,299,318,319,320,321,341,342,343,362);
    public List<Integer> topList = Arrays.asList(270,271,272,273,274,275,276,292,293,294,295,296,297,298,299,315,316,317,318,319,320,321,338,339,340,341,342,343,361,362);
    public List<Integer> bottomList = Arrays.asList(159,160,161,162,180,181,182,183,184,201,202,203,204,205,206,223,224,225,226,227,228,229,246,247,248,249,250,251,252,253);
    
    // FORMAT: hole number, x, y, z in meters
    public final double[][] domeCoordinates = new double[][]{
    /*  1*/ {-0.0398, -0.0050, 0.0446},
    /*  2*/ {-0.0398, -0.0017, 0.0448},
    /*  3*/ {-0.0398, 0.0017, 0.0448},
    /*  4*/ {-0.0398, 0.0050, 0.0446},
    /*  5*/ {-0.0375, -0.0137, 0.0447},
    /*  6*/ {-0.0375, -0.0104, 0.0456},
    /*  7*/ {-0.0375, -0.0069, 0.0463},
    /*  8*/ {-0.0375, -0.0035, 0.0467},
    /*  9*/ {-0.0375, 0.0000, 0.0468},
    /* 10*/ {-0.0375, 0.0035, 0.0467},
    /* 11*/ {-0.0375, 0.0069, 0.0463},
    /* 12*/ {-0.0375, 0.0104, 0.0456},
    /* 13*/ {-0.0375, 0.0137, 0.0447},
    /* 14*/ {-0.0351, -0.0191, 0.0447},
    /* 15*/ {-0.0351, -0.0158, 0.0460},
    /* 16*/ {-0.0351, -0.0124, 0.0470},
    /* 17*/ {-0.0351, -0.0089, 0.0478},
    /* 18*/ {-0.0351, -0.0054, 0.0483},
    /* 19*/ {-0.0351, -0.0018, 0.0486},
    /* 20*/ {-0.0351, 0.0018, 0.0486},
    /* 21*/ {-0.0351, 0.0054, 0.0483},
    /* 22*/ {-0.0351, 0.0089, 0.0478},
    /* 23*/ {-0.0351, 0.0124, 0.0470},
    /* 24*/ {-0.0351, 0.0158, 0.0460},
    /* 25*/ {-0.0351, 0.0191, 0.0447},
    /* 26*/ {-0.0327, -0.0213, 0.0456},
    /* 27*/ {-0.0327, -0.0179, 0.0470},
    /* 28*/ {-0.0327, -0.0144, 0.0482},
    /* 29*/ {-0.0327, -0.0109, 0.0491},
    /* 30*/ {-0.0327, -0.0073, 0.0498},
    /* 31*/ {-0.0327, -0.0037, 0.0502},
    /* 32*/ {-0.0327, 0.0000, 0.0503},
    /* 33*/ {-0.0327, 0.0037, 0.0502},
    /* 34*/ {-0.0327, 0.0073, 0.0498},
    /* 35*/ {-0.0327, 0.0109, 0.0491},
    /* 36*/ {-0.0327, 0.0144, 0.0482},
    /* 37*/ {-0.0327, 0.0179, 0.0470},
    /* 38*/ {-0.0327, 0.0213, 0.0456},
    /* 39*/ {-0.0301, -0.0267, 0.0445},
    /* 40*/ {-0.0301, -0.0234, 0.0463},
    /* 41*/ {-0.0301, -0.0200, 0.0479},
    /* 42*/ {-0.0301, -0.0165, 0.0492},
    /* 43*/ {-0.0301, -0.0129, 0.0503},
    /* 44*/ {-0.0301, -0.0093, 0.0511},
    /* 45*/ {-0.0301, -0.0056, 0.0516},
    /* 46*/ {-0.0301, -0.0019, 0.0519},
    /* 47*/ {-0.0301, 0.0019, 0.0519},
    /* 48*/ {-0.0301, 0.0056, 0.0516},
    /* 49*/ {-0.0301, 0.0093, 0.0511},
    /* 50*/ {-0.0301, 0.0129, 0.0503},
    /* 51*/ {-0.0301, 0.0165, 0.0492},
    /* 52*/ {-0.0301, 0.0200, 0.0479},
    /* 53*/ {-0.0301, 0.0234, 0.0463},
    /* 54*/ {-0.0301, 0.0267, 0.0445},
    /* 55*/ {-0.0275, -0.0288, 0.0449},
    /* 56*/ {-0.0275, -0.0255, 0.0469},
    /* 57*/ {-0.0275, -0.0221, 0.0485},
    /* 58*/ {-0.0275, -0.0186, 0.0500},
    /* 59*/ {-0.0275, -0.0150, 0.0512},
    /* 60*/ {-0.0275, -0.0113, 0.0521},
    /* 61*/ {-0.0275, -0.0076, 0.0528},
    /* 62*/ {-0.0275, -0.0038, 0.0532},
    /* 63*/ {-0.0275, 0.0000, 0.0533},
    /* 64*/ {-0.0275, 0.0038, 0.0532},
    /* 65*/ {-0.0275, 0.0076, 0.0528},
    /* 66*/ {-0.0275, 0.0113, 0.0521},
    /* 67*/ {-0.0275, 0.0150, 0.0512},
    /* 68*/ {-0.0275, 0.0186, 0.0500},
    /* 69*/ {-0.0275, 0.0221, 0.0485},
    /* 70*/ {-0.0275, 0.0255, 0.0469},
    /* 71*/ {-0.0275, 0.0288, 0.0449},
    /* 72*/ {-0.0248, -0.0308, 0.0452},
    /* 73*/ {-0.0248, -0.0275, 0.0472},
    /* 74*/ {-0.0248, -0.0241, 0.0490},
    /* 75*/ {-0.0248, -0.0206, 0.0506},
    /* 76*/ {-0.0248, -0.0170, 0.0519},
    /* 77*/ {-0.0248, -0.0133, 0.0530},
    /* 78*/ {-0.0248, -0.0096, 0.0538},
    /* 79*/ {-0.0248, -0.0058, 0.0543},
    /* 80*/ {-0.0248, -0.0019, 0.0546},
    /* 81*/ {-0.0248, 0.0019, 0.0546},
    /* 82*/ {-0.0248, 0.0058, 0.0543},
    /* 83*/ {-0.0248, 0.0096, 0.0538},
    /* 84*/ {-0.0248, 0.0133, 0.0530},
    /* 85*/ {-0.0248, 0.0170, 0.0519},
    /* 86*/ {-0.0248, 0.0206, 0.0506},
    /* 87*/ {-0.0248, 0.0241, 0.0490},
    /* 88*/ {-0.0248, 0.0275, 0.0472},
    /* 89*/ {-0.0248, 0.0308, 0.0452},
    /* 90*/ {-0.0220, -0.0327, 0.0452},
    /* 91*/ {-0.0220, -0.0295, 0.0474},
    /* 92*/ {-0.0220, -0.0261, 0.0493},
    /* 93*/ {-0.0220, -0.0226, 0.0510},
    /* 94*/ {-0.0220, -0.0190, 0.0525},
    /* 95*/ {-0.0220, -0.0153, 0.0537},
    /* 96*/ {-0.0220, -0.0116, 0.0546},
    /* 97*/ {-0.0220, -0.0077, 0.0553},
    /* 98*/ {-0.0220, -0.0039, 0.0557},
    /* 99*/ {-0.0220, 0.0000, 0.0558},
    /*100*/ {-0.0220, 0.0039, 0.0557},
    /*101*/ {-0.0220, 0.0077, 0.0553},
    /*102*/ {-0.0220, 0.0116, 0.0546},
    /*103*/ {-0.0220, 0.0153, 0.0537},
    /*104*/ {-0.0220, 0.0190, 0.0525},
    /*105*/ {-0.0220, 0.0226, 0.0510},
    /*106*/ {-0.0220, 0.0261, 0.0493},
    /*107*/ {-0.0220, 0.0295, 0.0474},
    /*108*/ {-0.0220, 0.0327, 0.0452},
    /*109*/ {-0.0192, -0.0346, 0.0451},
    /*110*/ {-0.0192, -0.0314, 0.0474},
    /*111*/ {-0.0192, -0.0280, 0.0494},
    /*112*/ {-0.0192, -0.0246, 0.0513},
    /*113*/ {-0.0192, -0.0210, 0.0528},
    /*114*/ {-0.0192, -0.0173, 0.0541},
    /*115*/ {-0.0192, -0.0136, 0.0552},
    /*116*/ {-0.0192, -0.0097, 0.0560},
    /*117*/ {-0.0192, -0.0059, 0.0565},
    /*118*/ {-0.0192, -0.0020, 0.0568},
    /*119*/ {-0.0192, 0.0020, 0.0568},
    /*120*/ {-0.0192, 0.0059, 0.0565},
    /*121*/ {-0.0192, 0.0097, 0.0560},
    /*122*/ {-0.0192, 0.0136, 0.0552},
    /*123*/ {-0.0192, 0.0173, 0.0541},
    /*124*/ {-0.0192, 0.0210, 0.0528},
    /*125*/ {-0.0192, 0.0246, 0.0513},
    /*126*/ {-0.0192, 0.0280, 0.0494},
    /*127*/ {-0.0192, 0.0314, 0.0474},
    /*128*/ {-0.0192, 0.0346, 0.0451},
    /*129*/ {-0.0163, -0.0363, 0.0449},
    /*130*/ {-0.0163, -0.0332, 0.0473},
    /*131*/ {-0.0163, -0.0299, 0.0494},
    /*132*/ {-0.0163, -0.0265, 0.0513},
    /*133*/ {-0.0163, -0.0229, 0.0530},
    /*134*/ {-0.0163, -0.0193, 0.0544},
    /*135*/ {-0.0163, -0.0155, 0.0556},
    /*136*/ {-0.0163, -0.0117, 0.0565},
    /*137*/ {-0.0163, -0.0078, 0.0572},
    /*138*/ {-0.0163, -0.0039, 0.0576},
    /*139*/ {-0.0163, 0.0000, 0.0577},
    /*140*/ {-0.0163, 0.0039, 0.0576},
    /*141*/ {-0.0163, 0.0078, 0.0572},
    /*142*/ {-0.0163, 0.0117, 0.0565},
    /*143*/ {-0.0163, 0.0155, 0.0556},
    /*144*/ {-0.0163, 0.0193, 0.0544},
    /*145*/ {-0.0163, 0.0229, 0.0530},
    /*146*/ {-0.0163, 0.0265, 0.0513},
    /*147*/ {-0.0163, 0.0299, 0.0494},
    /*148*/ {-0.0163, 0.0332, 0.0473},
    /*149*/ {-0.0163, 0.0363, 0.0449},
    /*150*/ {-0.0134, -0.0379, 0.0445},
    /*151*/ {-0.0134, -0.0348, 0.0470},
    /*152*/ {-0.0134, -0.0316, 0.0492},
    /*153*/ {-0.0134, -0.0282, 0.0512},
    /*154*/ {-0.0134, -0.0247, 0.0530},
    /*155*/ {-0.0134, -0.0211, 0.0545},
    /*156*/ {-0.0134, -0.0174, 0.0558},
    /*157*/ {-0.0134, -0.0136, 0.0569},
    /*158*/ {-0.0134, -0.0098, 0.0577},
    /*159*/ {-0.0134, -0.0059, 0.0582},
    /*160*/ {-0.0134, -0.0020, 0.0585},
    /*161*/ {-0.0134, 0.0020, 0.0585},
    /*162*/ {-0.0134, 0.0059, 0.0582},
    /*163*/ {-0.0134, 0.0098, 0.0577},
    /*164*/ {-0.0134, 0.0136, 0.0569},
    /*165*/ {-0.0134, 0.0174, 0.0558},
    /*166*/ {-0.0134, 0.0211, 0.0545},
    /*167*/ {-0.0134, 0.0247, 0.0530},
    /*168*/ {-0.0134, 0.0282, 0.0512},
    /*169*/ {-0.0134, 0.0316, 0.0492},
    /*170*/ {-0.0134, 0.0348, 0.0470},
    /*171*/ {-0.0134, 0.0379, 0.0445},
    /*172*/ {-0.0105, -0.0364, 0.0465},
    /*173*/ {-0.0105, -0.0332, 0.0488},
    /*174*/ {-0.0105, -0.0299, 0.0509},
    /*175*/ {-0.0105, -0.0265, 0.0528},
    /*176*/ {-0.0105, -0.0229, 0.0545},
    /*177*/ {-0.0105, -0.0193, 0.0559},
    /*178*/ {-0.0105, -0.0155, 0.0570},
    /*179*/ {-0.0105, -0.0117, 0.0579},
    /*180*/ {-0.0105, -0.0078, 0.0586},
    /*181*/ {-0.0105, -0.0039, 0.0590},
    /*182*/ {-0.0105, 0.0000, 0.0591},
    /*183*/ {-0.0105, 0.0039, 0.0590},
    /*184*/ {-0.0105, 0.0078, 0.0586},
    /*185*/ {-0.0105, 0.0117, 0.0579},
    /*186*/ {-0.0105, 0.0155, 0.0570},
    /*187*/ {-0.0105, 0.0193, 0.0559},
    /*188*/ {-0.0105, 0.0229, 0.0545},
    /*189*/ {-0.0105, 0.0265, 0.0528},
    /*190*/ {-0.0105, 0.0299, 0.0509},
    /*191*/ {-0.0105, 0.0332, 0.0488},
    /*192*/ {-0.0105, 0.0364, 0.0465},
    /*193*/ {-0.0075, -0.0378, 0.0460},
    /*194*/ {-0.0075, -0.0347, 0.0483},
    /*195*/ {-0.0075, -0.0315, 0.0505},
    /*196*/ {-0.0075, -0.0281, 0.0525},
    /*197*/ {-0.0075, -0.0246, 0.0542},
    /*198*/ {-0.0075, -0.0210, 0.0557},
    /*199*/ {-0.0075, -0.0173, 0.0570},
    /*200*/ {-0.0075, -0.0135, 0.0580},
    /*201*/ {-0.0075, -0.0097, 0.0587},
    /*202*/ {-0.0075, -0.0058, 0.0592},
    /*203*/ {-0.0075, -0.0020, 0.0595},
    /*204*/ {-0.0075, 0.0020, 0.0595},
    /*205*/ {-0.0075, 0.0058, 0.0592},
    /*206*/ {-0.0075, 0.0097, 0.0587},
    /*207*/ {-0.0075, 0.0135, 0.0580},
    /*208*/ {-0.0075, 0.0173, 0.0570},
    /*209*/ {-0.0075, 0.0210, 0.0557},
    /*210*/ {-0.0075, 0.0246, 0.0542},
    /*211*/ {-0.0075, 0.0281, 0.0525},
    /*212*/ {-0.0075, 0.0315, 0.0505},
    /*213*/ {-0.0075, 0.0347, 0.0483},
    /*214*/ {-0.0075, 0.0378, 0.0460},
    /*215*/ {-0.0045, -0.0391, 0.0453},
    /*216*/ {-0.0045, -0.0361, 0.0477},
    /*217*/ {-0.0045, -0.0329, 0.0499},
    /*218*/ {-0.0045, -0.0296, 0.0520},
    /*219*/ {-0.0045, -0.0262, 0.0538},
    /*220*/ {-0.0045, -0.0227, 0.0554},
    /*221*/ {-0.0045, -0.0190, 0.0567},
    /*222*/ {-0.0045, -0.0153, 0.0578},
    /*223*/ {-0.0045, -0.0116, 0.0587},
    /*224*/ {-0.0045, -0.0077, 0.0593},
    /*225*/ {-0.0045, -0.0039, 0.0597},
    /*226*/ {-0.0045, 0.0000, 0.0598},
    /*227*/ {-0.0045, 0.0039, 0.0597},
    /*228*/ {-0.0045, 0.0077, 0.0593},
    /*229*/ {-0.0045, 0.0116, 0.0587},
    /*230*/ {-0.0045, 0.0153, 0.0578},
    /*231*/ {-0.0045, 0.0190, 0.0567},
    /*232*/ {-0.0045, 0.0227, 0.0554},
    /*233*/ {-0.0045, 0.0262, 0.0538},
    /*234*/ {-0.0045, 0.0296, 0.0520},
    /*235*/ {-0.0045, 0.0329, 0.0499},
    /*236*/ {-0.0045, 0.0361, 0.0477},
    /*237*/ {-0.0045, 0.0391, 0.0453},
    /*238*/ {-0.0015, -0.0403, 0.0445},
    /*239*/ {-0.0015, -0.0373, 0.0469},
    /*240*/ {-0.0015, -0.0343, 0.0492},
    /*241*/ {-0.0015, -0.0310, 0.0513},
    /*242*/ {-0.0015, -0.0277, 0.0532},
    /*243*/ {-0.0015, -0.0242, 0.0549},
    /*244*/ {-0.0015, -0.0207, 0.0563},
    /*245*/ {-0.0015, -0.0170, 0.0575},
    /*246*/ {-0.0015, -0.0133, 0.0585},
    /*247*/ {-0.0015, -0.0096, 0.0592},
    /*248*/ {-0.0015, -0.0057, 0.0597},
    /*249*/ {-0.0015, -0.0019, 0.0600},
    /*250*/ {-0.0015, 0.0019, 0.0600},
    /*251*/ {-0.0015, 0.0057, 0.0597},
    /*252*/ {-0.0015, 0.0096, 0.0592},
    /*253*/ {-0.0015, 0.0133, 0.0585},
    /*254*/ {-0.0015, 0.0170, 0.0575},
    /*255*/ {-0.0015, 0.0207, 0.0563},
    /*256*/ {-0.0015, 0.0242, 0.0549},
    /*257*/ {-0.0015, 0.0277, 0.0532},
    /*258*/ {-0.0015, 0.0310, 0.0513},
    /*259*/ {-0.0015, 0.0343, 0.0492},
    /*260*/ {-0.0015, 0.0373, 0.0469},
    /*261*/ {-0.0015, 0.0403, 0.0445},
    /*262*/ {0.0015, -0.0388, 0.0457},
    /*263*/ {0.0015, -0.0358, 0.0481},
    /*264*/ {0.0015, -0.0327, 0.0503},
    /*265*/ {0.0015, -0.0294, 0.0523},
    /*266*/ {0.0015, -0.0260, 0.0541},
    /*267*/ {0.0015, -0.0225, 0.0556},
    /*268*/ {0.0015, -0.0189, 0.0569},
    /*269*/ {0.0015, -0.0152, 0.0580},
    /*270*/ {0.0015, -0.0114, 0.0589},
    /*271*/ {0.0015, -0.0077, 0.0595},
    /*272*/ {0.0015, -0.0038, 0.0599},
    /*273*/ {0.0015, 0.0000, 0.0600},
    /*274*/ {0.0015, 0.0038, 0.0599},
    /*275*/ {0.0015, 0.0077, 0.0595},
    /*276*/ {0.0015, 0.0114, 0.0589},
    /*277*/ {0.0015, 0.0152, 0.0580},
    /*278*/ {0.0015, 0.0189, 0.0569},
    /*279*/ {0.0015, 0.0225, 0.0556},
    /*280*/ {0.0015, 0.0260, 0.0541},
    /*281*/ {0.0015, 0.0294, 0.0523},
    /*282*/ {0.0015, 0.0327, 0.0503},
    /*283*/ {0.0015, 0.0358, 0.0481},
    /*284*/ {0.0015, 0.0388, 0.0457},
    /*285*/ {0.0046, -0.0376, 0.0465},
    /*286*/ {0.0046, -0.0345, 0.0488},
    /*287*/ {0.0046, -0.0313, 0.0510},
    /*288*/ {0.0046, -0.0279, 0.0529},
    /*289*/ {0.0046, -0.0245, 0.0546},
    /*290*/ {0.0046, -0.0209, 0.0561},
    /*291*/ {0.0046, -0.0172, 0.0573},
    /*292*/ {0.0046, -0.0135, 0.0583},
    /*293*/ {0.0046, -0.0096, 0.0590},
    /*294*/ {0.0046, -0.0058, 0.0595},
    /*295*/ {0.0046, -0.0019, 0.0598},
    /*296*/ {0.0046, 0.0019, 0.0598},
    /*297*/ {0.0046, 0.0058, 0.0595},
    /*298*/ {0.0046, 0.0096, 0.0590},
    /*299*/ {0.0046, 0.0135, 0.0583},
    /*300*/ {0.0046, 0.0172, 0.0573},
    /*301*/ {0.0046, 0.0209, 0.0561},
    /*302*/ {0.0046, 0.0245, 0.0546},
    /*303*/ {0.0046, 0.0279, 0.0529},
    /*304*/ {0.0046, 0.0313, 0.0510},
    /*305*/ {0.0046, 0.0345, 0.0488},
    /*306*/ {0.0046, 0.0376, 0.0465},
    /*307*/ {0.0077, -0.0393, 0.0447},
    /*308*/ {0.0077, -0.0363, 0.0472},
    /*309*/ {0.0077, -0.0331, 0.0494},
    /*310*/ {0.0077, -0.0298, 0.0515},
    /*311*/ {0.0077, -0.0264, 0.0533},
    /*312*/ {0.0077, -0.0228, 0.0550},
    /*313*/ {0.0077, -0.0192, 0.0563},
    /*314*/ {0.0077, -0.0154, 0.0575},
    /*315*/ {0.0077, -0.0116, 0.0584},
    /*316*/ {0.0077, -0.0078, 0.0590},
    /*317*/ {0.0077, -0.0039, 0.0594},
    /*318*/ {0.0077, 0.0000, 0.0595},
    /*319*/ {0.0077, 0.0039, 0.0594},
    /*320*/ {0.0077, 0.0078, 0.0590},
    /*321*/ {0.0077, 0.0116, 0.0584},
    /*322*/ {0.0077, 0.0154, 0.0575},
    /*323*/ {0.0077, 0.0192, 0.0563},
    /*324*/ {0.0077, 0.0228, 0.0550},
    /*325*/ {0.0077, 0.0264, 0.0533},
    /*326*/ {0.0077, 0.0298, 0.0515},
    /*327*/ {0.0077, 0.0331, 0.0494},
    /*328*/ {0.0077, 0.0363, 0.0472},
    /*329*/ {0.0077, 0.0393, 0.0447},
    /*330*/ {0.0108, -0.0379, 0.0453},
    /*331*/ {0.0108, -0.0348, 0.0477},
    /*332*/ {0.0108, -0.0316, 0.0499},
    /*333*/ {0.0108, -0.0282, 0.0519},
    /*334*/ {0.0108, -0.0247, 0.0536},
    /*335*/ {0.0108, -0.0211, 0.0551},
    /*336*/ {0.0108, -0.0174, 0.0564},
    /*337*/ {0.0108, -0.0136, 0.0574},
    /*338*/ {0.0108, -0.0098, 0.0582},
    /*339*/ {0.0108, -0.0059, 0.0587},
    /*340*/ {0.0108, -0.0020, 0.0590},
    /*341*/ {0.0108, 0.0020, 0.0590},
    /*342*/ {0.0108, 0.0059, 0.0587},
    /*343*/ {0.0108, 0.0098, 0.0582},
    /*344*/ {0.0108, 0.0136, 0.0574},
    /*345*/ {0.0108, 0.0174, 0.0564},
    /*346*/ {0.0108, 0.0211, 0.0551},
    /*347*/ {0.0108, 0.0247, 0.0536},
    /*348*/ {0.0108, 0.0282, 0.0519},
    /*349*/ {0.0108, 0.0316, 0.0499},
    /*350*/ {0.0108, 0.0348, 0.0477},
    /*351*/ {0.0108, 0.0379, 0.0453},
    /*352*/ {0.0138, -0.0364, 0.0457},
    /*353*/ {0.0138, -0.0332, 0.0480},
    /*354*/ {0.0138, -0.0299, 0.0502},
    /*355*/ {0.0138, -0.0265, 0.0521},
    /*356*/ {0.0138, -0.0229, 0.0537},
    /*357*/ {0.0138, -0.0193, 0.0551},
    /*358*/ {0.0138, -0.0155, 0.0563},
    /*359*/ {0.0138, -0.0117, 0.0572},
    /*360*/ {0.0138, -0.0078, 0.0579},
    /*361*/ {0.0138, -0.0039, 0.0583},
    /*362*/ {0.0138, 0.0000, 0.0584},
    /*363*/ {0.0138, 0.0039, 0.0583},
    /*364*/ {0.0138, 0.0078, 0.0579},
    /*365*/ {0.0138, 0.0117, 0.0572},
    /*366*/ {0.0138, 0.0155, 0.0563},
    /*367*/ {0.0138, 0.0193, 0.0551},
    /*368*/ {0.0138, 0.0229, 0.0537},
    /*369*/ {0.0138, 0.0265, 0.0521},
    /*370*/ {0.0138, 0.0299, 0.0502},
    /*371*/ {0.0138, 0.0332, 0.0480},
    /*372*/ {0.0138, 0.0364, 0.0457},
    /*373*/ {0.0168, -0.0347, 0.0460},
    /*374*/ {0.0168, -0.0315, 0.0483},
    /*375*/ {0.0168, -0.0281, 0.0503},
    /*376*/ {0.0168, -0.0246, 0.0521},
    /*377*/ {0.0168, -0.0210, 0.0536},
    /*378*/ {0.0168, -0.0174, 0.0549},
    /*379*/ {0.0168, -0.0136, 0.0560},
    /*380*/ {0.0168, -0.0097, 0.0568},
    /*381*/ {0.0168, -0.0059, 0.0573},
    /*382*/ {0.0168, -0.0020, 0.0576},
    /*383*/ {0.0168, 0.0020, 0.0576},
    /*384*/ {0.0168, 0.0059, 0.0573},
    /*385*/ {0.0168, 0.0097, 0.0568},
    /*386*/ {0.0168, 0.0136, 0.0560},
    /*387*/ {0.0168, 0.0174, 0.0549},
    /*388*/ {0.0168, 0.0210, 0.0536},
    /*389*/ {0.0168, 0.0246, 0.0521},
    /*390*/ {0.0168, 0.0281, 0.0503},
    /*391*/ {0.0168, 0.0315, 0.0483},
    /*392*/ {0.0168, 0.0347, 0.0460},
    /*393*/ {0.0197, -0.0329, 0.0461},
    /*394*/ {0.0197, -0.0296, 0.0483},
    /*395*/ {0.0197, -0.0262, 0.0502},
    /*396*/ {0.0197, -0.0227, 0.0519},
    /*397*/ {0.0197, -0.0191, 0.0533},
    /*398*/ {0.0197, -0.0154, 0.0545},
    /*399*/ {0.0197, -0.0116, 0.0555},
    /*400*/ {0.0197, -0.0078, 0.0561},
    /*401*/ {0.0197, -0.0039, 0.0565},
    /*402*/ {0.0197, 0.0000, 0.0567},
    /*403*/ {0.0197, 0.0039, 0.0565},
    /*404*/ {0.0197, 0.0078, 0.0561},
    /*405*/ {0.0197, 0.0116, 0.0555},
    /*406*/ {0.0197, 0.0154, 0.0545},
    /*407*/ {0.0197, 0.0191, 0.0533},
    /*408*/ {0.0197, 0.0227, 0.0519},
    /*409*/ {0.0197, 0.0262, 0.0502},
    /*410*/ {0.0197, 0.0296, 0.0483},
    /*411*/ {0.0197, 0.0329, 0.0461},
    /*412*/ {0.0226, -0.0310, 0.0461},
    /*413*/ {0.0226, -0.0277, 0.0482},
    /*414*/ {0.0226, -0.0243, 0.0500},
    /*415*/ {0.0226, -0.0208, 0.0516},
    /*416*/ {0.0226, -0.0171, 0.0529},
    /*417*/ {0.0226, -0.0134, 0.0539},
    /*418*/ {0.0226, -0.0096, 0.0547},
    /*419*/ {0.0226, -0.0058, 0.0553},
    /*420*/ {0.0226, -0.0019, 0.0555},
    /*421*/ {0.0226, 0.0019, 0.0555},
    /*422*/ {0.0226, 0.0058, 0.0553},
    /*423*/ {0.0226, 0.0096, 0.0547},
    /*424*/ {0.0226, 0.0134, 0.0539},
    /*425*/ {0.0226, 0.0171, 0.0529},
    /*426*/ {0.0226, 0.0208, 0.0516},
    /*427*/ {0.0226, 0.0243, 0.0500},
    /*428*/ {0.0226, 0.0277, 0.0482},
    /*429*/ {0.0226, 0.0310, 0.0461},
    /*430*/ {0.0254, -0.0290, 0.0459},
    /*431*/ {0.0254, -0.0257, 0.0479},
    /*432*/ {0.0254, -0.0223, 0.0496},
    /*433*/ {0.0254, -0.0187, 0.0510},
    /*434*/ {0.0254, -0.0151, 0.0522},
    /*435*/ {0.0254, -0.0114, 0.0531},
    /*436*/ {0.0254, -0.0076, 0.0538},
    /*437*/ {0.0254, -0.0038, 0.0542},
    /*438*/ {0.0254, 0.0000, 0.0543},
    /*439*/ {0.0254, 0.0038, 0.0542},
    /*440*/ {0.0254, 0.0076, 0.0538},
    /*441*/ {0.0254, 0.0114, 0.0531},
    /*442*/ {0.0254, 0.0151, 0.0522},
    /*443*/ {0.0254, 0.0187, 0.0510},
    /*444*/ {0.0254, 0.0223, 0.0496},
    /*445*/ {0.0254, 0.0257, 0.0479},
    /*446*/ {0.0254, 0.0290, 0.0459},
    /*447*/ {0.0282, -0.0270, 0.0456},
    /*448*/ {0.0282, -0.0236, 0.0474},
    /*449*/ {0.0282, -0.0202, 0.0490},
    /*450*/ {0.0282, -0.0167, 0.0503},
    /*451*/ {0.0282, -0.0131, 0.0513},
    /*452*/ {0.0282, -0.0094, 0.0521},
    /*453*/ {0.0282, -0.0056, 0.0527},
    /*454*/ {0.0282, -0.0019, 0.0529},
    /*455*/ {0.0282, 0.0019, 0.0529},
    /*456*/ {0.0282, 0.0056, 0.0527},
    /*457*/ {0.0282, 0.0094, 0.0521},
    /*458*/ {0.0282, 0.0131, 0.0513},
    /*459*/ {0.0282, 0.0167, 0.0503},
    /*460*/ {0.0282, 0.0202, 0.0490},
    /*461*/ {0.0282, 0.0236, 0.0474},
    /*462*/ {0.0282, 0.0270, 0.0456},
    /*463*/ {0.0309, -0.0248, 0.0450},
    /*464*/ {0.0309, -0.0215, 0.0467},
    /*465*/ {0.0309, -0.0181, 0.0481},
    /*466*/ {0.0309, -0.0146, 0.0493},
    /*467*/ {0.0309, -0.0110, 0.0502},
    /*468*/ {0.0309, -0.0074, 0.0509},
    /*469*/ {0.0309, -0.0037, 0.0513},
    /*470*/ {0.0309, 0.0000, 0.0514},
    /*471*/ {0.0309, 0.0037, 0.0513},
    /*472*/ {0.0309, 0.0074, 0.0509},
    /*473*/ {0.0309, 0.0110, 0.0502},
    /*474*/ {0.0309, 0.0146, 0.0493},
    /*475*/ {0.0309, 0.0181, 0.0481},
    /*476*/ {0.0309, 0.0215, 0.0467},
    /*477*/ {0.0309, 0.0248, 0.0450},
    /*478*/ {0.0335, -0.0194, 0.0458},
    /*479*/ {0.0335, -0.0160, 0.0471},
    /*480*/ {0.0335, -0.0125, 0.0482},
    /*481*/ {0.0335, -0.0090, 0.0490},
    /*482*/ {0.0335, -0.0054, 0.0495},
    /*483*/ {0.0335, -0.0018, 0.0497},
    /*484*/ {0.0335, 0.0018, 0.0497},
    /*485*/ {0.0335, 0.0054, 0.0495},
    /*486*/ {0.0335, 0.0090, 0.0490},
    /*487*/ {0.0335, 0.0125, 0.0482},
    /*488*/ {0.0335, 0.0160, 0.0471},
    /*489*/ {0.0335, 0.0194, 0.0458},
    /*490*/ {0.0360, -0.0173, 0.0448},
    /*491*/ {0.0360, -0.0139, 0.0459},
    /*492*/ {0.0360, -0.0105, 0.0468},
    /*493*/ {0.0360, -0.0070, 0.0475},
    /*494*/ {0.0360, -0.0035, 0.0479},
    /*495*/ {0.0360, 0.0000, 0.0480},
    /*496*/ {0.0360, 0.0035, 0.0479},
    /*497*/ {0.0360, 0.0070, 0.0475},
    /*498*/ {0.0360, 0.0105, 0.0468},
    /*499*/ {0.0360, 0.0139, 0.0459},
    /*500*/ {0.0360, 0.0173, 0.0448},
    /*501*/ {0.0384, -0.0119, 0.0445},
    /*502*/ {0.0384, -0.0085, 0.0453},
    /*503*/ {0.0384, -0.0051, 0.0458},
    /*504*/ {0.0384, -0.0017, 0.0460},
    /*505*/ {0.0384, 0.0017, 0.0460},
    /*506*/ {0.0384, 0.0051, 0.0458},
    /*507*/ {0.0384, 0.0085, 0.0453},
    /*508*/ {0.0384, 0.0119, 0.0445},
    };

    /** DPC Refocus fields **/
    public float DPC_ZMIN = -1;
    public float DPC_ZSTEP = -1;
    public float DPC_ZMAX = -1;
    public String DPC_FOCUS_DATASET_ROOT = "";
}
