#include <jni.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <thread>
#include <android/log.h>

#define TAG "SocketServer"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#define PORT 1024  // must be > 1024 for non-root

void socketServer() {
    LOGI("Starting socket server...");

    int server_fd, client_fd;
    struct sockaddr_in address{};
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        LOGE("Socket creation failed");
        return;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        LOGE("Bind failed (port %d)", PORT);
        close(server_fd);
        return;
    }

    if (listen(server_fd, 1) < 0) {
        LOGE("Listen failed");
        close(server_fd);
        return;
    }

    LOGI("Listening on port %d...", PORT);

    while (true) {
        client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (client_fd < 0) {
            LOGE("Accept failed");
            continue;
        }

        LOGI("Client connected");

        char buffer[1024] = {0};
        int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            LOGI("Received: %s", buffer);
            std::string reply = std::string("Echo: ") + buffer;
            send(client_fd, reply.c_str(), reply.length(), 0);
        }

        close(client_fd);
    }

    close(server_fd);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_betcpt_server_MainActivity_startSocketServer(JNIEnv *, jobject) {
    std::thread(socketServer).detach();
    LOGI("Server thread started");
}
