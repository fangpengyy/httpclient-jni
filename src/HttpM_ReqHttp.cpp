#include "HttpM_ReqHttp.h"
#include "httpmng.h"
#include "utils.h"
#include "log.h"
#include "httpconn.h"


#define DEF_ReadStr(id, field, cls, obj, str)\
    id = env->GetFieldID(cls, #field, "Ljava/lang/String;");\
    if (id == 0){\
        return (jint)-100;\
    }\
    jstring field##1 = (jstring)env->GetObjectField(obj, id);\
    str = env->GetStringUTFChars(field##1, nullptr);


#define DEF_ReleaseStr(field, str)\
    env->ReleaseStringUTFChars(field##1, str);


#define DEF_GetInt(id, field, cls, obj, result) \
   id = env->GetFieldID(cls, #field, "I");\
   result = env->GetLongField(obj, id);


#define DEF_SetInt(id, field, cls, obj, result)\
      id = env->GetFieldID(cls, #field, "I");\
      env->SetIntField(obj, id, result);


enum InterCodeType: uint8_t {
   enum_code_ok=100,
   enum_code_send_failed=101,
   enum_code_recv_failed=102,
   enum_code_conn_failed=103,
   enum_code_error_param=104,
   enum_code_error_limit=105,
   enum_code_error_over=106,
};


JNIEXPORT jint JNICALL Java_HttpM_ReqHttp_Init
  (JNIEnv *env, jobject obj, jint req_maxid)
{
    const char* dir = "/var/http-m/";	
    TUTILS::Createdir(dir);
    LOG_INIT(dir, "/http-m.log");	
    return HttpMng::Instance().InitPool(req_maxid);
}

JNIEXPORT void JNICALL Java_HttpM_ReqHttp_UnInit
  (JNIEnv *env, jobject obj)
{

    HttpMng::Instance().UnInitPool();
    LOG_UINIT();
}

JNIEXPORT jint JNICALL Java_HttpM_ReqHttp_Requset
  (JNIEnv *env, jobject obj, jint id, jobject task_req)
{
    jfieldID fid;
    int seqv;
    int flags = 0;
    int err = -33;
    int body_len = 0;
    const char* pbody = nullptr;
    const char* urlv = nullptr;
    const char* pheaders = nullptr;

    uint64_t t1 = TUTILS::GetUsTime();

    jclass cls = env->FindClass("LHttpM/ReqTask;");
    if (cls == nullptr) {
        return -30;
    }

    HttpMng& httpMng = HttpMng::Instance();
    if (httpMng.IsStop()) {
	DEF_SetInt(fid, resp_code, cls, task_req, enum_code_error_limit);    
        return -31;
    }

    if (id < 0 || id >= httpMng.MaxId()) {
	DEF_SetInt(fid, resp_code, cls, task_req, enum_code_error_over);     
        return -32;
    }

    if (httpMng.GetStatus(id) != enum_status_connected) {
        const char* phost = nullptr;
        int portv = 0;

        DEF_ReadStr(fid, host, cls, task_req, phost);
        DEF_GetInt(fid, port, cls, task_req, portv);
        
	if (phost == nullptr || portv <= 0) {
	    if (phost)
	        DEF_ReleaseStr(host, phost);	
	    DEF_SetInt(fid, resp_code, cls, task_req, enum_code_error_param);
            return -33;
        }

        err = httpMng.Open(id, phost, portv);

	int n = httpMng.GetConnTimes(id);
	LOG("id=%d try_conn_times=%d connect %s:%d error=%d", id, n, phost, portv, err);

      	if (phost)
            DEF_ReleaseStr(host, phost);	
        if (err != 0) {
	    DEF_SetInt(fid, resp_code, cls, task_req, enum_code_conn_failed);	
	    return -34; 
        }	   
    }

    DEF_GetInt(fid, seq, cls, task_req, seqv);
    DEF_GetInt(fid, body_len, cls, task_req, body_len);
    
    DEF_ReadStr(fid, url, cls, task_req, urlv);
    if (urlv == nullptr) {
        DEF_SetInt(fid, resp_code, cls, task_req, enum_code_error_param);
	return -35;
    }

    DEF_ReadStr(fid, ext_headers, cls, task_req, pheaders);
    DEF_GetInt(fid, flags, cls, task_req, flags);

    STRU_ReqHttp req = {
    seq : seqv,
    url: (char*)urlv,
    body:NULL,
    body_len:body_len,
    ext_headers : (char*)pheaders,
    flags : flags
    };

    if (body_len > 0) {
       	DEF_ReadStr(fid, body, cls, task_req, pbody);
	req.body = (char*)pbody;

	err = httpMng.Request(id, req);
        if (pbody)
            DEF_ReleaseStr(body, pbody);
    }
    else {
        err = httpMng.Request(id, req);
    }
    
    if (pheaders)
        DEF_ReleaseStr(ext_headers, pheaders);

    register uint64_t t2 = 0;

    if (err == 0) {
	t2 = TUTILS::GetUsTime();
    
        STRU_RECV_BUF* recv_buf = httpMng.GetResp(id);
        recv_buf->pbody[recv_buf->body_size] = '\0';

        jfieldID sId = env->GetFieldID(cls, "resp", "Ljava/lang/String;");
	const char* p = (const char*)recv_buf->pbody;
       
	jstring jstr;
       	if (p) 
	    jstr = env->NewStringUTF(p);
	else
	    jstr = env->NewStringUTF("empty");	

        env->SetObjectField(task_req, sId, jstr);  
        DEF_SetInt(fid, resp_code, cls, task_req, enum_code_ok); 
	DEF_SetInt(fid, resp_size, cls, task_req, recv_buf->body_size);
#if 0
	LOG("code=%d body_size=%d %s", recv_buf->code, recv_buf->body_size, p);
#endif    
    }
    else {
        HttpConn* pConn = httpMng.GetHttpConn(id);
	if (pConn->GetError() == enum_error_net_recv) { 
            DEF_SetInt(fid, resp_code, cls, task_req, enum_code_recv_failed);
	}
	else {
	    DEF_SetInt(fid, resp_code, cls, task_req, enum_code_send_failed);
	}
    }

    if (flags & 2) {
        uint64_t t3 = TUTILS::GetUsTime();
        LOG("seq=%d use_total=%dus, new_reqtask %dus, error=%d", seqv, t3-t1, t3-t2, err);
#if 0
       	LOG("seq=%d use_total=%dus, new_reqtask %dus, error=%d (send=%dus recv=%dus total=%dus)", 
	      seqv, t3-t1, t3-t2, err, req.send_us, req.recv_us, req.total_us);
#endif
    }
    return err;	
}

