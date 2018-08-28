package com.chigoo.wifilocation;

import android.content.Context;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.WifiLock;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by zoe on 18-6-21.
 */

public class ChigooWifiLocation{

    private WifiManager mWifiManager;
    private WifiInfo mWifiInfo;
    private List<ScanResult> mWifiList;
    private Context mContext;
    private ScanThread mScanThread;
    private LocationInfo locationInfo;

    private ArrayList<ChigooWifiInfo> mWifiInfoArray;


    // Used to load the 'native-lib' library on application startup.
    //static {

    //}

    public ChigooWifiLocation(Context context) {
        System.loadLibrary("ChigooWifiLocation");
        mContext = context;
        mScanThread = new ScanThread();
        mWifiInfoArray = new ArrayList<>();
    }

    public void setLocationInfo(LocationInfo listener)
    {
        locationInfo = listener;
    }

    public void init(String ConfigFilePath) {
    //Open Wifi
        mWifiManager = (WifiManager)mContext.getSystemService(Context.WIFI_SERVICE);
        mWifiInfo = mWifiManager.getConnectionInfo();
        if (!mWifiManager.isWifiEnabled()) {
            mWifiManager.setWifiEnabled(true);
        }

    }

    public void deinit() {
        if (mWifiManager.isWifiEnabled()) {
            mWifiManager.setWifiEnabled(false);
        }

    }

    public void startLocation(int frequence) {
        mScanThread.setmSleepTime(frequence);
        mScanThread.setThreadRunning(true);
        mScanThread.start();
    }

    public void stopLocation() {
        mScanThread.setThreadRunning(false);
        mScanThread.interrupt();

    }

    class ScanThread extends Thread {
        private boolean isThreadRunning;
        private int mSleepTime;

        public void setThreadRunning(boolean threadRunning) {
            isThreadRunning = threadRunning;
        }

        public void setmSleepTime(int mSleepTime) {
            this.mSleepTime = mSleepTime;
        }

        public boolean Stoped(){
            //判断线程是否要停止
            return (isThreadRunning==true) ? false : true;
        }

        @Override
        public void run() {
            while(!Stoped() && !interrupted()) {
                mWifiInfoArray.clear();
                mWifiManager.startScan();
                mWifiList = mWifiManager.getScanResults();
                for (ScanResult result : mWifiList) {
                    String mac = result.BSSID;
                    int rssi = result.level;
                    ChigooWifiInfo info = new ChigooWifiInfo(mac, rssi);
                    mWifiInfoArray.add(info);
                }
                String out = Locate(mWifiInfoArray);
                locationInfo.GetLocationInfo(out);
                try {
                    sleep(1000 * mSleepTime);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public interface LocationInfo {
        public void GetLocationInfo(String outJson);
    }


    //jni interface
    public native String Locate(ArrayList<ChigooWifiInfo> infoArrayList);
}
