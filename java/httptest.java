package Httptest;

import HttpM.ReqTask;
import HttpM.ReqHttp;

public class httptest {
		static {
			System.load("/home/yy/http-m/bin/libhttp-keeper.so");
		}

		public static void main(String[] args) {
			ReqHttp reqHttp = new ReqHttp();
			int err = reqHttp.Init(2);
			
			System.out.println("init err=" + err);  
			
			ReqTask reqTask = new ReqTask();
			
			reqTask.host = "api.crypto.com";
			reqTask.port = 443;
			reqTask.url = "https://api.crypto.com/v2/public/get-book?instrument_name=BTC_USDT&depth=10";
			
		    reqTask.body =  "{\"id\":5,\"jsonrpc\": \"2.0\",\"method\": \"eth_chainId\", \"params\":[]}";
		    reqTask.body_len = 0; //reqTask.body.length();
		    
		    for (int i = 0; i < 10; i++) {
		    	reqTask.seq = i;
		        err = reqHttp.Requset(0, reqTask);
		        System.out.println("request err=" + err + " code=" + reqTask.resp_code);
		    
		        System.out.println("num=" + i + " resp= " + reqTask.resp);
		    }
		    
		    try {
				Thread.sleep(100000);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		    System.out.println(" uinit");
		    reqHttp.UnInit();
		    
		    
		}
}
