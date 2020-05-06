/// sPrint.h
/// печать без указания типа параметра, в разнобой
/// без зависимостей от внешних библиотек
/// printo("текст", double / float / uint(8-32-64)_t / int(8-32-64)_t )
/// размер - 1907 байт при агрессивной оптимизации
/// на вкус и цвет... добавить собственную функцию печати в
/// soft_print()


#ifndef _sPrinto_

#include <stdint.h>

#include "monitor.h" /// используется функция печати monitor_print(txt)
/// количество  10-18
#define OUT_TXT_SIZE_FLOATING   14

static void soft_print(char* txt){ monitor_print(txt);};


char* ui32_char (uint32_t value);
char* i32_char (int32_t value);
char* i64_char (int64_t value);
char* ui64_char (uint64_t value);
char* float_char (float value);
char* double_char (double value);
char* nex_char (uint32_t value);



//static inline void print(...)__attribute__((alias("printo"))); //???
//__attribute__((transparent_union)) ???

static inline void soft_print_con(const char* txt){soft_print((char*)txt);};
static inline void print_uint8(uint8_t value){soft_print(ui32_char(value));};
static inline void print_uint16(uint16_t value){soft_print(ui32_char(value));};
static inline void print_uint32(uint32_t value){soft_print(ui32_char(value));};
static inline void print_uint64(uint64_t value){soft_print(ui64_char(value));};
static inline void print_int8(int8_t value){soft_print(i32_char(value));};
static inline void print_int16(int16_t value){soft_print(i32_char(value));};
static inline void print_int32(int32_t value){soft_print(i32_char(value));};
static inline void print_int64(int64_t value){soft_print(i64_char(value));};
static inline void print_float(float value){soft_print(float_char(value));};
static inline void print_double(double value){soft_print(double_char(value));};
static inline void print_nex(uint32_t value){soft_print(nex_char(value));};
#endif
#define _sPrinto_


#ifndef __cplusplus

#define dpr_(X) _Generic((X),           \
    const char*:        soft_print_con, \
    char*:              soft_print,     \
    uint8_t:            print_uint8,    \
    uint16_t:           print_uint16,   \
    uint32_t:           print_uint32,   \
    uint64_t:           print_uint64,   \
    int8_t:             print_int8,     \
    int16_t:            print_int16,    \
    int32_t:            print_int32,    \
    int64_t:            print_int64,    \
    float:              print_float,    \
    double:             print_double,   \
    default:            print_nex       \
)(X)

#ifndef CONCAT

#define CONCAT(a,b) a##b
#define CONCAT2(a,b) CONCAT(a,b)

#define ELEVENTH_ARGUMENT(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, ...) a11
#define COUNT_ARGS(...) ELEVENTH_ARGUMENT(_, ##__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define dpr_1(X) dpr_(X)
#define dpr_2(X,Y) do{dpr_(X); soft_print(" "); dpr_(Y);}while(0)
#define dpr_3(X,Y,Z) do{dpr_(X); soft_print(" "); dpr_(Y); soft_print(" "); dpr_(Z);}while(0)
#define dpr_4(X,Y,Z,W) do{dpr_(X); soft_print(" "); dpr_(Y); soft_print(" "); dpr_(Z); soft_print(" "); dpr_(W);}while(0)
#define dpr_5(X,Y,Z,W,E) do{dpr_(X); soft_print(" "); dpr_(Y); soft_print(" "); dpr_(Z); soft_print(" "); dpr_(W);soft_print(" ");dpr_(E);}while(0)
#define dpr_6(X,Y,Z,W,E,F) do{dpr_(X); soft_print(" "); dpr_(Y); soft_print(" "); dpr_(Z); soft_print(" "); dpr_(W);soft_print(" ");dpr_(E);soft_print(" ");dpr_(F);}while(0)
#define dpr_7(X,Y,Z,W,E,F,G) do{dpr_(X); soft_print(" "); dpr_(Y); soft_print(" "); dpr_(Z); soft_print(" "); dpr_(W);soft_print(" ");dpr_(E);soft_print(" ");dpr_(F);soft_print(" ");dpr_(G);}while(0)

#define dpr(...) CONCAT2(dpr_, COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__)

#define printo(...) do{dpr(__VA_ARGS__);}while(0)

#endif

#else

static inline void dpr(char* obj) { soft_print(obj); };
static inline void dpr(const char* obj) { soft_print_con(obj); };
static inline void dpr(uint8_t obj ) { print_uint8(obj; };
static inline void dpr(uint16_t obj) { print_uint16(obj; };
static inline void dpr(uint32_t obj) { print_uint32(obj; };
static inline void dpr(uint64_t obj) { print_uint64(obj; };
static inline void dpr(uint8_t obj) {print_int8(obj); };
static inline void dpr(int16_t obj) { print_int16(obj); };
static inline void dpr(int32_t obj) { print_int32(obj); };
static inline void dpr(int64_t obj) { print_int64(obj); };
static inline void dpr(float obj) { print_float(obj); };
static inline void dpr(double obj) { print_double(obj); };

template <typename T, typename ... Tail> void dpr(const T& obj, const Tail& ... tail) { dpr(obj); soft_print(' '); dpr(tail ...); }
template<typename ... T> void printo(const T& ... obj) { dpr(obj ...);  }
#endif


