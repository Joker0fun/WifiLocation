package com.chigoo.wifilocation;

import android.app.Activity;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.widget.TextView;
import com.chigoo.wifilocation.ChigooWifiLocation;
import com.chigoo.wifilocation.ChigooWifiLocation.LocationInfo;

import java.util.ArrayList;

public class MainActivity extends Activity {
    private final String TAG = "WifiLocation";
    private final int MSG_LOCATION = 0;
    private ChigooWifiLocation chigooWifiLocation;
    private LocationInfo locationInfo;
    TextView tv;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case MSG_LOCATION:
                    String data = (String)msg.obj;
                    tv.setText(data);
                    break;
                default:
                    break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        locationInfo = new LocationInfo() {
            @Override
            public void GetLocationInfo(String outJson) {
                Log.d(TAG, "" + outJson);
                Message message = new Message();
                message.what = MSG_LOCATION;
                message.obj = outJson;
                mHandler.sendMessage(message);
            }
        };

        // Example of a call to a native method
        tv = (TextView) findViewById(R.id.sample_text);

        chigooWifiLocation = new ChigooWifiLocation(this);
        chigooWifiLocation.setLocationInfo(locationInfo);
        chigooWifiLocation.init(null);
        chigooWifiLocation.startLocation(5);

    }

}
