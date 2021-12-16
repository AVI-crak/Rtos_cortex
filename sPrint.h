/// sPrint.h
/// печать без указания типа параметра, в разнобой
/// без зависимостей от внешних библиотек
/// printo("текст", double / float / uint(8-16-32-64)_t / int(8-16-32-64)_t )
/// размер - 1907 байт при агрессивной оптимизации
/// на вкус и цвет... добавить собственную функцию печати в
/// soft_print()

#ifndef _sPrinto_

#include <stdint.h>

#include "monitor.h" /// используется функция печати monitor_print(txt)
/// количество  9-20
#define OUT_TXT_SIZE_FLOATING  20


static void soft_print(char* txt){ monitor_print(txt);};



char* i32_char(char* tail_txt, int32_t value);
char* u32_char(char* tail_txt, uint32_t value);
char* u64_char(char* tail_txt, uint64_t value);
char* i64_char(char* tail_txt, int64_t value);
char* float_char(float value, char* text);
char* double_char(double value, char* text);
char* hex_char(char* tail_txt, uint32_t value);
void compress_char(char* tex_in, char* tex_out);


static inline void soft_print_con(const char* txt){soft_print((char*)txt);};
static inline void print_uint8(uint8_t value){char print_buf8[8]; soft_print(u32_char(&print_buf8[7],value));};
static inline void print_uint16(uint16_t value){char print_buf8[8]; soft_print(u32_char(&print_buf8[7],value));};
static inline void print_uint32(uint32_t value){char print_buf12[12]; char* dot3u; dot3u = u32_char(&print_buf12[11],value); compress_char(dot3u,&print_buf12[11]); soft_print(dot3u);};
static inline void print_uint64(uint64_t value){char print_buf24[24]; char* dot6u; dot6u = u64_char(&print_buf24[23],value); compress_char(dot6u,&print_buf24[23]); soft_print(dot6u);};
static inline void print_int8(int8_t value){char print_buf8[8]; soft_print(i32_char(&print_buf8[7],value));};
static inline void print_int16(int16_t value){char print_buf8[8]; soft_print(i32_char(&print_buf8[7],value));};
static inline void print_int32(int32_t value){char print_buf12[12]; char* doti3; doti3 = i32_char(&print_buf12[11],value); compress_char(doti3,&print_buf12[11]); soft_print(doti3);};
static inline void print_int64(int64_t value){char print_buf24[24]; char* dot6i; dot6i = i64_char(&print_buf24[23],value); compress_char(dot6i,&print_buf24[23]); soft_print(dot6i);};
static inline void print_float(float value){char print_buf_f[20]; soft_print(float_char(value, &print_buf_f[0]));};
static inline void print_double(double value){char print_buf_d[20]; soft_print(double_char(value, &print_buf_d[0]));};
static inline void print_hex(uint32_t value){char print_buf_hex[12]; soft_print(hex_char(&print_buf_hex[11],value));};

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
    default:            print_hex       \
)(X)

#ifndef CONCAT

#define CONCAT(a,b) a##b
#define CONCAT2(a,b) CONCAT(a,b)

#define ELEVENTH_ARGUMENT(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, ...) a11
#define COUNT_ARGS(...) ELEVENTH_ARGUMENT(_, ##__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define dpr_0() soft_print(" ")
#define dpr_1(X) dpr_(X)
#define dpr_2(X,Y) do{dpr_(X); dpr_(Y);}while(0)
#define dpr_3(X,Y,Z) do{dpr_(X); dpr_(Y); dpr_(Z);}while(0)
#define dpr_4(X,Y,Z,W) do{dpr_(X); dpr_(Y); dpr_(Z); dpr_(W);}while(0)
#define dpr_5(X,Y,Z,W,E) do{dpr_(X); dpr_(Y); dpr_(Z); dpr_(W);dpr_(E);}while(0)
#define dpr_6(X,Y,Z,W,E,F) do{dpr_(X); dpr_(Y); dpr_(Z); dpr_(W);dpr_(E);dpr_(F);}while(0)
#define dpr_7(X,Y,Z,W,E,F,G) do{dpr_(X); dpr_(Y); dpr_(Z); dpr_(W);dpr_(E);dpr_(F);dpr_(G);}while(0)

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
static inline void dpr(int8_t obj)   { print_int8(obj); };
static inline void dpr(int16_t obj)  { print_int16(obj); };
static inline void dpr(int32_t obj)  { print_int32(obj); };
static inline void dpr(int64_t obj)  { print_int64(obj); };
static inline void dpr(float obj)    { print_float(obj); };
static inline void dpr(double obj)   { print_double(obj); };
static inline void dpr(void obj)     { print_hex(obj); };

template <typename T, typename ... Tail> void dpr(const T& obj, const Tail& ... tail) { dpr(obj); dpr(tail ...); }
template<typename ... T> void printo(const T& ... obj) { dpr(obj ...);  }
#endif


