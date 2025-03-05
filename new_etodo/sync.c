#include <json-c/json_object.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <json-c/json.h>
#include <string.h>
#include <unistd.h>

void transfer_list(json_object* jobj, char* ip, int port) {
    struct sockaddr_in addr = {
        AF_INET,
        htons(port)
    };
    inet_pton(AF_INET, ip, &addr.sin_addr);
    socklen_t addr_size = sizeof(addr);
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sock, (struct sockaddr*)&addr, addr_size) == -1) return;

    const char* buff = json_object_to_json_string_ext(jobj, JSON_C_TO_STRING_PRETTY);
    size_t buff_size = strlen(buff);
    // send the json string
    while (buff_size) {
        int n = write(sock, buff, buff_size);
        buff+=n;
        buff_size -= n;
    }
    // close connection
    close(sock);
}