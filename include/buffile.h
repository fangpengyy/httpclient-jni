#ifndef __BUFFILE_H_
#define __BUFFILE_H_

#include <stdio.h>
#include <pthread.h>
#include <atomic>
#include <string.h>
#include <unistd.h>



struct Link
{
    void* next;
    void* prev;
};


class SLink
{
public:	
    Link head;
    Link* tail;
    int spin;
    int count;

    SLink() {
        head.next = nullptr;
        tail = &head;
	spin = 0;
	count = 0;
    }

    void Put(void * p);
    void* Pop();
};

#if 0
class DLink
{
public:
    Link* head;
    Link* tail;
    int spin;


    DLink();
    void Put(void * p);
    int Remove(void* p);
};
#endif

struct BufIndx
{
    Link link;	
    int index;
    uint32_t buf_size;
    uint32_t w_pos;
    uint32_t sectime;
    char* pbuf;

    void Clean() {
       w_pos = 0;

    }

    int Write(char* data, uint32_t size) {
       if (buf_size -  w_pos < size)
	   return -1;
       
       memcpy(pbuf + w_pos,  data, size);
       w_pos += size;
       //printf("write w_pos=%d\n", w_pos);
       return 0;   
    }

    int Save(FILE* file) {
       if (w_pos == 0) 
	  return 0;

       if (fwrite(pbuf, 1, w_pos, file) == w_pos) {
	   //printf(" Save succ size=%d\n", w_pos);
	   w_pos = 0;
	   return 0;
       }
       printf(" Save error\n");
       return -1;
    }
};


class BufFile
{

public:
    BufFile();
    ~BufFile();
    int Open(const char* path, const char* filename, int id, int buf_size, int buf_count);
    int Write(char* buf, uint32_t size);
   
    //int Write(BufIndx*& bufindex, char* buf, uint32_t size);
    void Close();

    void Flush(){_flush = true;}

    BufIndx* GetIdleBuf();
    void Wait();

private:
    static void* OnWriteFile(void* param);
    void* DoWrite();
private:
    uint32_t _buf_count;
    uint32_t _buf_size;
    char* _pbuf;
    BufIndx* _arrbuf_indx;
    SLink _idle_link;
    SLink _store_link;

    char _path[256];
    char _filename[32];
    int _id; 

    pthread_t _th;
    int _w_buf_index;

    bool  _stop;
    bool _enable; 
    bool _flush{false};
};



#endif
