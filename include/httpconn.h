#ifndef __HTTP_CONN_H__
#define __HTTP_CONN_H__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <atomic>

#include <sys/un.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <error.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>



struct STRU_HTTP_ADDR
{
    sockaddr_in svr_addr;
    char host[32];
};

enum ErrorType :int8_t  {
   enum_ok =0,
   enum_error_net_conn = -1,
   enum_error_net_send = -2,
   enum_error_net_recv = -3,
   enum_error_parse = -4,
   enum_error_result = -5,
   enum_error_func_check = -6,
   enum_error_callsysfunc = -7,
   enum_error_status = -8,

   enum_net_recv_ok=100,
};

enum HttpNetStatus:int8_t {
    enum_status_unconnected=0,
    enum_status_connecting,
    enum_status_disconnecting,
    enum_status_connected,
};

#define DEF_BUF_SIZE  (64*1024)

struct STRU_RECV_BUF
{
    int seq{-1};	
    char buf[DEF_BUF_SIZE];
    int buf_size{0};
    
    char* pstart_recv{nullptr};
    int flags{0};

    char* precv_pos{nullptr};
    int need_recv_size{0};
    
    char* pbody{nullptr};
    int body_size{0};

    int code{0};

    STRU_RECV_BUF() {
	buf_size = sizeof(buf);
    }
    
    ~STRU_RECV_BUF() {
        if (flags & 0x1 && pstart_recv) {
	    free(pstart_recv);
	    pstart_recv = nullptr;
	}
    }
   
};

struct STRU_ReqHttp
{
    int seq;	
    char* url;
    char* body;
    int body_len; 
    char* ext_headers;
    int flags{0}; 

    uint32_t send_us;
    uint32_t recv_us;
    uint32_t total_us;
};

class HttpConn
{
public:
    HttpConn();
    ~HttpConn();
    int SetHttpHost(const char* host, int port);
    
    int OpenConn();
    void CloseConn();
    int Request(STRU_ReqHttp& req, STRU_RECV_BUF& stru_recv);

    ErrorType GetError(){return _error_type;}
    HttpNetStatus GetStatus(){return _status;}
    int GetRespCode(){ return _resp_code;}   
    
    std::atomic<int> _connect_times{0};
private:
    HttpNetStatus _status;
    ErrorType _error_type{enum_ok};
    int _resp_code{200};
    int _sockFd{-1};
    STRU_HTTP_ADDR _http_addr;
   
    SSL_CTX* _ctx{NULL};
    SSL* _ssl{NULL};
private:  
    int Connect(STRU_HTTP_ADDR& addr, int sockFd);
    int RecvHead(int sockFd, STRU_RECV_BUF& stru_recv);

    int RecvData(int sockFd, STRU_RECV_BUF& stru_recv);
    int SendData(int sockFd, STRU_ReqHttp& req);
    void ParseRespCode(char* p, int& code);

    int InitSSL(int sockFd);
    void UnInitSSL();
    int RecvDataSSL(STRU_RECV_BUF& stru_recv);

};


#endif
