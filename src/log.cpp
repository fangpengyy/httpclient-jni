#include "log.h"
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>


static uint64_t GetUsTime()
{
    struct timeval val;
    gettimeofday(&val, nullptr);
    return val.tv_sec * 1000000 + val.tv_usec;
}


Log& Log::Instance()
{
    static Log _log;
    return _log;
}

Log::Log()
{

}

Log::~Log()
{

}

int Log::Open(const char* path, const char* filename)
{
    return  _bfile.Open(path, filename, 1, 1024, 10000);
}

void Log::Write(int line, const char* funcName, const char* format, ...)
{
    va_list alist;
    va_start(alist, format);

    char buf[1024] = {0};
    uint64_t timestamp = GetUsTime();

    int liSize = sprintf(buf, "ustime:%ld %s line:%d ", timestamp, funcName, line);
    liSize += vsnprintf(buf + liSize, sizeof(buf) - liSize,  format, alist);
    va_end(alist);

    buf[liSize] = '\n';
    liSize++;

    _bfile.Write(buf, liSize);
}

void Log::WaitWriteFinish()
{
    _bfile.Wait();
}



