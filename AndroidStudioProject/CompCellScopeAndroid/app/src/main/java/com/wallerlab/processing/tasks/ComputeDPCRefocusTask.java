package com.wallerlab.processing.tasks;

import android.content.Context;

import com.wallerlab.processing.datasets.Dataset;

public class ComputeDPCRefocusTask extends ImageProgressTask {

	public ComputeDPCRefocusTask(Context context) {
        super(context);
        this.progressDialog.setMessage("Calculating Refocus Stack from DPC Data...");
    }
	
	@Override
	protected Void doInBackground(Dataset... params) {
		// TODO Auto-generated method stub
		String dataset = "/storage/emulated/0/CellScope/FPM_Sample";
		computeFPM(dataset, 10);
		return null;
	}
	
	/** Native Functions **/
//	public native void computeDPCRefocus(float zMin, float zStep, float zMax, String datasetRoot);

	public native int computeFPM(String dataset, int iterations);

}
