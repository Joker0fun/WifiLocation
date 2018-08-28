package com.chigoo.wifilocation;

/**
 * Created by zoe on 18-6-22.
 */

public class ChigooWifiInfo {
    private String mac;
    private int rssi;

    public ChigooWifiInfo() {}

    public ChigooWifiInfo(String mac, int rssi) {
        this.mac = mac;
        this.rssi = rssi;
    }

    public String getMac() {
        return mac;
    }


    public int getRssi() {
        return rssi;
    }
}
