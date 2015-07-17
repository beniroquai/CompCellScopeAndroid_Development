package com.wallerlab.processing.tasks;

import android.content.Context;

import com.wallerlab.processing.datasets.Dataset;

public class ComputeDPCRefocusTask extends ImageProgressTask {
	Dataset mDataset;

	public ComputeDPCRefocusTask(Context context) {
        super(context);
        this.progressDialog.setMessage("Calculating Refocus Stack from DPC Data...");
    }
	
	@Override
	protected Void doInBackground(Dataset... params) {
		// TODO Auto-generated method stub
		mDataset = params[0];
		mDataset.DPC_FOCUS_DATASET_ROOT = "/storage/emulated/0/CellScope/Full_Scan_Dataset-Sample";
		computeDPCRefocus(mDataset.DPC_ZMIN, mDataset.DPC_ZSTEP, mDataset.DPC_ZMAX, mDataset.DPC_FOCUS_DATASET_ROOT);
		return null;
	}
	
	/** Native Functions **/
	public native void computeDPCRefocus(float zMin, float zStep, float zMax, String datasetRoot);

}
