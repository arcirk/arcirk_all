package ru.arcirk.lscanner.qtandroidservice;

import android.content.Context;
import android.content.Intent;
import android.util.Log;
import org.qtproject.qt.android.bindings.QtService;
import android.content.IntentFilter;
import android.os.IBinder;

public class QtAndroidService extends QtService{
    private static final String TAG = "QtAndroidService";

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG, "Creating Service");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.i(TAG, "Destroying Service");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        int ret = super.onStartCommand(intent, flags, startId);

        String name = new String(intent.getByteArrayExtra("name"));
        Intent sendToUiIntent = new Intent();
        sendToUiIntent.setAction(ActivityUtils.BROADCAST_NAME_ACTION);
        sendToUiIntent.putExtra("name", name);
        Log.i(TAG, "Service sending broadcast");
        sendBroadcast(sendToUiIntent);

        return ret;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return super.onBind(intent);
    }

}
