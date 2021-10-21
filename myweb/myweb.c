#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>

#define HTTP_HEADER_LEN 256
#define HTTP_REQUEST_LEN 256
#define HTTP_METHOD_LEN 6
#define HTTP_URI_LEN 100
#define HTTP_URL_PATH 100

#define REQ_END 100
#define ERR_NO_URI -100
#define ERR_ENDLESS_URI -101

struct http_req {
	char request[HTTP_REQUEST_LEN];
	char method[HTTP_METHOD_LEN];
	char uri[HTTP_URI_LEN];
	char url_path[HTTP_URL_PATH];
	char url_params[HTTP_URL_PATH];
	// version
	// user_agent
	// server
	// accept
};

int fill_req(char *buf, struct http_req *req) {
	if (strlen(buf) == 2) {
		// пустая строка (\r\n) означает конец запроса
		return REQ_END;
	}
	char *p, *a, *b;
	// Это строка GET-запроса
	p = strstr(buf, "GET");
	if (p == buf) {
		// Строка запроса должна быть вида
		// GET /dir/ HTTP/1.0
		// GET /dir HTTP/1.1
		// GET /test123?r=123 HTTP/1.1
		// и т.п.



        fprintf(stderr,  "buf %s\n", buf);
        char *path = p + strlen("GET ");
        char *find = strstr(path, "/ ");
        if (find == path)
        {
            //у нас пустой get запрос
            fprintf(stderr, "empty get  %s\n", find);
        }
        else
        {
            fprintf(stderr, "clear path %s\n", req->url_path);
            find = strstr(path, "?");
            if (NULL == find)
            {
                find = strstr(path, " HTTP");
                if (find == NULL)
                {
                    fprintf(stderr, "FAIL requet\n");
                    return ERR_NO_URI;
                }
                fprintf(stderr, "Don't have params\n");
                memcpy(req->url_path, path, find-path);
                req->url_path[find-path] = 0;
            }
            else
            {
                char *param = find + 1;
                memcpy(req->url_path, path, find-path);
                req->url_path[find-path] = 0;

                find = strstr(path, " HTTP");
                if (find == NULL)
                {
                    fprintf(stderr, "FAIL requet\n");
                    return ERR_NO_URI;
                }
                memcpy(req->url_params, param, find-param);
                req->url_params[find-param] = 0;
            }

            fprintf(stderr, "path %s\n", req->url_path);
            fprintf(stderr, "params %s\n", req->url_params);

        }


		a = strchr(buf, '/');
		if ( a != NULL) { // есть запрашиваемый URI 
			b = strchr(a, ' ');
			if ( b != NULL ) { // конец URI
                //strncpy_s(req->uri, sizeof (req->uri), a, b-a);
                strncpy(req->uri, a, b-a);
			} else {
				return ERR_ENDLESS_URI;  
				// тогда это что-то не то
			}
		} else {
			return ERR_NO_URI; 
			// тогда это что-то не то
		}
	}

	return 0;	
}

int log_req(char *log_path, char *log_mes) {
    int fd;
    if ((fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0600 )) < 0)
    {
        perror(log_path);
        return 1;
    }
    if (write(fd, log_mes, strlen(log_mes)) != (ssize_t)strlen(log_mes))
    {
        perror(log_mes);
        return 2;
    }
    write(fd, "\n", 1);
    fsync(fd);
    close(fd);
    return 0;
    // fprintf(stderr, "%s %s\n%s\n", req->request, req->method, req->uri);
	return 0;
}

int make_resp(struct http_req *req) {
    (void) req;
	printf("HTTP/1.1 200 OK\r\n");
	printf("Content-Type: text/html\r\n");
	printf("\r\n");
	printf("<html><body><title>Page title</title><h1>Page Header1</h1></body></html>\r\n");
	return 0;
}

int main (void) {
	char buf[HTTP_HEADER_LEN];
	struct http_req req;
	while(fgets(buf, sizeof(buf),stdin)) {
		int ret = fill_req(buf, &req);
		if (ret == 0) 
			// строка запроса обработана, переходим к следующей
			continue;
		if (ret == REQ_END ) 
			// конец HTTP запроса, вываливаемся на обработку
			break;
		else
			// какая-то ошибка 
			printf("Error: %d\n", ret);
		
	}

    log_req("some.log", "message");
	make_resp(&req);
}
