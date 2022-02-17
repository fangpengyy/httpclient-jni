#ifndef __HTTP_MNG_H__
#define __HTTP_MNG_H__
#include <atomic>


class HttpConn;

struct STRU_RECV_BUF;
struct STRU_ReqHttp;

class HttpMng
{
public:
    HttpMng();
    ~HttpMng();

    static HttpMng& Instance();
    int InitPool(int maxid);
    void UnInitPool();

    int Open(int id, const char* host, int port);
    void Close(int id);

    int Request(int id, STRU_ReqHttp& req);
    STRU_RECV_BUF* GetResp(int id);
    int GetConnTimes(int id);

    int GetStatus(int id);
    HttpConn* GetHttpConn(int id);
    int MaxId(){return _max_id;};
    bool IsStop(){return _stop;};
private:
    int _max_id;
    static const int _conn_num = 8;
    HttpConn* _conns[_conn_num];
    STRU_RECV_BUF* _recv_buf[_conn_num];
    char* _pBuf{nullptr};
    std::atomic<bool> _stop{false};
};



#endif
