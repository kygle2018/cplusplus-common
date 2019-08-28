#ifndef UTIL_SINGLETON_H_
#define UTIL_SINGLETON_H_
#include "util/noncopyable.h"
#include <pthread.h>
#include <stdlib.h>

namespace util {
/**
* simple sample:
*    class A {
*    public:
*        void Fun() {}
*    };
*
*    util::Singleton<A>::Instance().Fun();
*
*/

template<typename T>
class Singleton : private Noncopyable {
public:
    static T& Instance() {
        pthread_once(&ponce_, &Singleton::Init);
        return *instance_;
    }

    static T* PointInstance() {
        pthread_once(&ponce_, &Singleton::Init);
        return instance_;
    }

private:
    Singleton() {}
    ~Singleton() {}

    static void Init() {
        instance_ = new T();
        ::atexit(Destroy);
    }

    static void Destroy() {
        delete instance_;
    }

private:
    static pthread_once_t ponce_;
    static T* instance_;
};

template<typename T>
pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

template<typename T>
T* Singleton<T>::instance_ = NULL;

}

#endif // UTIL_SINGLETON_H_

