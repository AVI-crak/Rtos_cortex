/**
#include "printo.h"
 @file+   "printo.h"
 @author  AVI-crak
 @version V-90%
 @date    january 2022
 @brief   Cortex ARM, GCC, EmBitz
 license MIT (Massachusetts Institute of Technology)

 intended purpose - macro printo();
 printo("text", double, float, uint(8-16-32-64)_t, int(8-16-32-64)_t )
 printing without specifying the type of the variable,
 up to 9 variables and constants at a time,
 has no external dependencies,
 all functions are thread independent,
 minimum weight +140 bytes, complete set 1684 bytes,
 maximum speed 20~470 ticks,
 does not use division and floating point,
 optimized for ARM

 discussion forum
 http://forum.ixbt.com/topic.cgi?id=48:11735

 repository
 https://github.com/AVI-crak/Rtos_cortex
*/

#ifndef _printo_
#include <stdint.h>

#include "monitor.h" /// my version M_print(txt)

/// determine the number of characters for numbers (6-20)
#define OUT_TXT_SIZE_FLOATING  18

#define DEL_ZERO    0



///define a text string output function
static inline void soft_print(char* txt){M_print(txt);};



typedef enum
{
    raw_error,
    raw_u32,
    raw_u64,
    raw_double,
}Tip_TypeDef;


typedef struct
{
    union LLL_
    {
        double   dou;
        uint64_t u64;
        uint32_t u32[2];
    }L;
    char*        txt;
    Tip_TypeDef  tip;
}Raw_TypeDef;

union w64_all
{
    uint64_t        w64;
    double          double_raw;
    struct{
        uint64_t    massa  :52;
        uint64_t    order  :11;
        uint64_t    sign   :1;
    };
    struct{
        uint32_t    wlo;
        uint32_t    whi;
    };
    struct{
        float       flo1;
        float       flo2;
    };
};


Raw_TypeDef scano(char* txt);

char* i32_char(char* tail_txt, int32_t value);
char* u32_char(char* tail_txt, uint32_t value);
char* u64_char(char* tail_txt, uint64_t value);
char* i64_char(char* tail_txt, int64_t value);
char* float_char(char* text, float value);
char* double_char(char* text, double value);


void test_printo (void);

static inline void print__char(char value){
    char print_buf4[4];
    print_buf4[0] = value;
    print_buf4[1] = 0;
    soft_print(&print_buf4[0]);
};

static inline void soft_print_con(const char* txt){
    soft_print((char*)txt);
};

static inline void print_uint8(uint8_t value){
    char print_buf8[8];
    soft_print(u32_char(&print_buf8[7],value));
};

static inline void print_uint16(uint16_t value){
    char print_buf8[8];
    soft_print(u32_char(&print_buf8[7],value));
};

static inline void print_uint32(uint32_t value){
    char print_buf12[12];
    soft_print(u32_char(&print_buf12[11],value));
};

static inline void print_uint64(uint64_t value){
    char print_buf24[24];
    soft_print(u64_char(&print_buf24[23],value));
};

static inline void print_int8(int8_t value){
    char print_buf8[8];
    soft_print(i32_char(&print_buf8[7],value));
};

static inline void print_int16(int16_t value){
    char print_buf8[8];
    soft_print(i32_char(&print_buf8[7],value));
};

static inline void print_int32(int32_t value){
    char print_buf12[12];
    soft_print(i32_char(&print_buf12[11],value));
};

static inline void print_int64(int64_t value){
    char print_buf24[24];
    soft_print(i64_char(&print_buf24[23],value));
};

static inline void print_float(float value){
    char print_buf_f[(OUT_TXT_SIZE_FLOATING + 10)];
    soft_print(float_char(&print_buf_f[0],value));
};

static inline void print_double(double value){
    char print_buf_d[(OUT_TXT_SIZE_FLOATING + 10)];
    soft_print(double_char(&print_buf_d[0], value));
};



void  hex_char(uint64_t value, char* txt, int32_t isize);

static inline void print_hex8(uint8_t value){
    char print_buf_hex8[8];
    hex_char( value, print_buf_hex8, 5);
    soft_print(print_buf_hex8);
};

static inline void print_hex16(uint16_t value){
    char print_buf_hex16[8];
    hex_char( value, print_buf_hex16, 7);
    soft_print(print_buf_hex16);
};

static inline void print_hex32(uint32_t value){
    char print_buf_hex32[12];
    hex_char( value, print_buf_hex32, 10);
    soft_print(print_buf_hex32);
};

static inline void print_hex64(uint64_t value){
    char print_buf_hex64[20];
    hex_char( value, print_buf_hex64, 18);
    soft_print(print_buf_hex64);
};




#endif
#define _printo_


#ifndef __cplusplus

#define dpr_(X) _Generic((X),           \
    const char*:        soft_print_con, \
    char*:              soft_print,     \
    char:               print__char,    \
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
    default:            print_uint32       \
)(X)

#ifndef CONCAT

#define CONCAT(a,b) a##b
#define CONCAT2(a,b) CONCAT(a,b)

#define ELEVENTH_ARGUMENT(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, ...) a11
#define COUNT_ARGS(...) ELEVENTH_ARGUMENT(_, ##__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define dpr_0() soft_print(" ")
#define dpr_1(X) dpr_(X)
#define dpr_2(X,Y) do{dpr_(X);dpr_(Y);}while(0)
#define dpr_3(X,Y,Z) do{dpr_(X);dpr_(Y);dpr_(Z);}while(0)
#define dpr_4(X,Y,Z,W) do{dpr_(X);dpr_(Y);dpr_(Z);dpr_(W);}while(0)
#define dpr_5(X,Y,Z,W,E) do{dpr_(X);dpr_(Y);dpr_(Z);dpr_(W);dpr_(E);}while(0)
#define dpr_6(X,Y,Z,W,E,F) do{dpr_(X);dpr_(Y);dpr_(Z);dpr_(W);dpr_(E);dpr_(F);}while(0)
#define dpr_7(X,Y,Z,W,E,F,G) do{dpr_(X);dpr_(Y);dpr_(Z);dpr_(W);dpr_(E);dpr_(F);dpr_(G);}while(0)
#define dpr_8(X,Y,Z,W,E,F,G,H) do{dpr_(X);dpr_(Y);dpr_(Z);dpr_(W);dpr_(E);dpr_(F);dpr_(G);dpr_(H);}while(0)
#define dpr_9(X,Y,Z,W,E,F,G,H,I) do{dpr_(X);dpr_(Y);dpr_(Z);dpr_(W);dpr_(E);dpr_(F);dpr_(G);dpr_(H);dpr_(I);}while(0)


#define dpr(...) CONCAT2(dpr_, COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__)

#define printo(...) do{dpr(__VA_ARGS__);}while(0)

#endif

#else

static inline void dpr(char* obj)    { soft_print(obj); };
static inline void dpr(const char* obj) { soft_print_con(obj); };
static inline void dpr(char obj )    { print__char(obj); };
static inline void dpr(uint8_t obj ) { print_uint8(obj); };
static inline void dpr(uint16_t obj) { print_uint16(obj); };
static inline void dpr(uint32_t obj) { print_uint32(obj); };
static inline void dpr(uint64_t obj) { print_uint64(obj); };
static inline void dpr(int8_t obj)   { print_int8(obj); };
static inline void dpr(int16_t obj)  { print_int16(obj); };
static inline void dpr(int32_t obj)  { print_int32(obj); };
static inline void dpr(int64_t obj)  { print_int64(obj); };
static inline void dpr(float obj)    { print_float(obj); };
static inline void dpr(double obj)   { print_double(obj); };
static inline void dpr(uintptr_t obj){ print_hex(obj); };

template <typename T, typename ... Tail> void dpr(const T& obj, const Tail& ... tail) { dpr(obj); dpr(tail ...); }
template<typename ... T> void printo(const T& ... obj) { dpr(obj ...);  }
#endif

