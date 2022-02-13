# httpclient-jni
##java http/https jni so库

```
/*仅支持 Content-Length 数据格式
 * java 接口
 * only support Content-Length
 */

public class ReqHttp {
		
	public native int Init(int req_maxid);
	public native void UnInit();
	public native int Requset(int id, ReqTask req_task); 
}
```
