#include <jni.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <netinet/in.h>
#include <unistd.h>
#include <android/log.h>
#include <arpa/inet.h>

#define TAG "NativeServer"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define PORT 1024

JavaVM* g_vm;
jobject g_activity = nullptr;

std::mutex clients_mutex;
std::vector<int> clientSockets;
std::vector<std::string> clientDescs;

void notifyClientConnected(const std::string& desc) {
    JNIEnv* env;
    g_vm->AttachCurrentThread(&env, nullptr);
    jclass cls = env->GetObjectClass(g_activity);
    jmethodID mid = env->GetMethodID(cls, "onClientConnected", "(Ljava/lang/String;)V");
    jstring jdesc = env->NewStringUTF(desc.c_str());
    env->CallVoidMethod(g_activity, mid, jdesc);
    env->DeleteLocalRef(jdesc);
}

void notifyClientDisconnected(int index) {
    JNIEnv* env;
    g_vm->AttachCurrentThread(&env, nullptr);
    jclass cls = env->GetObjectClass(g_activity);
    jmethodID mid = env->GetMethodID(cls, "onClientDisconnected", "(I)V");
    env->CallVoidMethod(g_activity, mid, index);
}

void handleClient(int clientSocket, std::string clientDesc, int index) {
    char buffer[2048];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int len = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (len <= 0) break;
        LOGI("[Client %s] -> %s", clientDesc.c_str(), buffer);
    }

    close(clientSocket);
    std::lock_guard<std::mutex> lock(clients_mutex);
    clientSockets[index] = -1;
    notifyClientDisconnected(index);
}

extern "C" JNIEXPORT void JNICALL
Java_com_betcpt_server_MainActivity_startServer(JNIEnv* env, jobject thiz) {
    env->GetJavaVM(&g_vm);
    g_activity = env->NewGlobalRef(thiz);

    std::thread([] {
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(PORT);

        bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
        listen(serverSocket, 5);
        LOGI("Server started on port %d", PORT);

        while (true) {
            sockaddr_in clientAddr{};
            socklen_t addrLen = sizeof(clientAddr);
            int clientSock = accept(serverSocket, (sockaddr*)&clientAddr, &addrLen);
            if (clientSock < 0) continue;

            std::string ip = inet_ntoa(clientAddr.sin_addr);
            int port = ntohs(clientAddr.sin_port);
            std::string desc = ip + ":" + std::to_string(port);

            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                clientSockets.push_back(clientSock);
                clientDescs.push_back(desc);
            }

            notifyClientConnected(desc);
            std::thread(handleClient, clientSock, desc, clientSockets.size() - 1).detach();
        }
    }).detach();
}

extern "C" JNIEXPORT void JNICALL
Java_com_betcpt_server_MainActivity_sendCommandTo(JNIEnv* env, jobject, jint index, jstring cmd) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    if (index >= 0 && index < clientSockets.size() && clientSockets[index] != -1) {
        const char* c_cmd = env->GetStringUTFChars(cmd, nullptr);
        send(clientSockets[index], c_cmd, strlen(c_cmd), 0);
        env->ReleaseStringUTFChars(cmd, c_cmd);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_betcpt_server_MainActivity_sendCommandToAll(JNIEnv* env, jobject, jstring cmd) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    const char* c_cmd = env->GetStringUTFChars(cmd, nullptr);
    for (int sock : clientSockets) {
        if (sock != -1) {
            send(sock, c_cmd, strlen(c_cmd), 0);
        }
    }
    env->ReleaseStringUTFChars(cmd, c_cmd);
}
