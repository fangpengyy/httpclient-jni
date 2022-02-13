#include "utils.h"
#include "log.h"

#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/file.h>


namespace TUTILS
{
    uint64_t GetNsTime()
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        return ts.tv_sec*1000000000 + ts.tv_nsec;
    }

    uint64_t GetUsTime() {
        struct timeval val;
        gettimeofday(&val, nullptr);
        return val.tv_sec * 1000000 + val.tv_usec;
    }

    uint64_t GetMsTime() {
        struct timeval val;
        gettimeofday(&val, nullptr);
        return val.tv_sec * 1000 + val.tv_usec / 1000;
    }

    void MsTimeToStr(uint64_t mstime, char buf[32])
    {
        int ms = mstime % 1000;
        time_t t = mstime / 1000;
        int i = strftime(buf, 32, "%Y-%m-%d %H:%M:%S", localtime(&t));
        sprintf(buf+i, ".%d", ms);
    }


    void MaskPipe()
    {
        sigset_t signal_mask;
        sigemptyset (&signal_mask);
        sigaddset (&signal_mask, SIGPIPE);
        int rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
        if (rc != 0)
            printf("block sigpipe error/n");
    }


    bool GetExecPath(std::string& path)
    {
        char szPath[256] = {0};
        if (readlink("/proc/self/exe", szPath, sizeof(szPath)) <= 0)
            return false;

        char* p1 = strrchr(szPath, '/');
        if (p1 == nullptr)
            return false;
        *(++p1) = '\0';
        path = szPath;
        return true;
    }

    void Createdir(const char* szPath)
    {
        if (access(szPath, 0) != 0) {
            umask(0);
	    if (mkdir(szPath, 0664) != 0) //S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0)
                printf("%s create path=%s error=%s\n", __func__, szPath, strerror(errno));
        }
    }

    int ParseFileName(const char* filepathname, char* filename, char* path) 
    {
        int len = strlen(filepathname);
	if (len == 0)
            return -1;

	int i = len;
        while (--i > -1) {
           if (filepathname[i] == '/') 
	       break;	   
	}
	
	if (i > -1) {
	    int n = len - i;
	    memcpy(filename, &filepathname[i], n);
	    filename[n] = '\0';

	    i++;
	    memcpy(path, filepathname, i);
	    path[i] = '\0';
	}
        else {
            memcpy(filename, filepathname, len);
            filename[len] = '\0'; 
	    strcpy(path, "./");
       	}	
    }

    int SetCoreMaxLimit()
    {
       struct rlimit rlim;
       memset(&rlim, 0, sizeof(rlim));

       rlim.rlim_cur = rlim.rlim_max = RLIM_INFINITY;
       if (setrlimit(RLIMIT_CORE, &rlim) < 0) {
           printf("setrlimit error=%s\n", strerror(errno));
           return -1;
       }

       memset(&rlim, 0, sizeof(rlim));
       if (getrlimit(RLIMIT_CORE, &rlim) < 0) {
           printf("getrlimit error=%s\n", strerror(errno));
           return -1;
       }
       return 0;
   }

   
void set_proctitle(char** argv, const char* new_name)
{
    int size = 0;
    int i;
    for (i = 0; environ[i]; i++) {
        size += strlen(environ[i]) + 1;
    }
    char* p = (char*)malloc(size);
    char* last_argv = argv[0];
    for (i = 0; argv[i]; i++) {
        if (last_argv == argv[i]) {
            last_argv = argv[i] + strlen(argv[i]) + 1;
        }
    }

    for (i = 0; environ[i]; i++) {
        if (last_argv == environ[i]) {
            size = strlen(environ[i]) + 1;
            last_argv = environ[i] + size;

            memcpy(p, environ[i], size);
            environ[i] = (char*)p;
            p += size;
        }
    }
    last_argv--;
    strncpy(argv[0], new_name, last_argv - argv[0]);
    p = argv[0] + strlen(argv[0]) + 1;
    if (last_argv - p > 0) {
        memset(p, 0, last_argv - p);
    }
}

uint64_t Hex2U64(char* hex)
{
    uint64_t res = 0;
    char c;
    while ((c = *hex++)) {
        char v = (c & 0xF) + (c >> 6) | ((c >> 3) & 0x8);
        res = (res << 4) | (uint64_t) v;
    }
    return res;
}

int IntToHexBuf(uint64_t value, char* out)
{
   register const char base[] = "0123456789ABCDEF";

    int j = 0;
    uint8_t ch;
    char hex[130] = {0};
    char* p = &hex[129];
    *p = '\0';

    do  {
       ch = value % 16;
       value /= 16;
       p--;
       *p = base[ch];
       j++;

    } while (value);

    if (j % 2 != 0) {
        *(--p) = base[0];
        j++;
    }

    memcpy(out, p, j);
    out[j] = '\0';

    return j;
}


uint32_t DJBHash(char* str, unsigned int len)  
{  
   unsigned int hash = 5381;  
   unsigned int i = 0;  
   for(i = 0; i < len; str++, i++)  
   {  
       hash = ((hash << 5) + hash) + (*str);  
   }  
   return hash;  
}  


int OpenFileLock(const char* filename)
{
    int fd = open(filename, O_WRONLY|O_CREAT, 0640);
    if (fd < 0) {
        printf("open %s failed=%s\n", filename, strerror(errno));
        return -1;
    }

    int err = flock(fd, LOCK_EX|LOCK_NB);
    if (err == -1) {
        printf("lock failed\n");
        return -1;
    }
    return fd;
}

void CloseFileLock(int fd)
{
    flock(fd, LOCK_UN);
}


};

