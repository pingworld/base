/* win32.h
 *
 */

#ifndef WIN32_H
#define WIN32_H

#undef  _WIN32_WINNT
#define _WIN32_WINNT    0x0501        /* Needed to resolve getaddrinfo et al. */

#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <io.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <process.h>

// TODO
// �ж��Ƴ����Ч��
// #define EWOULDBLOCK        EAGAIN
// #define EAFNOSUPPORT       47
// #define EADDRINUSE         WSAEADDRINUSE
// #define ENOTCONN           WSAENOTCONN
// #define ECONNRESET         WSAECONNRESET
// #define EAI_SYSTEM         -11

#define PATH_MAX		MAX_PATH

#ifndef SIGHUP
#define SIGHUP -1
#endif

typedef char *caddr_t;

#define O_BLOCK 0
#define O_NONBLOCK 1
#define F_GETFL 3
#define F_SETFL 4

#define RUSAGE_SELF    0

#define IOV_MAX 1024

struct msghdr {
    void         *msg_name;         /* Socket name            */
    int          msg_namelen;       /* Length of name        */
    struct iovec *msg_iov;          /* Data blocks            */
    int          msg_iovlen;        /* Number of blocks        */
    void         *msg_accrights;    /* Per protocol magic (eg BSD file descriptor passing) */
    int          msg_accrightslen;  /* Length of rights list */
};

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

/* Structure which says how much of each resource has been used.  */
struct rusage {
    /* Total amount of user time used.  */
    struct timeval ru_utime;
    /* Total amount of system time used.  */
    struct timeval ru_stime;
    /* Maximum resident set size (in kilobytes).  */
    long int ru_maxrss;
    /* Amount of sharing of text segment memory
       with other processes (kilobyte-seconds).  */
    long int ru_ixrss;
    /* Amount of data segment memory used (kilobyte-seconds).  */
    long int ru_idrss;
    /* Amount of stack memory used (kilobyte-seconds).  */
    long int ru_isrss;
    /* Number of soft page faults (i.e. those serviced by reclaiming
       a page from the list of pages awaiting reallocation.  */
    long int ru_minflt;
    /* Number of hard page faults (i.e. those that required I/O).  */
    long int ru_majflt;
    /* Number of times a process was swapped out of physical memory.  */
    long int ru_nswap;
    /* Number of input operations via the file system.  Note: This
       and `ru_oublock' do not include operations with the cache.  */
    long int ru_inblock;
    /* Number of output operations via the file system.  */
    long int ru_oublock;
    /* Number of IPC messages sent.  */
    long int ru_msgsnd;
    /* Number of IPC messages received.  */
    long int ru_msgrcv;
    /* Number of signals delivered.  */
    long int ru_nsignals;
    /* Number of voluntary context switches, i.e. because the process
       gave up the process before it had to (usually to wait for some
       resource to be available).  */
    long int ru_nvcsw;
    /* Number of involuntary context switches, i.e. a higher priority process
       became runnable or the current process used up its time slice.  */
    long int ru_nivcsw;
};

int fcntl(SOCKET s, int cmd, int val);
int inet_aton(register const char *cp, struct in_addr *addr);

#define close(s) closesocket(s)

static inline void mapErr(int error) {
    switch(error) {
        default:
            break;
        case WSAEPFNOSUPPORT:
            errno = EAFNOSUPPORT;
            break;
        case WSA_IO_PENDING:
        case WSATRY_AGAIN:
            errno = EAGAIN;
            break;
        case WSAEWOULDBLOCK:
            errno = EWOULDBLOCK;
            break;
        case WSAEMSGSIZE:
            errno = E2BIG;
            break;
        case WSAECONNRESET:
            errno = 0;
            break;
    }
}

//#define write mem_write

static inline size_t mem_write(int s, void *buf, size_t len)
{
    DWORD dwBufferCount = 0;
    int error;

    WSABUF wsabuf = { len, (char *)buf} ;
    if(WSASend(s, &wsabuf, 1, &dwBufferCount, 0, NULL, NULL) == 0) {
        return dwBufferCount;
    }
        error = WSAGetLastError();
    if(error == WSAECONNRESET) return 0;
        mapErr(error);
    return -1;
}

//#define read mem_read

static inline size_t mem_read(int s, void *buf, size_t len)
{
    DWORD flags = 0;
    DWORD dwBufferCount;
    WSABUF wsabuf = { len, (char *)buf };
        int error;

    if(WSARecv((SOCKET)s,
        &wsabuf,
        1,
        &dwBufferCount,
        &flags,
        NULL,
        NULL
    ) == 0) {
        return dwBufferCount;
    }
    error = WSAGetLastError();
        if (error == WSAECONNRESET) return 0;
        mapErr(error);
    return -1;
}

static inline int sendmsg(int s, const struct msghdr *msg, int flags)
{
    DWORD dwBufferCount;
        int error;

    if(WSASendTo((SOCKET) s,
        (LPWSABUF)msg->msg_iov,
        (DWORD)msg->msg_iovlen,
        &dwBufferCount,
        flags,
        (const sockaddr *)msg->msg_name,
        msg->msg_namelen,
        NULL,
        NULL
    ) == 0) {
        return dwBufferCount;
    }
    error = WSAGetLastError();
        if (error == WSAECONNRESET) return 0;
        mapErr(error);
    return -1;
}

int createLocalListSock(struct sockaddr_in *saddr);
int createLocalSocketPair(int listSock, int *fds, struct sockaddr_in *saddr);
int getrusage(int who, struct rusage *usage);
int kill(int pid, int sig);
int sleep(int seconds);

struct sigaction {
    void (*sa_handler)(int);
    int sa_mask;
    int sa_flags;
};

#define sigemptyset(a) 0
extern int sigaction(int sig, struct sigaction *act, struct sigaction *oact);

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h> // portable: uint64_t   MSVC: __int64 

static inline int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
	// Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
	static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

	SYSTEMTIME  system_time;
	FILETIME    file_time;
	uint64_t    time;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	time = ((uint64_t)file_time.dwLowDateTime);
	time += ((uint64_t)file_time.dwHighDateTime) << 32;

	tp->tv_sec = (long)((time - EPOCH) / 10000000L);
	tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
	return 0;
}

#endif
