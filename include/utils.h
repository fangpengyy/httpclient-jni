#ifndef __T_UTILS_H__
#define __T_UTILS_H__

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <sys/syscall.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <mutex>
#include <condition_variable>
#include <vector>



#define DEF_GET_TID() syscall(SYS_gettid)


#if defined(__GNUC__)
        static inline bool (likely)(bool x) { return __builtin_expect((x), true); }
        static inline bool (unlikely)(bool x) { return __builtin_expect((x), false); }
#else
        static inline bool (likely)(bool x) { return x; }
        static inline bool (unlikely)(bool x) { return x; }
#endif


struct Spinlock
{
    int spin;

    Spinlock() {
       spin = 0;
    }
    ~ Spinlock() {
       spin = 0;
    }
};

#define spinlock_lock(locker) {\
    int n = 0;\
    while (__sync_lock_test_and_set(&(locker.spin),1)) {\
        if (++n % 100 == 0)\
           usleep(1);\
    }\
}

#define spinlock_unlock(locker) {\
    __sync_lock_release(&(locker.spin));\
}



#define DEF_CASE_VAL(val)\
        case val:\
          return #val;

#define DEF_MIN(a, b) ((a) > (b) ? (b) : (a))

#define DEF_MAX(a, b) ((a) > (b) ? (a) : (b))



namespace TUTILS
{

    extern uint64_t GetNsTime();
    extern uint64_t GetUsTime();
    extern uint64_t GetMsTime();
    extern void MsTimeToStr(uint64_t mstime, char buf[32]);

    extern bool GetExecPath(std::string& path);
    extern void MaskPipe();
    extern void Createdir(const char* szPath);
    extern int ParseFileName(const char* filepathname, char* filename, char* path); 

    extern int SetCoreMaxLimit();

    extern void set_proctitle(char** argv, const char* new_name);

    extern uint64_t Hex2U64(char* hex);
    extern int IntToHexBuf(uint64_t value, char* out);

    extern uint32_t DJBHash(char* str, unsigned int len);

    extern int OpenFileLock(const char* filename);  
    extern void CloseFileLock(int fd);

};


#endif
