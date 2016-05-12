package com.wallerlab.processing.tasks;

import android.content.Context;

import com.wallerlab.compcellscope.dialogs.DirectoryChooserDialog;
import com.wallerlab.processing.datasets.Dataset;

import android.content.Intent;
import android.content.SharedPreferences;
import android.util.Log;

public class ComputeDPCRefocusTask extends ImageProgressTask {
	Context thisContext;
	public ComputeDPCRefocusTask(Context context) {
        super(context);
		thisContext = context;
		Log.i("testing fpm", context.toString());
        this.progressDialog.setMessage("Calculating Refocus Stack from DPC Data...");
		chooseDirectory();
    }

	@Override
	protected Void doInBackground(Dataset... params) {
		// TODO Auto-generated method stub
		String dataset = "/storage/emulated/0/CellScope/FPM_Sample";

//		chooseDirectory();
		return null;
	}
	public void chooseDirectory()
	{
		final String m_chosenDir = "";
		boolean m_newFolderEnabled = true;
//		final SharedPreferences settings = getSharedPreferences(PREFS_NAME, 0);

		// Create DirectoryChooserDialog and register a callback
		DirectoryChooserDialog directoryChooserDialog =
				new DirectoryChooserDialog(thisContext,
						new DirectoryChooserDialog.ChosenDirectoryListener()
						{
							@Override
							public void onChosenDir(String chosenDir)
							{
								Log.i("fpm testing", chosenDir);
								computeFPM(1);
							}
						});
		// Toggle new folder button enabling
//        directoryChooserDialog.setNewFolderEnabled(m_newFolderEnabled);
		// Load directory chooser dialog for initial 'm_chosenDir' directory.
		// The registered callback will be called upon final directory selection.
//        directoryChooserDialog.chooseDirectory(m_chosenDir);
	}
	
	/** Native Functions **/
//	public native void computeDPCRefocus(float zMin, float zStep, float zMax, String datasetRoot);

	public native int computeFPM(int iterations);

}
