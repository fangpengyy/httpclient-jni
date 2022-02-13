#include "buffile.h"
#include "utils.h"
#include <stdlib.h>


#define DEF_DEBUG   0


#define def_init(locker) {\
    locker = 0;\
}

#define def_lock(locker) {\
    while (__sync_lock_test_and_set(&(locker),1)) {}\
}

#define def_unlock(locker) {\
    __sync_lock_release(&(locker));\
}

//---- SLink-------

void SLink::Put(void * p)
{
    if (p == nullptr)
        return;

    Link* link = (Link*)p;	
    link->next = nullptr;

    def_lock(spin);
    tail->next = link;
    tail = link;

    ++count;
    def_unlock(spin);
}

void* SLink::Pop()
{
    Link* link = nullptr;
    def_lock(spin);
    link = (Link*)head.next;
    if (link) {
        head.next = link->next;
        if (head.next == nullptr) 
	    tail = &head;	
	--count;
    } 
    def_unlock(spin);
    return link;
}

#if 0
//---DLink------------

DLink::DLink()
{
    head = nullptr;
    tail = nullptr;
    spin = 0;
}

void DLink::Put(void * p)
{
    if (p == nullptr)
       return;

    Link* link = (Link*)p;
    link->next = nullptr;
    link->prev = nullptr;

    def_lock(spin);
    if (tail == nullptr) {
        head = link;
        tail = link;
    }
    else {	    
        tail->next = link;
        link->prev = tail;
        tail = link;
    }

    def_unlock(spin);
}

int DLink::Remove(void* p)
{
    Link* link = (Link*)p;	

    if (head == link) {
        head = (Link*)link->next;
        if (head == nullptr)
            tail = nullptr;
        else {
            head->prev = nullptr;
	}	 
    }
    else {
        if (tail == p) {
	    tail = (Link*)link->prev;	
	    if (tail == nullptr)
	        head = nullptr;
            else	    
	        tail->next = nullptr;
	}
        else {
            Link* prev = (Link*)link->prev;
            Link* next = (Link*)link->next;
            if (prev)
	        prev->next = link->next;
	    else
		return -1;    
	    if (next)
	        next->prev = link->prev;
	    else
		return -2;    
	}	
    }
    return 0;
}
#endif

//----BufFile-----

BufFile::BufFile()
{

}

BufFile::~BufFile()
{
   if (_pbuf) {
       free(_pbuf);
       _pbuf = nullptr;
   }

   if (_arrbuf_indx) {
       free(_arrbuf_indx);
       _arrbuf_indx = nullptr;
       _buf_count = 0;
   }
}

int BufFile::Open(const char* path, const char* filename, int id, int buf_size, int buf_count)
{
    memcpy(_path, path, sizeof(_path));
    memcpy(_filename, filename, sizeof(_filename));

    _buf_count = buf_count;
    _buf_size = buf_size;   
    _pbuf = (char*)malloc(buf_size * buf_count);
  
    _arrbuf_indx = (BufIndx*)malloc(sizeof(BufIndx) * _buf_count);
    
    for (int i = 0; i < _buf_count; ++i) {
        _arrbuf_indx[i].index = i;
	_arrbuf_indx[i].buf_size = buf_size;
	_arrbuf_indx[i].pbuf = &_pbuf[buf_size * i];
        _arrbuf_indx[i].w_pos = 0;

	_idle_link.Put((void*)(_arrbuf_indx + i));
    }

    _id = id;
    _w_buf_index = 0;
    _enable = true;
    _stop = false;
 
    int ret = pthread_create(&_th, nullptr, OnWriteFile, this);
    if (ret != 0) {
        return -1;
    }

    return 0;
}


BufIndx* BufFile::GetIdleBuf()
{
    int i = 0;	
    void* p = nullptr;	
    BufIndx* buf_indx = nullptr;

    do {	
        p = _idle_link.Pop();
        if (p == nullptr) {
	    if (++i % 100)
                usleep(10);
	}
	    
    } while (p == nullptr);
    
    if (p) {
        buf_indx = (BufIndx*)p;
        buf_indx->sectime = time(0);
    }

    return buf_indx;
}

int BufFile::Write(char* buf, uint32_t size)
{
    if (!_enable)
       return -1;

    BufIndx* bufindex = GetIdleBuf();
    if (bufindex) {
        int ret = bufindex->Write(buf, size);
        if (ret == 0)
	    _store_link.Put(bufindex);
    }

    return 0;
}

void BufFile::Close()
{
#if (DEF_DEBUG == 1)	
    printf("BufFile::Close\n");	
#endif 
    _flush = true;    
    _stop = true;
    pthread_join(_th, nullptr);
}


void* BufFile::OnWriteFile(void* param)
{
    BufFile* pBufFile = (BufFile*)param;
    return pBufFile->DoWrite();
}

void* BufFile::DoWrite()
{
     pthread_setname_np(pthread_self(), "write-log");
	
     int n = 0;	
     int w_count = -1;
     char szfile[512];
     FILE* file = nullptr;
     int tick = 0;
     uint32_t last_time = time(0);
     short exit_count = 0;
     bool need_flush = false;

     while(1) {	
        if (w_count == -1 || w_count > 1000000){
            sprintf(szfile, "%s%s-%d-%ld-%d", _path, _filename, _id, TUTILS::GetMsTime(), n);
            n++;
	    if (file) 
	        fclose(file);
            file = fopen(szfile, "wb");
            if (file == nullptr) {
                _enable = false;
		return nullptr;
	    }
            tick = 0;
	    w_count = 0;
        }

	BufIndx* bufindex = (BufIndx*)_store_link.Pop();
	if (bufindex) {
            need_flush = true;		
            bufindex->Save(file);
            w_count++;
            _idle_link.Put(bufindex);
	}

	if (++tick % 100 == 0) {
            if (need_flush) {
		need_flush = false;    
                fflush(file); 
	    }	       
            usleep(100);
	}

	if (_stop){
            if (++ exit_count > 10) {
                _enable = false;		
	        if (file)
                    fclose(file);
	        break;  
	    }
	}   
    }
    return nullptr;
}

void BufFile::Wait()
{
    	
    Close();
}


