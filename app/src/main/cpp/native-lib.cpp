#include <jni.h>
#include <string>
#include <android/log.h>

#define TAG "NativeDemo"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)

// Native add function
int nativeAdd(int a, int b) {
    return a + b;
}

// Native message builder
std::string nativeGreet(const std::string& name) {
    return "Hello, " + name + "! Welcome from C++.";
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_betcpt_server_MainActivity_getGreeting(JNIEnv* env, jobject /* this */) {
    std::string result = nativeGreet("Kotlin User");
    LOGI("Greeting generated: %s", result.c_str());
    return env->NewStringUTF(result.c_str());
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_betcpt_server_MainActivity_addNumbers(JNIEnv* env, jobject /* this */, jint a, jint b) {
    int sum = nativeAdd(a, b);
    LOGI("Add %d + %d = %d", a, b, sum);
    return sum;
}
