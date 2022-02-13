#ifndef  __LOG_H__
#define  __LOG_H__ 

#include "buffile.h"


class Log
{
public:
    static Log& Instance();
    Log();
    ~Log();
    int Open(const char* path, const char* filename);
    void Write(int aiLine, const char* asFuncName, const char* asFormat, ...);

    void WaitWriteFinish();
private:
    BufFile _bfile;
};


#define LOG_INIT(path, filename)\
       Log::Instance().Open(path, filename);

#define LOG(strFormat, ...) \
       Log::Instance().Write( __LINE__, __FUNCTION__, strFormat, ##__VA_ARGS__);


#define LOG_UINIT()\
	 Log::Instance().WaitWriteFinish();

#endif
