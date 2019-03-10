#include <jni.h>
#include <string.h>
#include <ctype.h>
#include<stdio.h>
#include<android/log.h>

extern "C"
jlong Java_com_example_myapplication_Utils_gci( JNIEnv* env,
                                                  jobject thiz )
{
    return 4*5; /*To-do change this to factors of new keyid*/
}

extern "C" jbyteArray Java_com_example_myapplication_Utils_ss(JNIEnv* env,
                                                         jobject thiz, jstring str1, jint size){
    char *tempArray = new char[size];
    const char *temp = env->GetStringUTFChars(str1, NULL);
    strncpy(tempArray, temp, size);
    for(int i =0;i < size;i++)
    {
        char ch = tempArray[i];
        if (!isalpha(ch)){
            continue;
        }

        if(((ch >= 'a')&&(ch < 'n'))||((ch >= 'A') && (ch < 'N')))
        {
            tempArray[i] = ch + 13;
        }
        else
        {
            tempArray[i] = ch -13;
        }
    }
    jbyteArray outBuf = env->NewByteArray(size);
    env->SetByteArrayRegion(outBuf, 0, size, (jbyte*)tempArray);
    return outBuf;
}

extern "C" jbyteArray Java_com_example_myapplication_Utils_dks(JNIEnv* env,
                                                                       jobject thiz){
    int array[] = {116, 117, 122, 116, 47, 79, 86, 106, 65, 103, 77, 66, 65, 65, 69, 67, 103, 103, 69, 65, 67, 57, 84, 50, 51, 48, 85, 117, 73, 50, 53, 87, 49, 104, 117, 72, 88, 100, 87, 84, 98, 55, 110, 47, 118, 85, 73, 119, 55, 83, 83, 121, 84, 118, 104, 102, 68, 115, 87, 86, 107, 98, 43, 53, 10, 49, 43, 105, 57, 111, 100, 53, 83, 69, 83, 114, 86, 104, 55, 57, 115, 68, 82, 47, 110, 52, 78, 69, 107, 116, 56, 98, 108, 72, 53, 117, 108, 119, 74, 51, 103, 80, 79, 56, 87, 51, 52, 113, 65, 82, 72, 79, 82, 88, 50, 84, 78, 74, 103, 120, 98, 112, 97, 100, 50, 51, 49, 114, 89, 10, 101, 107, 117, 106, 43, 104, 87, 50, 97, 116, 70, 65, 54, 97, 69, 79, 48, 75, 43, 66, 119, 43, 55, 76, 55, 116, 119, 114, 115, 54, 106, 56, 112, 103, 76, 82, 52, 100, 49, 76, 90, 50, 101, 98, 89, 122, 50, 72, 87, 72, 87, 117, 73, 48, 54, 115, 50, 112, 67, 78, 86, 78, 121, 77, 10, 83, 77, 110, 53, 57, 51, 89, 103, 102, 109, 122, 78, 111, 116, 97, 111, 74, 51, 100, 72, 65, 119, 75, 71, 56, 80, 117, 78, 82, 72, 104, 48, 113, 68, 113, 113, 76, 110, 117, 48, 53, 55, 52, 100, 88, 107, 84, 70, 107, 69, 101, 80, 99, 101, 100, 121, 122, 122, 97, 48, 48, 55, 73, 121, 10, 67, 111, 78, 84, 85, 89, 74, 120, 68, 79, 55, 97, 74, 98, 84, 81, 80, 86, 121, 113, 109, 55, 109, 79, 101, 119, 104, 49, 83, 89, 85, 100, 72, 83, 111, 102, 99, 102, 65, 76, 108, 67, 57, 101, 84, 50, 110, 110, 54, 43, 99, 57, 113, 82, 103, 71, 79, 51, 65, 119, 98, 67, 56, 86, 10, 115, 57, 84, 109, 77, 82, 57, 43, 68, 87, 102, 72, 114, 85, 109, 80, 78, 56, 65, 75, 66, 52, 89, 75, 111, 118, 47, 81, 71, 85, 118, 50, 57, 115, 118, 73, 48, 100, 56, 89, 119, 81, 75, 66, 103, 81, 68, 84, 111, 98, 115, 68, 83, 73, 57, 54, 120, 109, 50, 115, 43, 78, 110, 69, 10};
    int size = sizeof(array) / sizeof(array[0]);
    char* buf = new char[size];
   for(int i=0; i<size; i++ ){
       buf[i] = array[i];
   }
   jbyteArray result = env->NewByteArray(size);
   env->SetByteArrayRegion(result,0,size,(jbyte*)buf);
    return result;

}

