#include "httpmng.h"
#include "httpconn.h"
#include <stdlib.h>
#include <openssl/ssl.h>


HttpMng::HttpMng()
{

}

HttpMng::~HttpMng()
{
        
}

HttpMng& HttpMng::Instance()
{
    static HttpMng _httpMng;
    return _httpMng;
}

int HttpMng::InitPool(int maxid)
{
    if (maxid < 1 || _conn_num < maxid)
        _max_id = _conn_num;
    else
        _max_id = maxid;

    for (int i = 0; i < _max_id; i++) {
        _conns[i] = new HttpConn();
        if (_conns[i] == nullptr)
            return -1;	 
    }

    _pBuf = (char*)malloc(sizeof(STRU_RECV_BUF) * _max_id);
    if (_pBuf == nullptr)
	return -2;    

    for(int i = 0; i < _max_id; i++)
        _recv_buf[i] = (STRU_RECV_BUF*)(_pBuf + i * sizeof(STRU_RECV_BUF));

#if defined(DEF_USE_SSL)
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
#endif

    _stop = false;
    return 0;
}

void HttpMng::UnInitPool()
{
    _stop = true;

    for (int i = 0; i < _max_id; i++) {
        if (_conns[i]) {
	    _conns[i]->CloseConn();	
	    delete _conns[i];
	}
    }	    

    if (_pBuf) {
        free(_pBuf);
        _pBuf = nullptr;	
    }
}

int HttpMng::Open(int id, const char* host, int port)
{
    HttpConn* pConn = _conns[id];	
    if (pConn->SetHttpHost(host, port) != 0)
        return -1;
    return pConn->OpenConn();    
}

int HttpMng::Request(int id, STRU_ReqHttp& req)
{   
    return _conns[id]->Request(req, *_recv_buf[id]);
}

int HttpMng::GetConnTimes(int id)
{
    return _conns[id]->_connect_times;
}

STRU_RECV_BUF* HttpMng::GetResp(int id)
{
    return _recv_buf[id];
}

int HttpMng::GetStatus(int id)
{
    return (int)_conns[id]->GetStatus();
}

HttpConn* HttpMng::GetHttpConn(int id)
{
    return _conns[id];
}

