#include "httpconn.h"
#include "log.h"
#include "httpmng.h"

#include <stdio.h>




int main(int argc, char** argv)
{
    int j = 8250;

    int v =  (int(j >> 12) << 12) + 4096;
    printf(" %d\n", v);

    LOG_INIT("./", "/http-m.log");
	
    HttpMng::Instance().InitPool(1);

    HttpConn http;

//    http.SetHttpHost("192.168.37.165", 8575);
    http.SetHttpHost("api.crypto.com", 443);
    int err = http.OpenConn();

    STR_RECV_BUF stru_recv;

    const char* fmt = "{\"id\":%d,\"jsonrpc\": \"2.0\",\"method\": \"eth_chainId\", \"params\":[]}";
    char body[512];
    int body_len = 0; //snprintf(body, sizeof(body), fmt, 4);


    const char* url = "https://api.crypto.com/v2/public/get-book?instrument_name=BTC_USDT&depth=10";
//    const char* url = "https://api.crypto.com/v2/public/get-instruments";

    int i = 0;
    for (; i < 100; i++) {
	    
        err = http.Request((char*)url, body, body_len, stru_recv);
        printf(" %s\n -------%d\n", stru_recv.pbody, i);
    }

    printf("code %d , error %d, status %d\n", http.GetRespCode(), http.GetError(), http.GetStatus());

    err = http.Request((char*)url, body, body_len, stru_recv);
    printf(" %d-------\n\n %s %d\n", i, stru_recv.pbody, stru_recv.body_size);

    printf("\n-----%d-----\n", i);

    sleep(10000);


    return 0;	
}
