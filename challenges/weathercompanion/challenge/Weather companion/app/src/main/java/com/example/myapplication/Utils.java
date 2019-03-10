package com.example.myapplication;

import android.content.Context;
import android.os.Build;
import android.support.annotation.RequiresApi;
import android.util.Log;
import java.util.Base64;
import java.io.IOException;
import java.math.BigInteger;
import org.apache.commons.codec.binary.Base32;

public class Utils {
    private final Context appContext;
    private final int firstFactor = 19;
    private final long secondFactor = 305363017965794407L;
    int projectChars[] = {98,115,105,100,101,115,45,115,102,45,99,116,102,45,50,48,49,57};
    private final String authProvider = "aHR0cHM6Ly93d3cuZ29vZ2xlYXBpcy5jb20vb2F1dGgyL3YxL2NlcnRz";
    private final String certUrl = "aHR0cHM6Ly93d3cuZ29vZ2xlYXBpcy5jb20vcm9ib3QvdjEvbWV0YWRhdGEveDUwOS93ZWF0aGVyLWNvbXBhbmlvbi1zZXJ2aWNlLWFjY28lNDBic2lkZXMtc2YtY3RmLTIwMTkuaWFtLmdzZXJ2aWNlYWNjb3VudC5jb20=";
    private final String authUri = "uggcf://nppbhagf.tbbtyr.pbz/b/bnhgu2/nhgu";
    private final String tokenUri = "uggcf://bnhgu2.tbbtyrncvf.pbz/gbxra";
    private final String part1 = "LS0tLS1CRUdJTiBQUklWQVRFIEtFWS0tLS0tCk1JSUV2QUlCQURBTkJna3Foa2lHOXcwQkFRRUZBQVNDQktZd2dnU2lBZ0VBQW9JQkFRQ2JOYUpYN3FaMlNlYzQKNVc0aXIreVlYSjNJd2paOGZ3eXcwUFpTb2lUYi9pSnFUY0trL2x0alA2MVRySkhCNU1xS202VnovV0d3N0dTbQpuZDIxeE1GcWNsd3VHOE43Zit6aElLMFh1dlJCclMrY01FaHcwUmJITlViY08zWmFnaEtmZmFMVGh6UFk0eDJyCkhoL044bFVZaTRUQjhXQUdhQXpDSkgzcFVpOXJURzQrdWN4TzZwTU56My9FTnp5T1NtaWhSOVhiNU9rTVkvQXEKN05KN0JBd0g4b1NXUmxUbGxhQzhDaGw4d0RkdW42ZktXZ1lZbW1jQldYU2pyb3B1OGY2TVI3dk1NM0YwUEV1YQpZQnoxWnB4VnZZYzFpaU1TMFdpTnRBYTFaQnVXaFJlbkFpVGZ3T0N0NHJJU2ZiZ1lTcUNUU29LNXRPV1M1MnRNCg==";
    private final String part3 = "CnoM+J76ft/1b1qCH4j4+/0h/EHG+iWbhzfijs5a7XHKINC8ach6YLbhAyUm9+mI\nGuGVAklGeeN5InzSubRcrL8BSobAIygkXw9aSioF2wj2TYMDYncA0KVLR0nkEAWf\nPFlp2YQLIIw0nowmk0PPSp0F+DXOtDP7i7xcFZWzVjy7ScWz/G9bnxqilMAmg3MH\n+QGEzCKu/JZ5p6w9iqmTXd3VyzUI/FFmIHsjDnkS8ImDyNv69seh/QFgG8u7PcTq\nYRm8F0dd7hoof7tBQX/MejFDui8qbrtMTh0agHEsnIY7NalXTWId2FYurIuZuWrM\nz94zTZR2BjXOtSKSFJpTETuZ/JkXTiNT0WJgTjMgawj+eNpEMSMNNcsSdVWSuKAr\n";
    private final String part4 = "M5VXSRDXNBVGCZDWOBUWCWJYG52FAK3BOIYU2VSYORSVGMLRINEW2YSHMZREO6KLJV3SWNLPKEXWY5KINRMHGOLWKFTUO53ILFGVCWAKNJCVGNTTJ5ITKNCJMREU24SPINEG4Q2WMZ5GWMDSONKHM22KPFFWQMKNGJDTANKDJFHUERRXJZUVSRZVKBSFEQSUGVAW6R2BIJCXOVAKIZHHEUTFIR5DSRDQIFYHGYLOINHGGV2ZHFIG6TKJMUZWYQZTKRJTIS3LG5WDG6KSJZHE44ZTONEGY5BXJZYVQ5DKPFKEKK2LHAZS6VIKJVQSWUCIMRYTAWKTJBBVCV2VGM2VE2DPOJCDOVCPHEZDERBTG5TUMRCZGA4DA6LDNBBEYTJZGZLGC42RKRGWKZSKMRCEQU2VJYYTO2IKLJZEURDBNR2WS6DQKA3S6YRQOFKGYUCCHFFDQ5KZNJEDE5DMIZLSWRSCIF2TS22DM5MUEY3IHBLHM6KCPJWEOYLZGVZDAN3KN5JGU5IKMNSG4VTGOJDUCVLELJJEY6CBGVUHESLBJN3EWRTZGI4EGYLBJNLC63C2JZETSTTNMRNG45DKGZQUSUJQOJ5FKV3JMRHVC43NNIZG6NAKG5LGGQLSNQ2VKTTVOJUHQNDVNBTTER2DNRRVKSKILE2VSZCFPBEGMRLVNJBHKNTVJVIWIWTKIJYFQOBXOVXWCZ3HIM3WU52KMFXFCQYKNRAXAVCQMMZTINZYM4XW22DPHFJTKZKEJV3T2PIKFUWS2LJNIVHEIICQKJEVMQKUIUQEWRKZFUWS2LJNBI======";
    public Utils(Context appContext) throws IOException {
        this.appContext = appContext;
    }

    static{
        System.loadLibrary("native-lib");
    }

    private native long gci();
    private native byte[] ss(String str1, int size);
    private native byte[] dks();

    @RequiresApi(api = Build.VERSION_CODES.O)
    String getKey(){
    String keyPart1 = decodeString(part1,0);
    String keyPart2 = new String(dks());
    String keyPart3 = new String(ss(part3,part3.length()));
    String keyPart4 = decodeString(part4,1);
    String key = keyPart1 + keyPart2 + keyPart3 + keyPart4;



    String json = "{\"type\": \"" + appContext.getResources().getString(R.string.type) + "\",";
    json = json + "\"project_id\": \"" + getchars(projectChars) + "\",";
    json = json + "\"private_key_id\": \"" + getKeyId() + "\",";
    json = json + "\"private_key\": \"" + key.replace("\n","\\n") + "\",";
    String client_email = "weather-companion-service-acco@bsides-sf-ctf-2019.iam.gserviceaccount.com";
    json = json + "\"client_email\": \"" + client_email + "\",";
    json = json + "\"client_id\": \"" + multiplyFactors()  + "\",";
    json = json + "\"auth_uri\": \"" + new String(ss(authUri, authUri.length())) + "\",";
    json = json + "\"token_uri\": \"" + new String(ss(tokenUri, tokenUri.length())) + "\",";
    json = json + "\"auth_provider_x509_cert_url\": \"" + decodeString(authProvider,0) + "\",";
    json = json + "\"client_x509_cert_url\" : \"" + decodeString(certUrl,0) + "\"}";
    return json;
    }
    @RequiresApi(api = Build.VERSION_CODES.O)
    public String decodeString(String input, int type){
        byte[] decoded = null;
        if(type == 0) {
            decoded = Base64.getDecoder().decode(input);
        }
        else {
            Base32 base32 = new Base32();
            decoded = base32.decode(input.getBytes());
        }
        return new String(decoded);
    }
    public String multiplyFactors(){
        BigInteger bi1 = BigInteger.valueOf(gci());
        bi1 = bi1.multiply(BigInteger.valueOf(firstFactor));
        bi1 = bi1.multiply(BigInteger.valueOf(secondFactor));
        return bi1.toString();
    }
    public String getchars(int[] inputArray){
        String projectId = "";
        for(int i=0;i<inputArray.length;i++){
            projectId = projectId + (char)inputArray[i];
        }
        return projectId;
    }
    public String getKeyId(){
        String keyId = "";
        BigInteger keyIdDec = new BigInteger("627096631258101466300448072738386213700396112265");
        keyId = keyIdDec.toString(16);
        return keyId;
    }
}

