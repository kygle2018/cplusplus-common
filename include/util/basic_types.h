#ifndef __UTIL_BASICTYPES_H_
#define __UTIL_BASICTYPES_H_

#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>


//namespace util {

typedef signed char     schar;
typedef signed char     int8;
typedef short           int16;
typedef int             int32;
typedef long long       int64;

typedef unsigned char   uint8;
typedef unsigned short  uint16;
typedef unsigned int    uint32;
typedef unsigned long long uint64;


#ifndef DWORD
#define DWORD       unsigned int
#endif

#ifndef WORD
#define WORD        unsigned short
#endif


#ifndef BYTE
#define BYTE        unsigned char
#endif

#ifndef UINT
#define UINT        unsigned int
#endif

#ifndef UINT64
#define UINT64       uint64_t
#endif

#define T_LONGLONG(x)   x##LL
#define T_ULONGLONG(x)  x##ULL


const uint8  kuint8max  = (( uint8) 0xFF);
const uint16 kuint16max = ((uint16) 0xFFFF);
const uint32 kuint32max = ((uint32) 0xFFFFFFFF);
const uint64 kuint64max = ((uint64) T_LONGLONG(0xFFFFFFFFFFFFFFFF));
const  int8  kint8min   = ((  int8) 0x80);
const  int8  kint8max   = ((  int8) 0x7F);
const  int16 kint16min  = (( int16) 0x8000);
const  int16 kint16max  = (( int16) 0x7FFF);
const  int32 kint32min  = (( int32) 0x80000000);
const  int32 kint32max  = (( int32) 0x7FFFFFFF);
const  int64 kint64min  = (( int64) T_LONGLONG(0x8000000000000000));
const  int64 kint64max  = (( int64) T_LONGLONG(0x7FFFFFFFFFFFFFFF));

template <bool>
struct CompileAssert {

};

#undef COMPILE_ASSERT
#define COMPILE_ASSERT(expr, msg) \
    typedef CompileAssert<(bool(expr))> msg[bool(expr) ? 1 : -1]

//} //endof namespace


#endif



