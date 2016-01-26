package com.wallerlab.compcellscope;

import android.app.Activity;
import android.content.Context;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.camera2.CameraAccessException;
import android.hardware.camera2.CameraCaptureSession;
import android.hardware.camera2.CameraCharacteristics;
import android.hardware.camera2.CameraDevice;
import android.hardware.camera2.CameraManager;
import android.hardware.camera2.CameraMetadata;
import android.hardware.camera2.CaptureRequest;
import android.hardware.camera2.TotalCaptureResult;
import android.hardware.camera2.params.StreamConfigurationMap;
import android.media.Image;
import android.media.ImageReader;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.util.Size;
import android.util.SparseIntArray;
import android.view.Surface;
import android.view.TextureView;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.Toast;
import android.media.MediaScannerConnection;
import android.hardware.camera2.DngCreator;
import android.graphics.Bitmap;


import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.io.InputStream;
import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import java.io.BufferedWriter;



import org.opencv.core.Core;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.Scalar;
import org.opencv.highgui.Highgui;
import org.opencv.imgproc.Imgproc;



import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Locale;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.DialogFragment;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.graphics.Color;
import android.media.MediaScannerConnection;
import android.net.Uri;
import android.os.AsyncTask;

import android.os.Bundle;
import android.os.Environment;
import android.os.SystemClock;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.view.View.OnTouchListener;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import android.graphics.Rect;

import com.wallerlab.compcellscope.bluetooth.BluetoothService;
import com.wallerlab.compcellscope.dialogs.AcquireSettings;
import com.wallerlab.compcellscope.dialogs.AcquireSettings.NoticeDialogListener;
import com.wallerlab.compcellscope.surfaceviews.AcquireSurfaceView;
import com.wallerlab.processing.datasets.Dataset;

import static org.opencv.highgui.Highgui.imwrite;
import static org.opencv.imgproc.Imgproc.COLOR_BayerGR2GRAY;
import static org.opencv.imgproc.Imgproc.COLOR_BayerGR2RGB;
import static org.opencv.imgproc.Imgproc.COLOR_BayerRG2GRAY;
import static org.opencv.imgproc.Imgproc.cvtColor;




public class AcquireActivity2 extends Activity implements NoticeDialogListener {


//    private class ImageReader2 extends ImageReader {
//         private ImageReader2() {
//            super();
//        }
//    }
    private HandlerThread thread;
    private static final boolean D = true;
    private BluetoothService mBluetoothService = null;
    private String acquireType = "MultiMode";
    Button btnSetup;


    public double objectiveNA = 0.1;
    public double brightfieldNA = 0.4; // Account for LED size to be sure we completly cover NA .025

    public int ledCount = 508;
    public int centerLED = 249;

    private int w;
    private int h;

    public boolean cameraReady = true;
    public int mmCount = 1;
    public float mmDelay = 0.0f;
    public int aecCompensation = 0;
    public String datasetName = "Dataset";
    public boolean usingHDR = false;
    public Dataset mDataset;
    public int exposureTime = 2800000;


    public Rect nRect= new Rect(0, 0, 100, 100);

    public DialogFragment settingsDialogFragment;
    private Size mImgSize;


    private File file;

    private final static String TAG = "Camera2testJ";
    private Size mPreviewSize;

    private TextureView mTextureView;
    private CameraDevice mCameraDevice;
    private CaptureRequest.Builder mPreviewBuilder;
    private CameraCaptureSession mPreviewSession;

    private Button mBtnShot;

    private ImageReader reader;
    private List<Surface> outputSurfaces;
    private Surface surface;

    private static final SparseIntArray ORIENTATIONS = new SparseIntArray();
    private CaptureRequest.Builder captureBuilder;

    private int index2;
    private DngCreator mDngCreator;

    public final static int MASK = 0xff;

    static {
        ORIENTATIONS.append(Surface.ROTATION_0, 90);
        ORIENTATIONS.append(Surface.ROTATION_90, 0);
        ORIENTATIONS.append(Surface.ROTATION_180, 270);
        ORIENTATIONS.append(Surface.ROTATION_270, 180);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setContentView(R.layout.acquire_layout2);



        settingsDialogFragment = new AcquireSettings();

        mTextureView = (TextureView)findViewById(R.id.texture);
        mTextureView.setSurfaceTextureListener(mSurfaceTextureListener);

        mBtnShot = (Button)findViewById(R.id.btn_takepicture);

        mBtnShot.setOnClickListener(new OnClickListener(){

            @Override
            public void onClick(View v) {

                objectiveNA = .1;
                new runScanningMode().execute();
            }

        });

        btnSetup = (Button)findViewById(R.id.btnSetup);
        btnSetup.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
                openSettingsDialog();
            }
        });

        /// new added -- new added
        GlobalApplicationClass BTAppClass = (GlobalApplicationClass) getApplication();
        mBluetoothService = BTAppClass.getBluetoothService();
        mDataset = BTAppClass.getDataset();

    }


    public void openSettingsDialog()
    {
        settingsDialogFragment.show(getFragmentManager(), "acquireSettings");

    }

    protected void takePicture() {
        cameraReady = false;
        if(null == mCameraDevice) {
            Log.e(TAG, "mCameraDevice is null, return");
            return;
        }

        CameraManager manager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
        try {
            final CameraCharacteristics characteristics = manager.getCameraCharacteristics(mCameraDevice.getId());

            Size[] imgSizes = null;
            if (characteristics != null) {
                imgSizes = characteristics
                        .get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP)
                        .getOutputSizes(ImageFormat.RAW_SENSOR);
                StreamConfigurationMap scm = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
                int[] formats = scm.getOutputFormats();
                for (int i = 0; i < formats.length; i++) {
                    Log.i("PXINFO", Integer.toString(formats[i]));
                }
            }
            int width = 640;
            int height = 480;
            if (imgSizes != null && 0 < imgSizes.length) {
                width = imgSizes[0].getWidth();
                height = imgSizes[0].getHeight();
            }
            w = width;
            h = height;
            mImgSize = imgSizes[0];
            reader = ImageReader.newInstance(width, height, ImageFormat.RAW_SENSOR, 1);
//            reader = ImageReader.newInstance(100, 100, ImageFormat.RAW_SENSOR, 1);


//
            outputSurfaces = new ArrayList<Surface>(2);
            outputSurfaces.add(reader.getSurface());

            outputSurfaces.add(new Surface(mTextureView.getSurfaceTexture()));

            captureBuilder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_STILL_CAPTURE);
            captureBuilder.addTarget(reader.getSurface());

            captureBuilder.set(CaptureRequest.CONTROL_AF_MODE, CameraMetadata.CONTROL_AF_MODE_OFF);
            captureBuilder.set(CaptureRequest.CONTROL_AE_ANTIBANDING_MODE, CameraMetadata.CONTROL_AE_ANTIBANDING_MODE_OFF);
            captureBuilder.set(CaptureRequest.CONTROL_AE_MODE,CameraMetadata.CONTROL_AE_MODE_OFF);
            captureBuilder.set(CaptureRequest.CONTROL_AWB_MODE, CameraMetadata.CONTROL_AWB_MODE_OFF);
            captureBuilder.set(CaptureRequest.CONTROL_EFFECT_MODE, CameraMetadata.CONTROL_EFFECT_MODE_OFF);
            captureBuilder.set(CaptureRequest.EDGE_MODE, CameraMetadata.EDGE_MODE_OFF);
            captureBuilder.set(CaptureRequest.FLASH_MODE, CameraMetadata.FLASH_MODE_OFF);
            captureBuilder.set(CaptureRequest.SCALER_CROP_REGION, nRect);


            //set camera exposure time (default is 1000000 ns)
            captureBuilder.set(CaptureRequest.SENSOR_EXPOSURE_TIME, (long) exposureTime);


            // Orientation
            int rotation = getWindowManager().getDefaultDisplay().getRotation();
            captureBuilder.set(CaptureRequest.JPEG_ORIENTATION, ORIENTATIONS.get(rotation));


            ImageReader.OnImageAvailableListener readerListener = new ImageReader.OnImageAvailableListener() {

                @Override
                public void onImageAvailable(ImageReader reader) {
                    Image image = null;
                    try {
                        image = reader.acquireLatestImage();
//
                        BufferedWriter writer = null;
//                        File image_txt = new File(file.getAbsolutePath());
                        String path = "/CellScope/" + "FPMScan" + "_" + datasetName;
                        file = new File(Environment.getExternalStorageDirectory()+path, "pic" +
                                "_scanning_" + String.format("%04d", index2) + ".txt");



                        ByteBuffer buf = image.getPlanes()[0].getBuffer();
                        byte[] byteArr = new byte[w*h*2];

                        String tag = "byteinfo";

                        for (int i = 0; i < w*h*2 ; i++) {
                            byteArr[i] = buf.get();
                        }
                        int[] cropped_img = new int[100*100];
                        int x = 0;
                        int y = 0;
                        int idx = 0;
                        for (int i = y; i < y+100; i++){
                            for (int j = x; j < 200; j += 2){
                                int byte1 = byteArr[i*w+j] & MASK;
                                int byte2 = byteArr[i*w+j+1] & MASK;
                                byte1 = byte1 << 8;
                                int byte3 = byte1 | byte2;
                                cropped_img[idx] = byte3;
                                idx++;
                            }
                        }






////

//                        buf.wrap(byteArr);
//                        byteArr = buf.wrap(byteArr);

////                        buf.wrap(byteArr);
//                        for (int i = (3280*1000+700)*2; i < (3280*1000+700)*2+3280*2; i++) {
//                            Log.i(tag, "Index " + Integer.toString(i) + ": " + Byte.toString(byteArr[i]));
//                            if (i%2 == 0) {
//                                int byte1 = byteArr[i] & MASK;
//                                int byte2 = byteArr[i+1] & MASK;
//                                byte1 = byte1 << 8;
//                                int byte3 = byte1 | byte2;
//                                Log.i(tag, "Index " + Integer.toString(i) + ": " + Integer.toString(byte3));
//                            }
//                        }
//                        buf.clear();
//                        Mat m = new Mat(w, h, CvType.CV_16UC1);
//                        m.put(0, 0, byteArr);
//                        imwrite(file.getAbsolutePath(), m);
//                        Highgui.imwrite(file, m);


                        save(image);
                        image.close();
                    } catch (FileNotFoundException e) {
                        e.printStackTrace();
                    } catch (IOException e) {
                        e.printStackTrace();
                    } finally {
                        if (image != null) {
                            image.close();
                            thread.quit();
                        }
                    }
                }

                private void save(Image img) throws IOException {
                    try {
                        mDngCreator.writeImage(new FileOutputStream(file.getAbsolutePath()), img);
//                        mDngCreator.writeByteBuffer();
                    } finally {
                        Log.i("PXINFO", "saved " + Integer.toString(index2));
                        cameraReady = true;
                    }
                }

            };

            thread = new HandlerThread("CameraPicture");
            thread.start();
            final Handler backgroudHandler = new Handler(thread.getLooper());
            reader.setOnImageAvailableListener(readerListener, backgroudHandler);

            final CameraCaptureSession.CaptureCallback captureListener = new CameraCaptureSession.CaptureCallback() {

                @Override
                public void onCaptureCompleted(CameraCaptureSession session,
                                               CaptureRequest request, TotalCaptureResult result) {

                    super.onCaptureCompleted(session, request, result);
                    mDngCreator = new DngCreator(characteristics, result);
                }

            };

            mCameraDevice.createCaptureSession(outputSurfaces, new CameraCaptureSession.StateCallback() {

                @Override
                public void onConfigured(CameraCaptureSession session) {

                    try {
                        session.capture(captureBuilder.build(), captureListener, backgroudHandler);
                    } catch (CameraAccessException e) {

                        e.printStackTrace();
                    }
                }

                @Override
                public void onConfigureFailed(CameraCaptureSession session) {

                }
            }, backgroudHandler);



            outputSurfaces.clear();
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }

    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.e(TAG, "onResume");
    }

    private void openCamera() {

        CameraManager manager = (CameraManager) getSystemService(Context.CAMERA_SERVICE);
        Log.e(TAG, "openCamera E");
        try {
            String cameraId = manager.getCameraIdList()[0];
            CameraCharacteristics characteristics = manager.getCameraCharacteristics(cameraId);
            StreamConfigurationMap map = characteristics.get(CameraCharacteristics.SCALER_STREAM_CONFIGURATION_MAP);
            mPreviewSize = map.getOutputSizes(SurfaceTexture.class)[0];

            manager.openCamera(cameraId, mStateCallback, null);
        } catch (CameraAccessException e) {
            e.printStackTrace();
        }
        Log.e(TAG, "openCamera X");
    }

    private TextureView.SurfaceTextureListener mSurfaceTextureListener = new TextureView.SurfaceTextureListener() {

        @Override
        public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
            Log.e(TAG, "onSurfaceTextureAvailable, width=" + width + ",height=" + height);
            openCamera();
        }

        @Override
        public void onSurfaceTextureSizeChanged(SurfaceTexture surface,
                                                int width, int height) {
            Log.e(TAG, "onSurfaceTextureSizeChanged");
        }

        @Override
        public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
            return false;
        }

        @Override
        public void onSurfaceTextureUpdated(SurfaceTexture surface) {
            //Log.e(TAG, "onSurfaceTextureUpdated");
        }

    };


    public void updateFileStructure(String currPath) {
        File f = new File(currPath);
        File[] fileList = f.listFiles();
        ArrayList<String> arrayFiles = new ArrayList<String>();
        if (!(fileList.length == 0))
        {
            for (int i=0; i<fileList.length; i++)
                arrayFiles.add(currPath+"/"+fileList[i].getName());
        }

        String[] fileListString = new String[arrayFiles.size()];
        fileListString = arrayFiles.toArray(fileListString);
        MediaScannerConnection.scanFile(AcquireActivity2.this,
                fileListString, null,
                new MediaScannerConnection.OnScanCompletedListener() {
                    public void onScanCompleted(String path, Uri uri) {
                    }
                });
    }

    private CameraDevice.StateCallback mStateCallback = new CameraDevice.StateCallback() {

        @Override
        public void onOpened(CameraDevice camera) {

            Log.e(TAG, "onOpened");
            mCameraDevice = camera;
            startPreview();
        }

        @Override
        public void onDisconnected(CameraDevice camera) {

            Log.e(TAG, "onDisconnected");
        }

        @Override
        public void onError(CameraDevice camera, int error) {

            Log.e(TAG, "onError");
        }

    };

    @Override
    protected void onPause() {

        Log.e(TAG, "onPause");
        super.onPause();
        if (null != mCameraDevice) {
            mCameraDevice.close();
            mCameraDevice = null;
        }
    }

    protected void startPreview() {

        if(null == mCameraDevice || !mTextureView.isAvailable() || null == mPreviewSize) {
            Log.e(TAG, "startPreview fail, return");
            return;
        }

        SurfaceTexture texture = mTextureView.getSurfaceTexture();
        if(null == texture) {
            Log.e(TAG,"texture is null, return");
            return;
        }

        texture.setDefaultBufferSize(mPreviewSize.getWidth(), mPreviewSize.getHeight());
        surface = new Surface(texture);

        try {
            mPreviewBuilder = mCameraDevice.createCaptureRequest(CameraDevice.TEMPLATE_PREVIEW);
        } catch (CameraAccessException e) {

            e.printStackTrace();
        }
        mPreviewBuilder.addTarget(surface);

        try {
            mCameraDevice.createCaptureSession(Arrays.asList(surface), new CameraCaptureSession.StateCallback() {

                @Override
                public void onConfigured(CameraCaptureSession session) {

                    mPreviewSession = session;
                    updatePreview();
                }

                @Override
                public void onConfigureFailed(CameraCaptureSession session) {

//                    Toast.makeText(AcquireActivity2.this, "onConfigureFailed", Toast.LENGTH_LONG).show();
                }
            }, null);
        } catch (CameraAccessException e) {

            e.printStackTrace();
        }
    }

    protected void updatePreview() {

        if(null == mCameraDevice) {
            Log.e(TAG, "updatePreview error, return");
        }

        mPreviewBuilder.set(CaptureRequest.CONTROL_MODE, CameraMetadata.CONTROL_MODE_AUTO);
        HandlerThread thread = new HandlerThread("CameraPreview");
        thread.start();
        Handler backgroundHandler = new Handler(thread.getLooper());

        try {
            mPreviewSession.setRepeatingRequest(mPreviewBuilder.build(), null, backgroundHandler);
        } catch (CameraAccessException e) {

            e.printStackTrace();
        }
    }

    private class runScanningMode extends AsyncTask<Void, Void, Void>
    {
        //ProgressDialog pdLoading = new ProgressDialog(AsyncExample.this);
        int centerCount = 0;
        long t = 0;
        int n = 0;

        String timestamp = new SimpleDateFormat("yyyyMMdd_HHmmssSSS",Locale.US).format(new Date());
        String path = "/CellScope/" + "FPMScan" + "_" + datasetName + "_" + timestamp;
        File myDir = new File(Environment.getExternalStorageDirectory()+ path);

        void mSleep(int sleepVal)
        {
            try {
                Thread.sleep(sleepVal);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        @Override
        protected void onPreExecute()
        {
            super.onPreExecute();

            if (usingHDR){
//                mPreviewBuilder.set(CaptureRequest.CONTROL_SCENE_MODE, CameraMetadata.CONTROL_SCENE_Mode);
            }

//            acquireTextView.setText(String.format("Acquiring - MODE: %s", acquireType));
//            acquireProgressBar.setVisibility(View.VISIBLE); // Make invisible at first, then have it pop up

            // Count how many LEDs there are for progress bar - this only happens once and should be fairly fast, but isn't optimal.
            for (int index=0; index<ledCount; index++) {
                if (Math.sqrt(Math.sin((double) (domeCoordinates[index][2]) / 1000.0) * Math.sin((double) (domeCoordinates[index][2]) / 1000.0) + Math.sin((double) (domeCoordinates[index][3]) / 1000.0) * Math.sin((double) (domeCoordinates[index][3]) / 1000.0)) < objectiveNA)
                    centerCount++;
            }
//            acquireProgressBar.setMax(centerCount);
            myDir.mkdirs();


            int edgeLED = centerLED; // For Now
            String cmd;
            if (acquireType.contains("Full"))
            {
                cmd = String.format("dh,%d", edgeLED);
            }
            else
            {
                cmd = String.format("dh,%d", centerLED);
            }
            sendData(cmd);
            try {
                Thread.sleep(2500);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            // Lock exposure - here we assume that the center LED is turned on when AcquireActivity class instance is created

            mDataset.DATASET_PATH = Environment.getExternalStorageDirectory()+path;
            mDataset.DATASET_TYPE = acquireType;


        }

        @Override
        protected void onProgressUpdate(Void...params)
        {
//            acquireProgressBar.setProgress(n);
//            long elapsed = SystemClock.elapsedRealtime() - t;
//            t = SystemClock.elapsedRealtime();
//            float timeLeft = (float)(((long)(centerCount - n)*elapsed)/1000.0);
//            timeLeftTextView.setText(String.format("Time left: %.2f seconds, %d/%d images saved", timeLeft,n,centerCount));
            //Log.d(TAG,String.format("Time left: %.2f seconds", timeLeft));
        }

        @Override
        protected Void doInBackground(Void... params) {
            Log.i("CAM2", "do in Background started");
            t = SystemClock.elapsedRealtime();
            int nDF = 5;
            final int exposureTimeBF = 2800000;
            final int exposureTimeDF = nDF * exposureTimeBF;
            exposureTime = 2800000;


            for (int index=1; index<=ledCount; index++) {
//            for (int index=250; index<=250; index++) {

                index2 = index;
                Log.i("CAM2", Integer.toString(domeCoordinates[index-1][0]));

                if (isBF(index)){
                    exposureTime = exposureTimeBF;
                    Log.i("BFINFO", Integer.toString(index) + " is BF");
                } else {
                    exposureTime = exposureTimeDF;
                    Log.i("BFINFO", Integer.toString(index) + " is DF");
                }
                String cmd = String.format("dh,%d",index);
                sendData(cmd);
                mSleep(100);
//                save image as DNG
                String timestamp = new SimpleDateFormat("yyyyMMdd_HHmmssSSS",Locale.US).format(new Date());
                file = new File(Environment.getExternalStorageDirectory()+path, "pic" +
                        timestamp + "_scanning_" + String.format("%04d", index) + ".dng");
                Log.i("PXINFO", "doInBackground: " + file.getAbsolutePath());
                captureImage();


                while (!cameraReady)
                {
                    try {
                        Thread.sleep(10);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                updateFileStructure(myDir.getAbsolutePath());
            }




            return null;
        }

        @Override
        protected void onPostExecute(Void result) {
            super.onPostExecute(result);
//            acquireProgressBar.setVisibility(View.INVISIBLE); // Make invisible at first, then have it pop up
            // Turn on the center LED
            //String cmd = String.format("p%d", centerLED);
            String cmd = "bf";
            sendData(cmd);

            //Clear the dome
            sendData("xx");
            mDngCreator.close();
            Log.i("PXINFO", "finished post execute");
//            updateFileStructure(myDir.getAbsolutePath());


        }
    }

    public boolean isBF(int holeNum){
        int index = holeNum - 1;
        double sin_theta_x = Math.sin(((double) domeCoordinates[index][2])/1000.0);
        double sin_theta_y = Math.sin(((double) domeCoordinates[index][3])/1000.0);
        double compNA = Math.sqrt(sin_theta_x*sin_theta_x + sin_theta_y*sin_theta_y);

        if (compNA < .1) {
            return true;
        } else {
            return false;
        }
    }

    public void captureImage()
    {
        takePicture();
    }

    public void sendData(String message) {
        message = message + "\n";
        // Check that we're actually connected before trying anything
        if (mBluetoothService.getState() != BluetoothService.STATE_CONNECTED) {
            Toast.makeText(this, "NOT CONNECTED", Toast.LENGTH_SHORT).show();
            return;
        }
        // Check that there's actually something to send
        if (message.length() > 0) {
            // Get the message bytes and tell the BluetoothChatService to write
            if (D) Log.d(TAG, message);
            byte[] send = message.getBytes();
            mBluetoothService.write(send);
        }

    }


    @SuppressLint("ClickableViewAccessibility")
    @Override
    public void onDialogPositiveClick(DialogFragment dialog) {

    }

    @Override
    public void onDialogNegativeClick(DialogFragment dialog) {

    }

    public void setHDR(boolean state)
    {
        if (state)
            usingHDR = true;
        else
            usingHDR = false;
    }

    public void setDatasetName(String name)
    {
        datasetName = name;
    }

    public void setNA(float na)
    {
        brightfieldNA = na;
        sendData(String.format("na,%d", (int) Math.round(na*100)));
    }

    public void setExposureTime(String expTime)
    {
        exposureTime = Integer.parseInt(expTime);
        Log.i("DLOGCON", Integer.toString(exposureTime));
    }

    // FORMAT: hole number,, channel, 1000*Theta_x, 1000*Theta_y
    static final int[][] domeCoordinates = new int[][]{
    /*  1*/ {1, 320, -113, -726 },
    /*  2*/ {2, 257, -38, -726 },
    /*  3*/ {3, 256, 38, -726 },
    /*  4*/ {4, 200, 113, -726 },
    /*  5*/ {5, 384, -298, -676 },
    /*  6*/ {6, 392, -223, -676 },
    /*  7*/ {7, 321, -149, -676 },
    /*  8*/ {8, 328, -74, -676 },
    /*  9*/ {9, 264, 0, -676 },
    /* 10*/ {10, 193, 74, -676 },
    /* 11*/ {11, 201, 149, -676 },
    /* 12*/ {12, 136, 223, -676 },
    /* 13*/ {13, 128, 298, -676 },
    /* 14*/ {14, 456, -405, -626 },
    /* 15*/ {15, 386, -331, -626 },
    /* 16*/ {16, 393, -258, -626 },
    /* 17*/ {17, 322, -184, -626 },
    /* 18*/ {18, 329, -110, -626 },
    /* 19*/ {19, 258, -37, -626 },
    /* 20*/ {20, 265, 37, -626 },
    /* 21*/ {21, 203, 110, -626 },
    /* 22*/ {22, 192, 184, -626 },
    /* 23*/ {23, 137, 258, -626 },
    /* 24*/ {24, 130, 331, -626 },
    /* 25*/ {25, 73, 405, -626 },
    /* 26*/ {26, 457, -437, -576 },
    /* 27*/ {27, 387, -364, -576 },
    /* 28*/ {28, 385, -291, -576 },
    /* 29*/ {29, 394, -218, -576 },
    /* 30*/ {30, 330, -146, -576 },
    /* 31*/ {31, 259, -73, -576 },
    /* 32*/ {32, 266, 0, -576 },
    /* 33*/ {33, 195, 73, -576 },
    /* 34*/ {34, 202, 146, -576 },
    /* 35*/ {35, 138, 218, -576 },
    /* 36*/ {36, 131, 291, -576 },
    /* 37*/ {37, 129, 364, -576 },
    /* 38*/ {38, 74, 437, -576 },
    /* 39*/ {39, 449, -540, -526 },
    /* 40*/ {40, 451, -468, -526 },
    /* 41*/ {41, 515, -396, -526 },
    /* 42*/ {42, 395, -324, -526 },
    /* 43*/ {43, 396, -252, -526 },
    /* 44*/ {44, 323, -180, -526 },
    /* 45*/ {45, 331, -108, -526 },
    /* 46*/ {46, 267, -36, -526 },
    /* 47*/ {47, 268, 36, -526 },
    /* 48*/ {48, 204, 108, -526 },
    /* 49*/ {49, 194, 180, -526 },
    /* 50*/ {50, 141, 252, -526 },
    /* 51*/ {51, 132, 324, -526 },
    /* 52*/ {52, 75, 396, -526 },
    /* 53*/ {53, 68, 468, -526 },
    /* 54*/ {54, 64, 540, -526 },
    /* 55*/ {55, 518, -570, -476 },
    /* 56*/ {56, 523, -498, -476 },
    /* 57*/ {57, 460, -427, -476 },
    /* 58*/ {58, 388, -356, -476 },
    /* 59*/ {59, 397, -285, -476 },
    /* 60*/ {60, 324, -214, -476 },
    /* 61*/ {61, 512, -142, -476 },
    /* 62*/ {62, 260, -71, -476 },
    /* 63*/ {63, 269, 0, -476 },
    /* 64*/ {64, 196, 71, -476 },
    /* 65*/ {65, 3, 142, -476 },
    /* 66*/ {66, 142, 214, -476 },
    /* 67*/ {67, 140, 285, -476 },
    /* 68*/ {68, 76, 356, -476 },
    /* 69*/ {69, 77, 427, -476 },
    /* 70*/ {70, 12, 498, -476 },
    /* 71*/ {71, 65, 570, -476 },
    /* 72*/ {72, 453, -598, -426 },
    /* 73*/ {73, 461, -528, -426 },
    /* 74*/ {74, 462, -458, -426 },
    /* 75*/ {75, 452, -387, -426 },
    /* 76*/ {76, 389, -317, -426 },
    /* 77*/ {77, 325, -246, -426 },
    /* 78*/ {78, 334, -176, -426 },
    /* 79*/ {79, 333, -106, -426 },
    /* 80*/ {80, 270, -35, -426 },
    /* 81*/ {81, 271, 35, -426 },
    /* 82*/ {82, 205, 106, -426 },
    /* 83*/ {83, 199, 176, -426 },
    /* 84*/ {84, 143, 246, -426 },
    /* 85*/ {85, 133, 317, -426 },
    /* 86*/ {86, 78, 387, -426 },
    /* 87*/ {87, 70, 458, -426 },
    /* 88*/ {88, 69, 528, -426 },
    /* 89*/ {89, 66, 598, -426 },
    /* 90*/ {90, 454, -626, -376 },
    /* 91*/ {91, 455, -557, -376 },
    /* 92*/ {92, 463, -487, -376 },
    /* 93*/ {93, 400, -418, -376 },
    /* 94*/ {94, 390, -348, -376 },
    /* 95*/ {95, 398, -278, -376 },
    /* 96*/ {96, 326, -209, -376 },
    /* 97*/ {97, 335, -139, -376 },
    /* 98*/ {98, 262, -70, -376 },
    /* 99*/ {99, 280, 0, -376 },
    /*100*/ {100, 198, 70, -376 },
    /*101*/ {101, 206, 139, -376 },
    /*102*/ {102, 209, 209, -376 },
    /*103*/ {103, 152, 278, -376 },
    /*104*/ {104, 135, 348, -376 },
    /*105*/ {105, 79, 418, -376 },
    /*106*/ {106, 71, 487, -376 },
    /*107*/ {107, 67, 557, -376 },
    /*108*/ {108, 13, 626, -376 },
    /*109*/ {109, 527, -654, -325 },
    /*110*/ {110, 464, -585, -325 },
    /*111*/ {111, 472, -516, -325 },
    /*112*/ {112, 473, -447, -325 },
    /*113*/ {113, 391, -378, -325 },
    /*114*/ {114, 399, -310, -325 },
    /*115*/ {115, 327, -241, -325 },
    /*116*/ {116, 336, -172, -325 },
    /*117*/ {117, 263, -103, -325 },
    /*118*/ {118, 261, -34, -325 },
    /*119*/ {119, 281, 34, -325 },
    /*120*/ {120, 207, 103, -325 },
    /*121*/ {121, 217, 172, -325 },
    /*122*/ {122, 154, 241, -325 },
    /*123*/ {123, 134, 310, -325 },
    /*124*/ {124, 9, 378, -325 },
    /*125*/ {125, 81, 447, -325 },
    /*126*/ {126, 80, 516, -325 },
    /*127*/ {127, 15, 585, -325 },
    /*128*/ {128, 14, 654, -325 },
    /*129*/ {129, 528, -680, -275 },
    /*130*/ {130, 537, -612, -275 },
    /*131*/ {131, 466, -544, -275 },
    /*132*/ {132, 474, -476, -275 },
    /*133*/ {133, 402, -408, -275 },
    /*134*/ {134, 409, -340, -275 },
    /*135*/ {135, 408, -272, -275 },
    /*136*/ {136, 337, -204, -275 },
    /*137*/ {137, 344, -136, -275 },
    /*138*/ {138, 272, -68, -275 },
    /*139*/ {139, 282, 0, -275 },
    /*140*/ {140, 208, 68, -275 },
    /*141*/ {141, 219, 136, -275 },
    /*142*/ {142, 216, 204, -275 },
    /*143*/ {143, 153, 272, -275 },
    /*144*/ {144, 144, 340, -275 },
    /*145*/ {145, 11, 408, -275 },
    /*146*/ {146, 82, 476, -275 },
    /*147*/ {147, 4, 544, -275 },
    /*148*/ {148, 24, 612, -275 },
    /*149*/ {149, 5, 680, -275 },
    /*150*/ {150, 532, -706, -225 },
    /*151*/ {151, 538, -638, -225 },
    /*152*/ {152, 465, -571, -225 },
    /*153*/ {153, 475, -504, -225 },
    /*154*/ {154, 401, -437, -225 },
    /*155*/ {155, 403, -370, -225 },
    /*156*/ {156, 411, -302, -225 },
    /*157*/ {157, 338, -235, -225 },
    /*158*/ {158, 346, -168, -225 },
    /*159*/ {159, 347, -101, -225 },
    /*160*/ {160, 273, -34, -225 },
    /*161*/ {161, 283, 34, -225 },
    /*162*/ {162, 221, 101, -225 },
    /*163*/ {163, 218, 168, -225 },
    /*164*/ {164, 210, 235, -225 },
    /*165*/ {165, 155, 302, -225 },
    /*166*/ {166, 145, 370, -225 },
    /*167*/ {167, 89, 437, -225 },
    /*168*/ {168, 83, 504, -225 },
    /*169*/ {169, 2, 571, -225 },
    /*170*/ {170, 17, 638, -225 },
    /*171*/ {171, 6, 706, -225 },
    /*172*/ {172, 530, -664, -175 },
    /*173*/ {173, 539, -598, -175 },
    /*174*/ {174, 467, -531, -175 },
    /*175*/ {175, 476, -465, -175 },
    /*176*/ {176, 404, -398, -175 },
    /*177*/ {177, 412, -332, -175 },
    /*178*/ {178, 513, -266, -175 },
    /*179*/ {179, 341, -199, -175 },
    /*180*/ {180, 348, -133, -175 },
    /*181*/ {181, 275, -66, -175 },
    /*182*/ {182, 284, 0, -175 },
    /*183*/ {183, 212, 66, -175 },
    /*184*/ {184, 220, 133, -175 },
    /*185*/ {185, 211, 199, -175 },
    /*186*/ {186, 156, 266, -175 },
    /*187*/ {187, 146, 332, -175 },
    /*188*/ {188, 58, 398, -175 },
    /*189*/ {189, 92, 465, -175 },
    /*190*/ {190, 84, 531, -175 },
    /*191*/ {191, 28, 598, -175 },
    /*192*/ {192, 16, 664, -175 },
    /*193*/ {193, 534, -689, -125 },
    /*194*/ {194, 540, -623, -125 },
    /*195*/ {195, 468, -558, -125 },
    /*196*/ {196, 469, -492, -125 },
    /*197*/ {197, 477, -426, -125 },
    /*198*/ {198, 406, -361, -125 },
    /*199*/ {199, 413, -295, -125 },
    /*200*/ {200, 339, -230, -125 },
    /*201*/ {201, 349, -164, -125 },
    /*202*/ {202, 277, -98, -125 },
    /*203*/ {203, 274, -33, -125 },
    /*204*/ {204, 286, 33, -125 },
    /*205*/ {205, 222, 98, -125 },
    /*206*/ {206, 213, 164, -125 },
    /*207*/ {207, 149, 230, -125 },
    /*208*/ {208, 158, 295, -125 },
    /*209*/ {209, 147, 361, -125 },
    /*210*/ {210, 94, 426, -125 },
    /*211*/ {211, 85, 492, -125 },
    /*212*/ {212, 27, 558, -125 },
    /*213*/ {213, 19, 623, -125 },
    /*214*/ {214, 7, 689, -125 },
    /*215*/ {215, 535, -713, -75 },
    /*216*/ {216, 541, -648, -75 },
    /*217*/ {217, 471, -583, -75 },
    /*218*/ {218, 470, -518, -75 },
    /*219*/ {219, 478, -454, -75 },
    /*220*/ {220, 405, -389, -75 },
    /*221*/ {221, 570, -324, -75 },
    /*222*/ {222, 414, -259, -75 },
    /*223*/ {223, 342, -194, -75 },
    /*224*/ {224, 448, -130, -75 },
    /*225*/ {225, 276, -65, -75 },
    /*226*/ {226, 287, 0, -75 },
    /*227*/ {227, 285, 65, -75 },
    /*228*/ {228, 223, 130, -75 },
    /*229*/ {229, 214, 194, -75 },
    /*230*/ {230, 159, 259, -75 },
    /*231*/ {231, 150, 324, -75 },
    /*232*/ {232, 148, 389, -75 },
    /*233*/ {233, 93, 454, -75 },
    /*234*/ {234, 86, 518, -75 },
    /*235*/ {235, 30, 583, -75 },
    /*236*/ {236, 18, 648, -75 },
    /*237*/ {237, 20, 713, -75 },
    /*238*/ {238, 552, -736, -25 },
    /*239*/ {239, 542, -672, -25 },
    /*240*/ {240, 488, -608, -25 },
    /*241*/ {241, 490, -544, -25 },
    /*242*/ {242, 480, -480, -25 },
    /*243*/ {243, 479, -416, -25 },
    /*244*/ {244, 424, -352, -25 },
    /*245*/ {245, 415, -288, -25 },
    /*246*/ {246, 360, -224, -25 },
    /*247*/ {247, 351, -160, -25 },
    /*248*/ {248, 278, -96, -25 },
    /*249*/ {249, 296, -32, -25 },
    /*250*/ {250, 288, 32, -25 },
    /*251*/ {251, 224, 96, -25 },
    /*252*/ {252, 53, 160, -25 },
    /*253*/ {253, 161, 224, -25 },
    /*254*/ {254, 151, 288, -25 },
    /*255*/ {255, 97, 352, -25 },
    /*256*/ {256, 95, 416, -25 },
    /*257*/ {257, 104, 480, -25 },
    /*258*/ {258, 31, 544, -25 },
    /*259*/ {259, 22, 608, -25 },
    /*260*/ {260, 23, 672, -25 },
    /*261*/ {261, 21, 736, -25 },
    /*262*/ {262, 553, -704, 26 },
    /*263*/ {263, 544, -640, 26 },
    /*264*/ {264, 489, -576, 26 },
    /*265*/ {265, 491, -512, 26 },
    /*266*/ {266, 481, -448, 26 },
    /*267*/ {267, 563, -384, 26 },
    /*268*/ {268, 416, -320, 26 },
    /*269*/ {269, 417, -256, 26 },
    /*270*/ {270, 343, -192, 26 },
    /*271*/ {271, 279, -128, 26 },
    /*272*/ {272, 297, -64, 26 },
    /*273*/ {273, 289, 0, 26 },
    /*274*/ {274, 215, 64, 26 },
    /*275*/ {275, 226, 128, 26 },
    /*276*/ {276, 8, 192, 26 },
    /*277*/ {277, 160, 256, 26 },
    /*278*/ {278, 168, 320, 26 },
    /*279*/ {279, 96, 384, 26 },
    /*280*/ {280, 105, 448, 26 },
    /*281*/ {281, 87, 512, 26 },
    /*282*/ {282, 33, 576, 26 },
    /*283*/ {283, 40, 640, 26 },
    /*284*/ {284, 41, 704, 26 },
    /*285*/ {285, 555, -680, 77 },
    /*286*/ {286, 545, -616, 77 },
    /*287*/ {287, 492, -551, 77 },
    /*288*/ {288, 482, -486, 77 },
    /*289*/ {289, 426, -421, 77 },
    /*290*/ {290, 419, -356, 77 },
    /*291*/ {291, 362, -292, 77 },
    /*292*/ {292, 361, -227, 77 },
    /*293*/ {293, 353, -162, 77 },
    /*294*/ {294, 352, -97, 77 },
    /*295*/ {295, 298, -32, 77 },
    /*296*/ {296, 290, 32, 77 },
    /*297*/ {297, 227, 97, 77 },
    /*298*/ {298, 225, 162, 77 },
    /*299*/ {299, 162, 227, 77 },
    /*300*/ {300, 169, 292, 77 },
    /*301*/ {301, 172, 356, 77 },
    /*302*/ {302, 98, 421, 77 },
    /*303*/ {303, 107, 486, 77 },
    /*304*/ {304, 106, 551, 77 },
    /*305*/ {305, 42, 616, 77 },
    /*306*/ {306, 43, 680, 77 },
    /*307*/ {307, 554, -722, 129 },
    /*308*/ {308, 556, -656, 129 },
    /*309*/ {309, 546, -590, 129 },
    /*310*/ {310, 493, -525, 129 },
    /*311*/ {311, 483, -459, 129 },
    /*312*/ {312, 420, -394, 129 },
    /*313*/ {313, 418, -328, 129 },
    /*314*/ {314, 364, -262, 129 },
    /*315*/ {315, 354, -197, 129 },
    /*316*/ {316, 355, -131, 129 },
    /*317*/ {317, 300, -66, 129 },
    /*318*/ {318, 291, 0, 129 },
    /*319*/ {319, 234, 66, 129 },
    /*320*/ {320, 50, 131, 129 },
    /*321*/ {321, 163, 197, 129 },
    /*322*/ {322, 170, 262, 129 },
    /*323*/ {323, 171, 328, 129 },
    /*324*/ {324, 99, 394, 129 },
    /*325*/ {325, 100, 459, 129 },
    /*326*/ {326, 108, 525, 129 },
    /*327*/ {327, 34, 590, 129 },
    /*328*/ {328, 44, 656, 129 },
    /*329*/ {329, 45, 722, 129 },
    /*330*/ {330, 557, -697, 180 },
    /*331*/ {331, 549, -631, 180 },
    /*332*/ {332, 547, -564, 180 },
    /*333*/ {333, 484, -498, 180 },
    /*334*/ {334, 427, -432, 180 },
    /*335*/ {335, 421, -365, 180 },
    /*336*/ {336, 363, -299, 180 },
    /*337*/ {337, 365, -232, 180 },
    /*338*/ {338, 357, -166, 180 },
    /*339*/ {339, 356, -100, 180 },
    /*340*/ {340, 299, -33, 180 },
    /*341*/ {341, 292, 33, 180 },
    /*342*/ {342, 228, 100, 180 },
    /*343*/ {343, 237, 166, 180 },
    /*344*/ {344, 239, 232, 180 },
    /*345*/ {345, 164, 299, 180 },
    /*346*/ {346, 174, 365, 180 },
    /*347*/ {347, 101, 432, 180 },
    /*348*/ {348, 109, 498, 180 },
    /*349*/ {349, 60, 564, 180 },
    /*350*/ {350, 36, 631, 180 },
    /*351*/ {351, 46, 697, 180 },
    /*352*/ {352, 558, -672, 232 },
    /*353*/ {353, 548, -605, 232 },
    /*354*/ {354, 494, -538, 232 },
    /*355*/ {355, 485, -470, 232 },
    /*356*/ {356, 429, -403, 232 },
    /*357*/ {357, 422, -336, 232 },
    /*358*/ {358, 367, -269, 232 },
    /*359*/ {359, 366, -202, 232 },
    /*360*/ {360, 358, -134, 232 },
    /*361*/ {361, 301, -67, 232 },
    /*362*/ {362, 293, 0, 232 },
    /*363*/ {363, 236, 67, 232 },
    /*364*/ {364, 231, 134, 232 },
    /*365*/ {365, 238, 202, 232 },
    /*366*/ {366, 165, 269, 232 },
    /*367*/ {367, 173, 336, 232 },
    /*368*/ {368, 102, 403, 232 },
    /*369*/ {369, 110, 470, 232 },
    /*370*/ {370, 38, 538, 232 },
    /*371*/ {371, 37, 605, 232 },
    /*372*/ {372, 63, 672, 232 },
    /*373*/ {373, 559, -646, 283 },
    /*374*/ {374, 495, -578, 283 },
    /*375*/ {375, 487, -510, 283 },
    /*376*/ {376, 486, -442, 283 },
    /*377*/ {377, 430, -374, 283 },
    /*378*/ {378, 428, -306, 283 },
    /*379*/ {379, 383, -238, 283 },
    /*380*/ {380, 359, -170, 283 },
    /*381*/ {381, 319, -102, 283 },
    /*382*/ {382, 302, -34, 283 },
    /*383*/ {383, 567, 34, 283 },
    /*384*/ {384, 229, 102, 283 },
    /*385*/ {385, 230, 170, 283 },
    /*386*/ {386, 166, 238, 283 },
    /*387*/ {387, 167, 306, 283 },
    /*388*/ {388, 175, 374, 283 },
    /*389*/ {389, 103, 442, 283 },
    /*390*/ {390, 111, 510, 283 },
    /*391*/ {391, 39, 578, 283 },
    /*392*/ {392, 62, 646, 283 },
    /*393*/ {393, 511, -619, 335 },
    /*394*/ {394, 509, -550, 335 },
    /*395*/ {395, 496, -482, 335 },
    /*396*/ {396, 446, -413, 335 },
    /*397*/ {397, 431, -344, 335 },
    /*398*/ {398, 423, -275, 335 },
    /*399*/ {399, 382, -206, 335 },
    /*400*/ {400, 368, -138, 335 },
    /*401*/ {401, 303, -69, 335 },
    /*402*/ {402, 295, 0, 335 },
    /*403*/ {403, 565, 69, 335 },
    /*404*/ {404, 240, 138, 335 },
    /*405*/ {405, 254, 206, 335 },
    /*406*/ {406, 176, 275, 335 },
    /*407*/ {407, 191, 344, 335 },
    /*408*/ {408, 61, 413, 335 },
    /*409*/ {409, 126, 482, 335 },
    /*410*/ {410, 127, 550, 335 },
    /*411*/ {411, 49, 619, 335 },
    /*412*/ {412, 510, -592, 386 },
    /*413*/ {413, 508, -522, 386 },
    /*414*/ {414, 497, -452, 386 },
    /*415*/ {415, 566, -383, 386 },
    /*416*/ {416, 447, -313, 386 },
    /*417*/ {417, 432, -244, 386 },
    /*418*/ {418, 381, -174, 386 },
    /*419*/ {419, 369, -104, 386 },
    /*420*/ {420, 305, -35, 386 },
    /*421*/ {421, 569, 35, 386 },
    /*422*/ {422, 568, 104, 386 },
    /*423*/ {423, 253, 174, 386 },
    /*424*/ {424, 177, 244, 386 },
    /*425*/ {425, 190, 313, 386 },
    /*426*/ {426, 47, 383, 386 },
    /*427*/ {427, 125, 452, 386 },
    /*428*/ {428, 48, 522, 386 },
    /*429*/ {429, 51, 592, 386 },
    /*430*/ {430, 507, -563, 438 },
    /*431*/ {431, 506, -493, 438 },
    /*432*/ {432, 498, -422, 438 },
    /*433*/ {433, 564, -352, 438 },
    /*434*/ {434, 434, -282, 438 },
    /*435*/ {435, 380, -211, 438 },
    /*436*/ {436, 370, -141, 438 },
    /*437*/ {437, 318, -70, 438 },
    /*438*/ {438, 306, 0, 438 },
    /*439*/ {439, 252, 70, 438 },
    /*440*/ {440, 242, 141, 438 },
    /*441*/ {441, 179, 211, 438 },
    /*442*/ {442, 187, 282, 438 },
    /*443*/ {443, 189, 352, 438 },
    /*444*/ {444, 124, 422, 438 },
    /*445*/ {445, 123, 493, 438 },
    /*446*/ {446, 52, 563, 438 },
    /*447*/ {447, 505, -534, 489 },
    /*448*/ {448, 501, -463, 489 },
    /*449*/ {449, 499, -392, 489 },
    /*450*/ {450, 443, -320, 489 },
    /*451*/ {451, 436, -249, 489 },
    /*452*/ {452, 371, -178, 489 },
    /*453*/ {453, 316, -107, 489 },
    /*454*/ {454, 317, -36, 489 },
    /*455*/ {455, 307, 36, 489 },
    /*456*/ {456, 244, 107, 489 },
    /*457*/ {457, 251, 178, 489 },
    /*458*/ {458, 178, 249, 489 },
    /*459*/ {459, 188, 320, 489 },
    /*460*/ {460, 114, 392, 489 },
    /*461*/ {461, 122, 463, 489 },
    /*462*/ {462, 54, 534, 489 },
    /*463*/ {463, 504, -504, 541 },
    /*464*/ {464, 502, -432, 541 },
    /*465*/ {465, 500, -360, 541 },
    /*466*/ {466, 442, -288, 541 },
    /*467*/ {467, 379, -216, 541 },
    /*468*/ {468, 372, -144, 541 },
    /*469*/ {469, 315, -72, 541 },
    /*470*/ {470, 308, 0, 541 },
    /*471*/ {471, 249, 72, 541 },
    /*472*/ {472, 243, 144, 541 },
    /*473*/ {473, 180, 216, 541 },
    /*474*/ {474, 186, 288, 541 },
    /*475*/ {475, 115, 360, 541 },
    /*476*/ {476, 120, 432, 541 },
    /*477*/ {477, 121, 504, 541 },
    /*478*/ {478, 503, -400, 592 },
    /*479*/ {479, 441, -328, 592 },
    /*480*/ {480, 437, -255, 592 },
    /*481*/ {481, 378, -182, 592 },
    /*482*/ {482, 373, -109, 592 },
    /*483*/ {483, 314, -36, 592 },
    /*484*/ {484, 309, 36, 592 },
    /*485*/ {485, 245, 109, 592 },
    /*486*/ {486, 181, 182, 592 },
    /*487*/ {487, 185, 255, 592 },
    /*488*/ {488, 116, 328, 592 },
    /*489*/ {489, 57, 400, 592 },
    /*490*/ {490, 440, -368, 644 },
    /*491*/ {491, 438, -294, 644 },
    /*492*/ {492, 377, -221, 644 },
    /*493*/ {493, 374, -147, 644 },
    /*494*/ {494, 313, -74, 644 },
    /*495*/ {495, 310, 0, 644 },
    /*496*/ {496, 248, 74, 644 },
    /*497*/ {497, 250, 147, 644 },
    /*498*/ {498, 182, 221, 644 },
    /*499*/ {499, 184, 294, 644 },
    /*500*/ {500, 119, 368, 644 },
    /*501*/ {501, 439, -260, 695 },
    /*502*/ {502, 376, -186, 695 },
    /*503*/ {503, 375, -112, 695 },
    /*504*/ {504, 312, -37, 695 },
    /*505*/ {505, 311, 37, 695 },
    /*506*/ {506, 247, 112, 695 },
    /*507*/ {507, 246, 186, 695 },
    /*508*/ {508, 183, 260, 695 }
    };
}
