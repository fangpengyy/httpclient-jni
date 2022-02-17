package HttpM;

/*
 * 
 * enum_code_ok=100,
   enum_code_send_failed=101,
   enum_code_recv_failed=102,
   enum_code_conn_failed=103,
   enum_code_error_param=104,
   enum_code_error_limit=105,
   enum_code_error_over=106,
 * 
 */

public class ReqTask {
	//seq_id
	public int seq;
	
	//connect http server
	public String host;
	public int port;
	
	//0bit =1 use https, =0 use http;
	//1bit =1 record func time log;
	public int flags; 
	
	//fullname eg. https://xxxx/xx
	public String url;
	
	public String ext_headers;
	
	public int body_len;
	public String body;
	
	public int resp_code;
	public int resp_size;
	public String resp;
}



