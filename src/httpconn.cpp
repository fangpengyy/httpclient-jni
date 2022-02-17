#include "httpconn.h"
#include "log.h"
#include "utils.h"
#include "socketfun.h"
#include <sched.h>

#include <sys/types.h>
#include <sys/socket.h>

#if 0

# define SSL_ERROR_NONE                  0
# define SSL_ERROR_SSL                   1
# define SSL_ERROR_WANT_READ             2
# define SSL_ERROR_WANT_WRITE            3
# define SSL_ERROR_WANT_X509_LOOKUP      4
# define SSL_ERROR_SYSCALL               5/* look at error stack/return
                                           * value/errno */
# define SSL_ERROR_ZERO_RETURN           6
# define SSL_ERROR_WANT_CONNECT          7
# define SSL_ERROR_WANT_ACCEPT           8
# define SSL_ERROR_WANT_ASYNC            9
# define SSL_ERROR_WANT_ASYNC_JOB       10
# define SSL_ERROR_WANT_CLIENT_HELLO_CB 11
# define SSL_ERROR_WANT_RETRY_VERIFY    12

#endif 



const char* req_head ="GET %s HTTP/1.1\r\n"\
                      "Host: %s\r\n"\
                      "User-Agent: client\r\n"\
                      "Accept: */*\r\n"\
		      "Cache-Control: no-cache\r\n"\
		      "Connection: keep-alive\r\n"\
                      "Content-Length: %d\r\n\r\n";

const char* req_head_body ="POST %s HTTP/1.1\r\n"\
                      "Host: %s\r\n"\
                      "User-Agent: client\r\n"\
                      "Accept: */*\r\n"\
		      "Cache-Control: no-cache\r\n"\
                      "Connection: keep-alive\r\n"\
                      "Content-Length: %d\r\n\r\n%s";

//自定义扩展header
const char* req_ext_head_body ="POST %s HTTP/1.1\r\n"\
                      "Host: %s\r\n"\
                      "User-Agent: client\r\n"\
                      "Accept: */*\r\n"\
                      "Cache-Control: no-cache\r\n"\
                      "Connection: keep-alive\r\n"\
		      "%s\r\n"\
                      "Content-Length: %d\r\n\r\n%s";


#define DEF_FLAG_2     "Encoding: chunked"
#define DEF_FLAG       "Content-Length:"
#define DEF_FLAG_LEN   15
		      
#define DEF_CONN_FLAG  "Connection:"
#define DEF_HEAD_END   "\r\n\r\n"


HttpConn::HttpConn()
{

}

HttpConn::~HttpConn()
{

}

int HttpConn::SetHttpHost(const char* host, int port)
{
    strcpy(_http_addr.host, host);
    if (Socket::Name2Addr(host, &_http_addr.svr_addr.sin_addr.s_addr) == -1)
    {
        LOG("Name2Addr error=%s", strerror(errno));
        return -1;
    }

    _http_addr.svr_addr.sin_family = AF_INET;
    _http_addr.svr_addr.sin_port = htons(port);
    return 0;
}

int HttpConn::Connect(STRU_HTTP_ADDR& addr, int sockFd)
{
    _connect_times++;

#if defined(DEf_USE_SSL)
   
    int ret = connect(sockFd, (struct sockaddr*)&addr.svr_addr, sizeof(addr.svr_addr));
    if (ret != 0) {
        LOG("connect error=%s", strerror(errno));
	return -1;
    }

    return 0;
#else		    
    int status = 0;
    int ret = connect(sockFd, (struct sockaddr*)&addr.svr_addr, sizeof(addr.svr_addr));
    if (ret != 0) {
       	if (errno != EINPROGRESS) {   
            LOG("connect error=%s", strerror(errno));  		
            return -1;
	}
       	status = 1;
    }
    
    if (status == 1) {
        struct pollfd arr_fd[1];
        arr_fd[0].fd = sockFd;
        arr_fd[0].events = POLLOUT;

        //10 ms
        int ret = poll(arr_fd, 1, 10000);
        if (ret < 0) {
            LOG("poll error=%s", strerror(errno));
            return -3;  
        }
        else if (ret == 0) {
            LOG("connect timeout error=%s", strerror(errno));
            return -4; 
        }
    
        if (!(arr_fd[0].revents & POLLOUT)) {
	    LOG("poll out error=%s", strerror(errno));	
            return -5;
        }
    }
    return 0;
#endif    
}

int HttpConn::OpenConn()
{
    _error_type = enum_ok;	
    _status = enum_status_connecting;

    _sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (unlikely(_sockFd == -1)) { 
        _error_type = enum_error_callsysfunc;
        _status = enum_status_unconnected;
       	LOG("socket error=%s", strerror(errno));
	return -1;
    }

#if !defined(DEF_USE_SSL)
    if (unlikely(Socket::SetNonBlock(_sockFd, true) == -1)) {
	CloseConn();
	_error_type = enum_error_callsysfunc;
	_status = enum_status_unconnected;
	LOG("fcntl error=%s", strerror(errno));
        return -1;
    }
#endif

#if 0    
    Socket::SetTOS(_sockFd, 0);
    Socket::SetMaxSeg(_sockFd, 1460);
#endif   
    Socket::SetTcpNoDelay(_sockFd, true);

    int ret = Connect(_http_addr, _sockFd);
    if (ret != 0) {
	CloseConn();    
        _error_type = enum_error_net_conn;
        _status = enum_status_unconnected;
        LOG("connect error=%d", ret);
        return -1;
    }

#if defined(DEF_USE_SSL)
    ret = InitSSL(_sockFd);
    if (ret != 0) {
       	CloseConn();
        _error_type = enum_error_net_conn;
        _status = enum_status_unconnected;
        LOG("initssl error=%d", ret);
	return -1;
    }
#endif

    _status = enum_status_connected;
    return 0;
}

int HttpConn::InitSSL(int sockFd)
{
    _ctx = SSL_CTX_new(TLS_client_method());
    if (_ctx == NULL)
        return -1;

    _ssl = SSL_new(_ctx);
    if (_ssl == NULL)
	return -2;    
 
    SSL_set_fd(_ssl, sockFd);
    int ret = SSL_connect(_ssl);
    if (ret < 1) {
        int error = SSL_get_error(_ssl, ret);
        LOG("ssl connect ret=%d error=%d", ret, error);
        return -2;
    }

    if (Socket::SetNonBlock(sockFd, true) == -1)
        return -3;

    return 0;
}

void HttpConn::UnInitSSL()
{
    SSL_shutdown(_ssl);
    SSL_free(_ssl);
    SSL_CTX_free(_ctx);
}

void HttpConn::CloseConn()
{
#if defined(DEF_USE_SSL)
    UnInitSSL();
#endif	

    if (_sockFd > 0) {	
        close(_sockFd);
	_sockFd = -1;
    }
}

int HttpConn::SendData(int sockFd, STRU_ReqHttp& req)
{ 
    char buf[8192];
    register int i = 0;
    register int n = 0;
    register int send_size = 0;

    if (req.body_len > 0 && req.ext_headers) {  //POST 
        n = snprintf(buf, sizeof(buf), req_ext_head_body, req.url, _http_addr.host,
		       	req.ext_headers, req.body_len, req.body);
    }
    else if (req.body_len > 0) { //POST
        n = snprintf(buf, sizeof(buf), req_head_body, req.url, _http_addr.host,
                        req.body_len, req.body);
    }
    else  { //GET 
        n = snprintf(buf, sizeof(buf), req_head, req.url, _http_addr.host, 0);
    }

    int error;
    while (i != n) {
#if defined(DEF_USE_SSL)
    	send_size = SSL_write(_ssl, buf + i, n - i);
#else
        send_size = send(sockFd, buf + i, n - i, MSG_NOSIGNAL);
#endif	    
	if (send_size > 0) {
            i += send_size;
        }
#if defined(DEF_USE_SSL)        
	else {
            error = SSL_get_error(_ssl, send_size);
	    if (error == SSL_ERROR_WANT_WRITE) { 
		sched_yield();    
                continue;
	    }
	    LOG("ssl send error=%d", error);
            return -1;
        }
#else
        else if (send_size == 0) {
            return -1;
        }
        else {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            return -2;
        }
#endif
    }
    return 0;
}

int HttpConn::Request(STRU_ReqHttp& req, STRU_RECV_BUF& stru_recv)
{
    _resp_code = -1;	
    _error_type = enum_ok;	
#if 0
    uint64_t t1;	
    uint64_t t2;
    uint64_t t3;

    bool log = req.flags & 2;
    t1 = TUTILS::GetUsTime();
#endif

    if (_status != enum_status_connected) {
        _error_type = enum_error_status;
	LOG("connect seq=%d error status=%d", req.seq, _status);
        return -1;
    }
  
    int error = SendData(_sockFd, req);
    if (error != 0) {
        CloseConn();	    
        _error_type = enum_error_net_send;
        _status = enum_status_unconnected;
	LOG("senddata seq=%d error=%d", req.seq, error);
        return -1;
    }

#if 0    
    t2 = TUTILS::GetUsTime();
    req.send_us = t2 - t1;
#endif

    error = RecvHead(_sockFd, stru_recv);
    if (error == 0) {
       	if (stru_recv.need_recv_size > 0) {
	    error = RecvData(_sockFd, stru_recv);
	}
#if 0
	t3 = TUTILS::GetUsTime();
        req.recv_us = t3 - t2;
	req.total_us = t3 - t1;

        if (log) {
	    LOG("seq=%d error=%d send=%dus, recv=%dus, req_total=%dus, need_recv=%d, body_size=%d)",
		       req.seq, error, t2 - t1, t3 - t2, t3 - t1,
		       stru_recv.need_recv_size, stru_recv.body_size);	
	}
#endif
	if (error == 0) {
            _resp_code = 200;		
            _error_type = enum_net_recv_ok;
	    return 0;
	}
    }
    
    CloseConn();
    _error_type = enum_error_net_recv;
    _status = enum_status_unconnected;

#if 0
    t3 = TUTILS::GetUsTime();
    req.recv_us = t3 - t2;
    req.total_us = t3 - t1;

    if (log) {
        t3 = TUTILS::GetUsTime();
        LOG("clsoe conn, seq=%d recvdata =%dus, error=%d", req.seq, t3 - t2, error);
    }
#endif
    return -1;  
}

int HttpConn::RecvHead(int sockFd, STRU_RECV_BUF& stru_recv)
{
    register int recv_len = 0;
    register int i = 0;
    register int j = 0;

    char* buf = stru_recv.buf;
    char* p = nullptr;
    char* p1 = nullptr;

    stru_recv.need_recv_size = 0;
    stru_recv.pbody = nullptr;
    stru_recv.body_size = 0;

    while (i < DEF_BUF_SIZE)
    {
#if defined(DEF_USE_SSL)
        recv_len = SSL_read(_ssl, buf + i, DEF_BUF_SIZE - i);
#else
        recv_len = recv(sockFd, buf + i, DEF_BUF_SIZE - i, 0);
#endif     
     	if (recv_len > 0) {
            i += recv_len;
            p = strstr(buf, DEF_HEAD_END);
            if (!p) {
                continue;
	    }

	   // ParseRespCode(buf, _resp_code);
	   // stru_recv.code = _resp_code;
	    
	    int recv_body_size = i - (p + 4 - (char*)buf);

            p1 = strstr(buf, DEF_FLAG);
            if (p1) {
                p1 += DEF_FLAG_LEN + 1;
                j = atoi(p1);
                
		stru_recv.body_size = j;
                
		if (j < DEF_BUF_SIZE) {
		    stru_recv.need_recv_size = j - recv_body_size;
		    stru_recv.pbody = p + 4;
                    stru_recv.precv_pos = buf + i;
                }
                else {
	            if (!(stru_recv.flags & 0x01)) {
			stru_recv.flags |= 0x1;
			stru_recv.buf_size = (int(j >> 12) << 12) + 4096;
	                stru_recv.pstart_recv = (char*)malloc(stru_recv.buf_size);
	            }
                    else {
                        if (stru_recv.buf_size < j) {
			    stru_recv.buf_size = (int(j >> 12) << 12) + 4096;	
			    LOG("realloc size=%d", stru_recv.buf_size);
                            stru_recv.pstart_recv = (char*)realloc(stru_recv.pstart_recv, stru_recv.buf_size);
	               	}
                    }

	            memcpy(stru_recv.pstart_recv, p + 4, recv_body_size);
	            stru_recv.pbody = stru_recv.pstart_recv;
	            stru_recv.precv_pos = stru_recv.pstart_recv + recv_body_size;	    
	            stru_recv.need_recv_size = j - recv_body_size;
		}
	       	break;
	    }
	}
#if defined(DEF_USE_SSL)
        else {
            int error = SSL_get_error(_ssl, recv_len);
            if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_CONNECT) {
		sched_yield();
                continue;
            }
	    LOG("ssl read error=%d", error);
            return -1;
	}
#else	
        else if (recv_len == -1) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
                sched_yield();   
		continue;
	    }
            return -1;
        }
        else {
            return -2;
        }
#endif	
    }
    return 0;
}

int HttpConn::RecvData(int sockFd, STRU_RECV_BUF& stru_recv) 
{
    register int recv_len = 0;
    register int i = 0;
   
    char* buf = stru_recv.precv_pos;
    register int len = stru_recv.need_recv_size;

    while (i < len)
    {
#if defined(DEF_USE_SSL)
        recv_len = SSL_read(_ssl, buf + i, len - i);
#else	    
        recv_len = recv(sockFd, buf + i, len - i, 0);
#endif     
     	if (recv_len > 0) {
            i += recv_len;		
	}
#if defined(DEF_USE_SSL)
        else {
            int error = SSL_get_error(_ssl, recv_len);
            if (error == SSL_ERROR_WANT_READ) {
                sched_yield();
                continue;
            }
            LOG("ssl read error=%d", error);
            return -1;
        }
#else
	else if (recv_len == -1) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            return -1;
        }
        else {
	    return -2;
        }
#endif	
    }
    
    return 0;
}

void HttpConn::ParseRespCode(char* p, int& code)
{
    while (p && *p != ' ') {
	p++;
    }

    while (*p == ' ') {
        p++;
    }    

    code = atoi(p);
}
