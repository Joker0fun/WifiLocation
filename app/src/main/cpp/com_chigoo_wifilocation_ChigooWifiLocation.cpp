//
// Created by zoe on 18-6-21.
//

#include "com_chigoo_wifilocation_ChigooWifiLocation.h"

#include <time.h>
#include "IpsLocate.h"
#include "typesdef.h"
#include <android/log.h>

#define TAG "WIFI-LOC-JNI"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)


IpsLocate Ips;

unsigned long GetTickCount()
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

char* jstringToChar(JNIEnv* env, jstring jstr) {
    char* rtn = NULL;
    jclass clsstring = env->FindClass("java/lang/String");
    jstring strencode = env->NewStringUTF("GB2312");
    jmethodID mid = env->GetMethodID(clsstring, "getBytes", "(Ljava/lang/String;)[B");
    jbyteArray barr = (jbyteArray) env->CallObjectMethod(jstr, mid, strencode);
    jsize alen = env->GetArrayLength(barr);
    jbyte* ba = env->GetByteArrayElements(barr, JNI_FALSE);
    if (alen > 0) {
        rtn = (char*) malloc(alen + 1);
        memcpy(rtn, ba, alen);
        rtn[alen] = 0;
    }
    env->ReleaseByteArrayElements(barr, ba, 0);
    return rtn;
}

JNIEXPORT jstring JNICALL Java_com_chigoo_wifilocation_ChigooWifiLocation_Locate
        (JNIEnv *env, jobject obj, jobject scanList,jobject  accelerometer)
{
    string out;
    string strUser = "test0001";
    vector<APRssi> vecAPRssi;

    //获得class ArrayList
    jclass cls_arraylist = env->GetObjectClass(scanList);

    //method in class ArrayList
    jmethodID arraylist_get = env->GetMethodID(cls_arraylist,"get","(I)Ljava/lang/Object;");
    jmethodID arraylist_size = env->GetMethodID(cls_arraylist,"size","()I");

    jint len = env->CallIntMethod(scanList, arraylist_size);
    LOGD("Get ArrayList<ChigooWifiInfo> size = %d!", len);

    jclass  cls_cvector = env->GetObjectClass( accelerometer);
    jmethodID ChigooCVector_getvx = env->GetMethodID(cls_cvector, "getVx", "()Ljava/lang/String;");
    jmethodID ChigooCVector_getvy = env->GetMethodID(cls_cvector, "getVy", "()Ljava/lang/String;");
    jmethodID ChigooCVector_getvz = env->GetMethodID(cls_cvector, "getVz", "()Ljava/lang/String;");

    jdouble vx = env->CallDoubleMethod(cls_cvector,ChigooCVector_getvx);
    jdouble vy = env->CallDoubleMethod(cls_cvector,ChigooCVector_getvy);
    jdouble vz = env->CallDoubleMethod(cls_cvector,ChigooCVector_getvz);

    CVector3* getmoves = new CVector3(vx,vy,vz);


    for (int i = 0; i < len; i++) {
        jobject obj_ChigooWifiInfo = env->CallObjectMethod(scanList, arraylist_get, i);
        jclass cls_ChigooWifiInfo = env->GetObjectClass(obj_ChigooWifiInfo);
        jmethodID ChigooWifiInfo_getMac = env->GetMethodID(cls_ChigooWifiInfo, "getMac", "()Ljava/lang/String;");
        if (ChigooWifiInfo_getMac == NULL) {
            LOGE("Get method getMac Fail !!!");
        }

        jmethodID ChigooWifiInfo_getRssi = env->GetMethodID(cls_ChigooWifiInfo, "getRssi", "()I");
        if (ChigooWifiInfo_getRssi == NULL) {
            LOGE("Get method getRssi Fail !!!");
        }


        jstring mac = (jstring)env->CallObjectMethod(obj_ChigooWifiInfo, ChigooWifiInfo_getMac);
        if (mac == NULL) {
            LOGE("Get mac Fail !!!");
        }

        char* charMac = jstringToChar(env, mac);
        if (charMac == NULL) {
            LOGE("Get charMac Fail !!!");
        }
        string strMac = charMac;

        jint rssi = env->CallIntMethod(obj_ChigooWifiInfo, ChigooWifiInfo_getRssi);
        if (rssi == 0) {
            LOGE("Get Rssi Fail !!!");
        }

        APRssi apRssi;
        apRssi.mac = strMac;
        apRssi.rssi = rssi;
        vecAPRssi.push_back(apRssi);
    }

    char isMove = 1;// add CVector3
    char isSys = 1;
    time_t timestamp = GetTickCount();
    char K = 4;
    LOGD("Enter into Locate");
    jclass WifiInfoCls = env->FindClass("com/chigoo/wifilocation/ChigooWifiInfo");
    if (WifiInfoCls != NULL) {
        LOGD("Find WIFI INFO CLASS");
    } else {
        LOGE("On Find Class");
    }

    out = Ips.Locate(strUser, vecAPRssi, isMove, isSys, timestamp, K);
    const char* const_out = out.data();
    jstring str = env->NewStringUTF(const_out);
    delete getmoves ;
    return str;
}