package com.wallerlab.processing.tasks;

import android.content.Context;

import com.wallerlab.processing.datasets.Dataset;

/**
 * Created by joelwhang on 11/6/15.
 */
public class ComputeFPMTask extends ImageProgressTask {
    public ComputeFPMTask(Context context) {
        super(context);
        this.progressDialog.setMessage("Calculating FPM...");
    }

    @Override
    protected Void doInBackground(Dataset... params) {
        // TODO Auto-generated method stub
        fpm(1);
        return null;
    }

    /** Native Functions **/
    public native void fpm(int iterations);

}

