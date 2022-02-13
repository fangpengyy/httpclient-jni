package HttpM;

/*
 * 
 * only support Content-Length
 */

public class ReqHttp {
		
	public native int Init(int req_maxid);
	public native void UnInit();
	public native int Requset(int id, ReqTask req_task); 
}
