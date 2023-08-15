#ifndef _ST_HPC_PPL_COMMON_WINDOWS_PTHREAD_H_
#define _ST_HPC_PPL_COMMON_WINDOWS_PTHREAD_H_

// XXX NOTE: partial pthread support for windows. search param name "unsupported".

#define NOGDI
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

typedef HANDLE pthread_t;

int pthread_create(pthread_t*, void* unsupported, void* (*f)(void*), void* arg);
int pthread_join(pthread_t, void* unsupported);
int pthread_detach(pthread_t);
int pthread_cancel(pthread_t);
pthread_t pthread_self();

/* -------------------------------------------------------------------------- */

typedef CRITICAL_SECTION pthread_mutex_t;

int pthread_mutex_init(pthread_mutex_t*, void* unsupported);
int pthread_mutex_lock(pthread_mutex_t*);
int pthread_mutex_unlock(pthread_mutex_t*);
int pthread_mutex_destroy(pthread_mutex_t*);

/* -------------------------------------------------------------------------- */

typedef CONDITION_VARIABLE pthread_cond_t;

int pthread_cond_init(pthread_cond_t*, void* unsupported);
int pthread_cond_signal(pthread_cond_t*);
int pthread_cond_broadcast(pthread_cond_t*);
int pthread_cond_wait(pthread_cond_t*, pthread_mutex_t* mutex);
int pthread_cond_destroy(pthread_cond_t*);

/* -------------------------------------------------------------------------- */

#define PTHREAD_ONCE_INIT 0

typedef LONG pthread_once_t;

int pthread_once(pthread_once_t*, void (*f)(void));

#ifdef __cplusplus
}
#endif

#endif
