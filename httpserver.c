#include "bind.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <err.h>
#include <unistd.h>
#include "queue.h"
#include <pthread.h>

#define BLOCK 4096
#define DEFAULT_THREAD_COUNT 4

const char *getResponse(int code) {
    switch (code) {
    case 200: return "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nOK";
    case 201: return "HTTP/1.1 201 Created\r\nContent-Length: 8\r\n\r\nCreated";
    case 400: return "HTTP/1.1 400 Bad Request\r\nContent-Length: 12\r\n\r\nBad Request";
    case 403: return "HTTP/1.1 403 Forbidden\r\nContent-Length: 10\r\n\r\nForbidden";
    case 404: return "HTTP/1.1 404 Not Found\r\nContent-Length: 10\r\n\r\nNot Found";
    case 500:
        return "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 22\r\n\r\nInternal Server "
               "Error";
    case 501: return "HTTP/1.1 501 Not Implemented\r\nContent-Length: 16\r\n\r\nNot Implemented";
    }
    return NULL;
}

//writes to audit log
//only called when client socket and fd_log is valid and alive
void writeLog(int thread_id, int fd_log, char *operation, char *uri, int status, int req_id,
    queue_t *log_queue) {
    char log_entry[50];
    void *queue_obj;
    while ((*(int *) queue_peek(log_queue)) != thread_id) {
        sleep(1);
    }
    sprintf(log_entry, "%s,/%s,%d,%d\n", operation, uri, status, req_id);
    write(fd_log, log_entry, strlen(log_entry));
    queue_pop(log_queue, &queue_obj);
}

//sends response header
void sendResponse(int client_socket, int status, char *buffer) {
    memset(buffer, 0, (int) sizeof(buffer));
    strcpy(buffer, getResponse(status));
    send(client_socket, buffer, strlen(buffer) + 1, 0);
}

struct thread_data {
    pthread_t id;
    bool active;
    int client_socket;
    int fd_log;
    queue_t *log_queue;
};

void *workerThreadInit(void *arg) {
    struct thread_data *t = arg;
    int client_socket = t->client_socket;
    queue_t *log_queue = t->log_queue;
    int fd_log = t->fd_log;
    int fd = 0;
    int recv_bytes = 0;
    struct stat st;
    int status = 0;
    void *queue_obj;
    char *buffer = (char *) malloc  (sizeof(char) * BLOCK);
    char *line = (char *) malloc(sizeof(char) * 2048);
    queue_push(log_queue, (void *) &t->id);

    memset(buffer, 0, (int) sizeof(buffer));
    recv_bytes = recv(client_socket, buffer, BLOCK, 0);

    if (recv_bytes <= 0) {
        status = 403;
        sendResponse(client_socket, status, buffer);
        goto exit_thread;
    }

    // parsing Method,URI,VERSION
    int i = 0; // request buffer pointer

    for (; i < recv_bytes; i++) {
        if (buffer[i] == '\r') {
            line[i] = '\0';
            i += 2;
            break;
        }
        line[i] = buffer[i];
    }

    char operation[9];
    char uri[20];
    char version[9];

    // invalid operation
    int sscanf_status = sscanf(line, "%s %s %s", operation, uri, version);

    if (!((strcmp(operation, "GET") == 0) || (strcmp(operation, "PUT") == 0)
            || (strcmp(operation, "HEAD") == 0))) {

        status = 400;
        sendResponse(client_socket, status, buffer);
        goto exit_thread;
    }

    // invalid request
    if ((sscanf_status != 3) || (strcmp(version, "HTTP/1.1") != 0) || (strstr(uri, "/") == 0)) {
        status = 400;
        sendResponse(client_socket, status, buffer);
        goto exit_thread;
    }

    // parsing headerfield
    char key[100];
    char val[100];
    int cont_length = 0;
    int req_id = 0;

    /**
      
      */
    for (int j = 0; i < recv_bytes; i++, j++) {
        if (buffer[i] == '\r') {
            line[j] = '\0';
            i += 2;
            break;
        }
        line[j] = buffer[i];
    }

    while (i < recv_bytes) {
        if ((strlen(line) > 0) && (strstr(line, ":") == 0)) {
            status = 400;
            sendResponse(client_socket, status, buffer);
            goto exit_thread;
        }
        sscanf_status = sscanf(line, "%s %s", key, val);
        if (strcmp(key, "Content-Length:") == 0) {
            cont_length = atoi(val);
        }
        if (strcmp(key, "Request-Id:") == 0) {
            req_id = atoi(val);
        }
        for (int j = 0; i < recv_bytes; i++, j++) {
            if (buffer[i] == '\r') {
                line[j] = '\0';
                i += 2;
                break;
            }
            line[j] = buffer[i];
        }
    }

    //takeoff the first '/' from uri
    int j = 1;
    for (; uri[j] != '\0'; j++) {
        uri[j - 1] = uri[j];
    }
    uri[j - 1] = uri[j];

    // GET or HEAD METHOD
    if ((strcmp(operation, "GET") == 0) || (strcmp(operation, "HEAD") == 0)) {
        memset(buffer, 0, (int) sizeof(buffer));

        // FIlE NOT FOUND
        if (access(uri, F_OK) == -1) {
            status = 404;
            sendResponse(client_socket, status, buffer);
            writeLog(t->id, fd_log, operation, uri, status, req_id, log_queue);
            goto exit_thread;
        }

        // FORBIDDEN
        if (access(uri, R_OK) == -1) {
            status = 403;
            sendResponse(client_socket, status, buffer);
            writeLog(t->id, fd_log, operation, uri, status, req_id, log_queue);
            goto exit_thread;
        }

        fd = open(uri, O_RDONLY);
        fstat(fd, &st);

        if (S_ISDIR(st.st_mode)) {
            status = 403;
            sendResponse(client_socket, status, buffer);
            writeLog(t->id, fd_log, operation, uri, status, req_id, log_queue);
            goto exit_thread;
        }

        char contentLength[10];
        status = 200;
        sprintf(contentLength, "%d", (int) st.st_size);
        strcat(buffer, "HTTP/1.1 200 OK\r\nContent-Length: ");
        strcat(buffer, contentLength);
        strcat(buffer, "\r\n\r\n");
        send(client_socket, buffer, strlen(buffer), 0);
        if (strcmp(operation, "GET") == 0) {
            memset(buffer, 0, (int) sizeof(buffer));
            int bytes_read = read(fd, buffer, BLOCK);
            while (bytes_read > 0) {
                send(client_socket, buffer, bytes_read, 0);
                memset(buffer, 0, (int) sizeof(buffer));
                bytes_read = read(fd, buffer, BLOCK);
            }
        }
        close(fd);
        writeLog(t->id, fd_log, operation, uri, status, req_id, log_queue);
        goto exit_thread;
    }

    // PUT METHOD 
    else if (strcmp(operation, "PUT") == 0) {

        //Content-Length not provided
        if (cont_length <= 0) {
            status = 400;
            sendResponse(client_socket, status, buffer);
            writeLog(t->id, fd_log, operation, uri, status, req_id, log_queue);
            goto exit_thread;
        }

        // no file exists
        if (access(uri, F_OK) == -1) {
            status = 201;
        }

        //if file exists, check for write permissions
        if ((status != 201) && (access(uri, W_OK) == -1)) {
            status = 403;
            sendResponse(client_socket, status, buffer);
            writeLog(t->id, fd_log, operation, uri, status, req_id, log_queue);
            goto exit_thread;
        }
        
        fd = open(uri, O_CREAT | O_WRONLY);

        //flush remaining bytes on buffer
        int written_bytes = write(fd, &(buffer[i]), (recv_bytes - i));
        i = 0;

        int total_recv_bytes = 0;
        do {
            if (recv(client_socket, buffer, sizeof(buffer), MSG_PEEK) <= 0) {
                break;
            }
            memset(buffer, 0, (int) sizeof(buffer));
            recv_bytes = recv(client_socket, buffer, BLOCK, 0);
            total_recv_bytes += recv_bytes;
            written_bytes = write(fd, buffer, recv_bytes);
        } while (total_recv_bytes < cont_length);
        close(fd);
        if (total_recv_bytes < cont_length) {
            status = 403;
            sendResponse(client_socket, status, buffer);
            writeLog(t->id, fd_log, operation, uri, status, req_id, log_queue);
            goto exit_thread;
        }

        // PUT RESPONSE
        status = ((status == 201) ? 201 : 200);
        sendResponse(client_socket, status, buffer);
        writeLog(t->id, fd_log, operation, uri, status, req_id, log_queue);
        goto exit_thread;
    } else {
      status = 501;
      sendResponse(client_socket, status, buffer);
      writeLog(t->id, fd_log, operation, uri, status, req_id, log_queue);
      goto exit_thread;
  }

  exit_thread:
  t->active = false; 
  if ((*(int *) queue_peek(log_queue)) == ((int) t->id)) {
    queue_pop(log_queue, &queue_obj);
  }
  close(client_socket);
  free(buffer);
  free(line);
  return NULL;
}

//testing commit #1
//testing commit #2
int main(int argc, char *argv[]) {
    if (argc < 2) {
        warnx("wrong arguments");
        return 1;
    }

    int fd_log = STDERR_FILENO;
    int thread_count = DEFAULT_THREAD_COUNT;
    int opt;
    while ((opt = getopt(argc, argv, "t:l:")) != -1) {
        switch (opt) {
        case 't': thread_count = atoi(optarg); break;
        case 'l': fd_log = open(optarg, O_RDWR | O_CREAT | O_TRUNC, 0777); break;
        }
    }
    int port = atoi(argv[argc - 1]);
    queue_t *log_queue = queue_new(thread_count);
    int server_socket;
    struct sockaddr_in client_addr;
    socklen_t client_addr_size;
    int client_socket = -1;
    int client_socket_arr[thread_count];
    server_socket = create_listen_socket(port);
    struct thread_data * tdata_arr = (struct thread_data *)malloc(sizeof(struct thread_data)*thread_count);
    for(int i=0; i<thread_count; i++) {
        tdata_arr[i].active = false;
        client_socket_arr[i] = -1;
    }
    printf("Listening on port %d...\n", port);
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_addr_size);
        for (int i = 0;; i++) {
            if (tdata_arr[i % thread_count].active == false) {
                client_socket_arr[i % thread_count] = client_socket;
                tdata_arr[i % thread_count].active = true;
                tdata_arr[i % thread_count].client_socket = client_socket_arr[i % thread_count];
                tdata_arr[i % thread_count].log_queue = log_queue;
                tdata_arr[i % thread_count].fd_log = fd_log;
                pthread_create(&tdata_arr[i % thread_count].id, NULL, workerThreadInit,
                    &tdata_arr[i % thread_count]);
                break;
            }
        }
    }

    free(tdata_arr);
    queue_delete(&log_queue);
    close(fd_log);
    return 0;
}
