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

/*
 * The purpose of this class is to extend the BluetoothService objects to a global context
 * see: http://stackoverflow.com/questions/12818898/android-blutoothchatservice-use-in-multiple-classes
 * 
 */

package com.wallerlab.compcellscope;

import android.app.Application;

import com.wallerlab.compcellscope.bluetooth.BluetoothService;
import com.wallerlab.processing.datasets.Dataset;

public class GlobalApplicationClass extends Application {
    private BluetoothService mBluetoothService;
    private Dataset mDataset;
    
    public GlobalApplicationClass() {
        super();
    }

    @Override
    public void onCreate() {
        super.onCreate();
    }

    public BluetoothService getBluetoothService() {
        return mBluetoothService;
    }

    public void setBluetoothService(BluetoothService mBluetoothService) {
        this.mBluetoothService = mBluetoothService;
    }
    
    public Dataset getDataset() {
    	return mDataset;
    }
    
    public void setDataset(Dataset newDataset)
    {
       this.mDataset = newDataset;
    }
}
