package com.wallerlab.compcellscope;

/**
 * Created by joelwhang on 7/9/15.
 */

import android.app.Activity;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.ImageView;
import android.graphics.Bitmap;
import android.graphics.drawable.Drawable;

//import opencv libraries
import org.opencv.core.Core;
import org.opencv.core.CvType;
import org.opencv.core.Mat;
import org.opencv.core.Scalar;
import org.opencv.imgproc.Imgproc;

import com.wallerlab.processing.utilities.ImageUtils;

public class TestViewerActivity extends Activity{

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.tester);

        Button qDPCButton = (Button)findViewById(R.id.qDPCButton);

        qDPCButton.setOnClickListener(
                new OnClickListener() {
                    @Override
                    public void onClick(View v) {
                        Log.i("TestQDPC", "clicked");


                        ImageView iv = (ImageView) findViewById(R.id.imageTransformed);
                        int imageResource = R.drawable.transfer135;
                        Drawable image = getResources().getDrawable(imageResource);
                        Log.i("TestQDPC","first hello here");
                        Log.i("TestQDPC", Integer.toString(image.getIntrinsicHeight()));
                        Log.i("TestQDPC", "hello there?");
                        iv.setImageDrawable(image);
                    }
                }
        );
    }

    public Drawable callqDPC() {
//        Bitmap bm;
        Drawable top, bottom, left, right, transfer45, transfer135;
        top = getResources().getDrawable(getResources().getIdentifier("dt", "zdrawable", getPackageName()));
        return top;


//        return bm;
    }




}
