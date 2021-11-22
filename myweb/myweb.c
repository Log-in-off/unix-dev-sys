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
#define HTTP_URL_PATH 300
#define FILE_NAME_LEN 500

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

char const *index_file = "/index.html";

int fill_req(char *buf, struct http_req *req) {
	if (strlen(buf) == 2) {
		// пустая строка (\r\n) означает конец запроса
		return REQ_END;
	}

    char *p = strstr(buf, "GET"); // Это строка GET-запроса
	if (p == buf) {
		// Строка запроса должна быть вида
		// GET /dir/ HTTP/1.0
		// GET /dir HTTP/1.1
		// GET /test123?r=123 HTTP/1.1


        if ( strchr(buf, '/') == NULL)
            return ERR_NO_URI;  // тогда это что-то не то

        fprintf(stderr,  "buf %s\n", buf);
        char *path = p + strlen("GET ");
        char *find = strstr(path, "/ ");
        if (find == path)
        {
            //у нас пустой get запрос
            fprintf(stderr, "empty get  %s\n", find);
            memcpy(req->url_path, index_file, strlen(index_file));
        }
        else
        {
            find = strstr(path, "?");
            //path++; // убрали '/'
            if (NULL == find)
            {
                find = strstr(path, " HTTP");
                if (find == NULL)
                {
                    fprintf(stderr, "FAIL requet\n");
                    return ERR_ENDLESS_URI;
                }
                fprintf(stderr, "Don't have params\n");
                memcpy(req->url_path, path, find-path);
                req->url_path[find-path] = 0;
                fprintf(stderr, "clear path %s\n", req->url_path);
            }
            else
            {
                char *param = find + 1;
                memcpy(req->url_path, path, find-path);
                req->url_path[find-path] = 0;

                fprintf(stderr, "path %s\n", req->url_path);
                find = strstr(path, " HTTP");
                if (find == NULL)
                {
                    fprintf(stderr, "FAIL requet\n");
                    return ERR_ENDLESS_URI;
                }
                memcpy(req->url_params, param, find-param);
                req->url_params[find-param] = 0;
            }

            //fprintf(stderr, "path %s\n", req->url_path);
            //fprintf(stderr, "params %s\n", req->url_params);
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

int make_resp(char *base_path, struct http_req *req)
{
    int fdin;
    struct stat statbuf;
    void *mmf_ptr;
    // определяем на основе запроса, что за файл открыть
    char res_file[FILE_NAME_LEN] = "";

    sprintf(res_file, "%s%s",base_path, req->url_path);
    fprintf(stderr, "path file %s\n", res_file);
    // открываем
    if ((fdin=open(res_file, O_RDONLY)) < 0)
    {
        perror(res_file);
        return 1;
    }
    // размер
    if (fstat(fdin, &statbuf) < 0)
    {
        perror(res_file);
        return 1;
    }
    // mmf
    if ((mmf_ptr = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fdin, 0)) == MAP_FAILED)
    {
        perror("myfile");
        return 1;
    }
    // Выводим HTTP-заголовки
    char *http_result = "HTTP/1.1 200 OK\r\n";
    write(1,http_result,strlen(http_result));
    char *http_contype = "Content-Type: text/html\r\n";
    write(1,http_contype,strlen(http_contype));

    char *header_end = "\r\n";
    write(1,header_end,strlen(header_end));
    // Выводим запрошенный ресурс
    if (write(1,mmf_ptr,statbuf.st_size) != statbuf.st_size)
    {
        perror("stdout");
        return 1;
    }
    // Подчищаем ресурсы
    close(fdin);
    munmap(mmf_ptr,statbuf.st_size);
    return 0;
}


/*
int make_resp(char *base_path, struct http_req *req)
{
    (void) req;
    printf("HTTP/1.1 200 OK\r\n");
    printf("Content-Type: text/html\r\n");
    printf("\r\n");
    printf("<html><body><title>Page title</title><h1>Page Header1</h1></body></html>\r\n");
    return 0;
}
*/


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
    make_resp("/usr/local/bin/webpages", &req);
}
