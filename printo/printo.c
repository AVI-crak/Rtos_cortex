/**
 @file+   "printo.c"
 @author  AVI-crak
 @version V-91%
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

#include "printo.h"

#if defined __ARM_ARCH
#if (__ARM_FEATURE_DSP > 0)
#define ARM_DSP_ONS__
#endif
#endif

#if defined ARM_DSP_ONS__
inline uint64_t f_umaal(uint32_t add1,uint32_t add2, uint32_t mult1,uint32_t mult2){
   uint64_t accum;
   asm volatile ("umaal %0, %1, %2, %3"
                : "+r" (add1), "+r" (add2)
                : "r" (mult1), "r" (mult2));
   accum = add2;
   accum <<= 32;
   accum |= add1;
    return accum;
}
#else
uint64_t f_umaal(uint32_t add1,uint32_t add2, uint32_t mult1,uint32_t mult2){
    uint64_t accum;
    accum = (uint64_t)mult1 * mult2 + add1 + add2;
    return accum;
}
#endif

/// 0-11 , 11-18, 18-25
uint64_t pow_tab[26] = {0xA686E3E8B11B0857,0xFD00B897478238D0,0xC0314325637A1939,
    0x91FF83775423CC06,0xDDD0467C64BCE4A0,0xA87FEA27A539E9A5,0x8000000000000000,
    0xC2781f49FFCFA6D5,0x93BA47C980E98CDF,0xE070F78D3927556A,0xAA7EEBFB9DF9DE8D,
    0x81842F29F2CCE375,0xBEBC200000000000,0x8E1BC9BF04000000,0xD3C21BCECCEDA100,
    0x9DC5ADA82B70B59D,0xEB194F8E1AE525FD,0xAF298D050E4395D6,0x82818F1281ED449F,
    0xA000000000000000,0xC800000000000000,0xFA00000000000000,0x9C40000000000000,
    0xC350000000000000,0xF424000000000000,0x9896800000000000,};



int16_t ord2_tab[26] = {-1276,-1064,-851,-638,-426,-213,0,212,425,637,850,1063,26,
    53,79,106,132,159,186,3,6,9,13,16,19,23,};

/// 0-11 , 11-18, 18-25
uint32_t pow_tab32[26] = {0xA686E3E8,0xFD00B897,0xC0314325,0x91FF8377,0xDDD0467C,
    0xA87FEA27,0x80000000,0xC2781f49,0x93BA47C9,0xE070F78D,0xAA7EEBFB,0x81842F29,
    0xBEBC2000,0x8E1BC9BF,0xD3C21BCE,0x9DC5ADA8,0xEB194F8E,0xAF298D05,0x82818F12,
    0xA0000000,0xC8000000,0xFA000000,0x9C400000,0xC3500000,0xF4240000,0x98968000,};

const char reserve_char[10] = { '0',0,'i','n','f',0,'N','a','N',0,};
char* floating_char (uint64_t val, char* txt, int32_t order2);
uint64_t mult64_do (uint64_t vall, int32_t* order2, int32_t degre);

uint64_t __attribute__ ((noinline, optimize("-Og"))) shift_left(uint64_t vall, int32_t nc){//28
    vall <<= nc;
    return vall;
};

uint64_t __attribute__ ((noinline, optimize("-Og"))) shift_right(uint64_t vall, int32_t nc){//28
    vall >>= nc;
    return vall;
};

uint64_t mult64_do (uint64_t vall, int32_t* order2, int32_t degre){//136
    int32_t rem, enc;
    uint32_t tmp;
    union w64_all val;
    union w64_all mul;
    union w64_all a2a1;
    union w64_all b2b1;
    val.w64 = vall;

    degre += 384;
    asm volatile("":::"memory");
    rem = degre & 7;
    if (rem != 0) rem += 18;
    enc = (degre & 0x38) >> 3;
    if (enc != 0) rem = (11 + enc) | (rem << 8);
    enc = (degre & 0x03C0) >> 6;
    rem = enc | (rem << 8);

    enc = *order2;
    do{
        tmp = rem & 31;
        enc += ord2_tab[tmp];
        mul.w64 = (uint64_t)pow_tab[tmp];
        tmp = (uint64_t) val.wlo * mul.wlo >> 32;
        a2a1.w64 = (uint64_t) val.wlo * mul.whi;
        b2b1.w64 = f_umaal(tmp, a2a1.wlo, val.whi, mul.wlo);
        val.w64 = f_umaal(a2a1.whi, b2b1.whi, val.whi, mul.whi);
        enc += val.whi >> 31;
        if ((val.whi >> 31) == 0 ){
            val.w64 += val.w64;
            val.wlo |= b2b1.wlo >> 31;
        };
        rem >>= 8;
    }while (rem);
    *order2 = enc;
    return val.w64;
}


void __attribute__ ((optimize("-O3"))) hex_char(uint64_t value, char* txt, int32_t isize){//52
    uint32_t tmp;
    txt[isize--] = 0;
    txt[1] = 'x';
    txt[0] = '0';
    do{
        tmp = value & 0x0Fu;
        tmp -= 0x0Au;
        tmp -= tmp >> 29;
        tmp += 0x30u + 0x0Au + 0x07u;
        txt[isize--] = tmp;
        value >>= 4;
    }while (isize != 1);
};

char* __attribute__ ((optimize("-Os"))) i32_char(char* tail_txt, int32_t value){//22
    int32_t mi = value >> 31;
    if (mi) value = 0 - value;
    tail_txt = u32_char(tail_txt, value);
    *(tail_txt - 1) = '-';
    tail_txt += mi;
    return tail_txt;
};

char* __attribute__ ((optimize("-Og"))) u32_char (char* tail_txt, uint32_t value){//40
    *tail_txt = 0;
    uint32_t tmp;
    do{
        tmp = (uint64_t) value * 0xCCCCCCCDUL >> 32;
        value += '0';
        tmp &= 0xFFFFFFF8;
        value -= tmp;
        value -= tmp >> 2;
        *(--tail_txt) = value;
        value = tmp >> 3;
    }while (value);
    return tail_txt;
};



char* __attribute__ ((optimize("-Os"))) i64_char(char* tail_txt, int64_t value){//26
    int32_t mi = value >> 63;
    if (mi) value = 0 - value;
    tail_txt = u64_char(tail_txt, value);
    *(tail_txt - 1) = '-';
    tail_txt += mi;
    return tail_txt;
};

char* __attribute__ ((optimize("-Os")))u64_char (char* tail_txt, uint64_t value){//68
    uint32_t res, mag;
    union w64_all val;
    union w64_all a2a1;
    union w64_all b2b1;
    union w64_all c3c2;
    val.w64 = value;
    *tail_txt = 0;
    do{
        mag = 0xCCCCCCCC;
        res = ((uint64_t)mag * val.wlo + val.wlo ) >> 32;
        a2a1.w64 = (uint64_t) mag * val.whi + val.whi;
        b2b1.w64 = f_umaal(res, a2a1.wlo, mag, val.wlo);
        c3c2.w64 = f_umaal(a2a1.whi, b2b1.whi, mag, val.whi);
        c3c2.w64 >>= 3;
        val.wlo -= c3c2.wlo << 1;
        val.wlo -= c3c2.wlo << 3;
        res = val.wlo + '0';
        val.w64 = c3c2.w64;
        *(--tail_txt) = res;
     }while (val.w64);
    return tail_txt;
};


char* __attribute__ ((optimize("-Os"))) float_char(char* txt, float value ){//100
    union w64_all massa;
    massa.flo2 = value;
    uint32_t tmp, sig;
    int32_t order2;
    char* txt_out;
    sig = (int32_t) massa.whi >> 31;
    tmp = 45 & sig;
    *txt = tmp;
    massa.whi <<= 1;
    asm volatile("":::"memory");
    if (massa.whi != 0){
        if (massa.whi < 0xFF000000UL){
            order2 = massa.whi >> 24;
            massa.whi <<= 7;
            if (order2 == 0){
                tmp = __builtin_clz(massa.whi);
                massa.whi <<= tmp;
                order2 = 1 - tmp;
            };
            order2 -= 127;
            massa.wlo = 0;
            txt_out = floating_char(massa.w64, txt, order2);
            return txt_out;
        }else if (massa.whi > 0xFF000000UL) massa.wlo = 6; // l_NaN;
        else massa.wlo = 2; // l_inf;
    }; // l_zero;
    txt_out = txt - sig;
    do{ tmp = reserve_char[massa.wlo++]; *txt_out++ = tmp;}while(tmp);
    return txt;
};//42

char* __attribute__ ((optimize("-Os"))) double_char(char* txt, double value ){//120
    union w64_all massa;
    massa.double_raw = value;
    uint32_t tmp, sig;
    int32_t order2;
    char* txt_out;

    sig = (int32_t) massa.whi >> 31;
    tmp = 45 & sig;
    *txt = tmp;
    massa.w64 = shift_left(massa.w64, 1);
    if (massa.w64 != 0){ // no goto l_zero;
        if (massa.w64 < 0xFFE0000000000000ULL){
            order2 = (uint32_t) massa.whi >> 21;
            if(order2 != 0) tmp = 10;
            else{
                    tmp = __builtin_clzll(massa.w64);
                    order2 = 11 - tmp;
                };
            order2 -= 1023;
            massa.w64 = shift_left(massa.w64, tmp);
            txt_out = floating_char(massa.w64, txt, order2);
            return txt_out;
        }else if (massa.w64 > 0xFFE0000000000000ULL) massa.wlo = 6;// l_NaN;
        else massa.wlo = 2;
    };
    txt_out = txt - sig;
    do{ tmp = reserve_char[massa.wlo++]; *txt_out++ = tmp;}while(tmp);
    return txt;
};

char* __attribute__((optimize("-Os"))) floating_char (uint64_t val, char* txt, int32_t order2){//188
    union w64_all massa;
    massa.w64 = val;
    uint32_t tmp;
    int32_t order10, ord_;

    massa.whi |= (uint32_t) 1 << 31;
    order10 = (int64_t)((int64_t) order2 * 1292917990 - 422) >> 32;
    ord_ = order2;
    tmp = 0 - order10;
    tmp = (uint64_t) mult64_do(massa.w64, &ord_, tmp) >> 32;
    ord_ = 31 - ord_;
    tmp >>= ord_;
    if (tmp > 9) order10++;
    tmp = OUT_TXT_SIZE_FLOATING - order10;
    massa.w64 = mult64_do(massa.w64, &order2, tmp);
    order2 = 62 - order2;
    massa.w64 = shift_right(massa.w64, order2);
    tmp = massa.wlo & 1;
    massa.w64 = shift_right(massa.w64, 1);
    massa.w64 += tmp;

    char* txt_out; char* txt_s; char* txt_ord;
    txt_s = &txt[OUT_TXT_SIZE_FLOATING + 4];
    txt_out = u64_char (txt_s, massa.w64);
    tmp = txt_s - txt_out - 1 - OUT_TXT_SIZE_FLOATING;
    txt_s -= tmp;
    order10 += tmp;
    txt_out--;
    txt_out[0] = txt_out[1];
    txt_out[1] = '.';
    if (*txt == '-') *(--txt_out) = '-';

#if (DEL_ZERO)
    do{tmp = *(--txt_s);} while (tmp == '0');
    if (tmp == '.') txt_s += 1;
    *(++txt_s) = 0;
#endif
    if (order10 != 0){
        txt_ord = txt_s + 6;
        txt_ord = i32_char(txt_ord, order10);
        *(--txt_ord) = 'e';
        do{
            tmp = *txt_ord++;
            *txt_s++ = tmp;
        }while (tmp != 0);
    };
    return txt_out;
};

Raw_TypeDef scano(char* txt){///422
    Raw_TypeDef raw;
    raw.tip = raw_error;
    uint32_t fmb, tmp;
    int32_t ord, ord_in, sig_o;
    union w64_all val;
    union w64_all mul;
    union w64_all a2a1;
    val.w64 = 0;

    if (*txt == '0') txt++;
    if ((*txt == 'x')||(*txt == 'X')){
        while(1){
            fmb = *(++txt);
            tmp = fmb >> 7;       // !(ASCII)
            fmb -= '0';
            if ( fmb > 9u) {
                fmb += ('0' -1); // 'a','A'-> 0b01x00000
                fmb &= '_';      // 'a'->'A'
                fmb -= ('0' + 6);// 'A'->10
            };
            tmp += fmb >> 4;
            if (tmp) goto exit0;      // !(0~F)
            if (val.whi >> 28) goto error;
            val.w64 = shift_left(val.w64, 4);
            val.wlo |= fmb;
        };// test godbolt.org/z/4nME5zadc

    }else if((*txt == 'b')||(*txt == 'B')){
        while(1){
            fmb = *(++txt);
            fmb -= '0';
            if (fmb > 1u) goto exit0;
            if (val.whi >> 31) goto error;
            val.w64 += val.w64;
            val.w64 |= fmb;
        };

    }else{
        ord = 0; sig_o = 0; txt--; mul.whi = 0; tmp = 10;
        while (1){
            fmb = *(++txt);
            if (fmb == '.'){
                sig_o--;
                continue;
            };
            fmb -= '0';
            if (fmb > 9u) break;
            if (mul.whi == 0){
                a2a1.wlo = fmb;
                a2a1.whi = mul.whi;
                asm volatile("":::"memory");
                a2a1.w64 += (uint64_t) val.wlo * tmp;
                mul.wlo = a2a1.whi;
                mul.w64 += (uint64_t) val.whi * tmp;
                if (mul.whi != 0) sig_o++;
                else{
                    val.wlo = a2a1.wlo;
                    val.whi = mul.wlo;
                };
            };
            ord += sig_o;
        };
        sig_o |= mul.whi;
        sig_o |= ord;

        if (sig_o != 0){
            raw.tip = raw_double;
            if (val.w64 == 0) goto exit;
            if ((*txt == 'e')||(*txt == 'E')){
                sig_o = 1;
                fmb = *(++txt);
                if (fmb == '-'){
                    sig_o = -1;
                    fmb = *(++txt);
                }else if (fmb == '+') fmb = *(++txt);
                tmp = 0;
                while (1){
                    fmb -= '0';
                    if (fmb > 9u) break;
                    tmp += tmp << 2;
                    tmp = fmb + (tmp << 1);
                    fmb = *(++txt);
                };
                ord += sig_o * tmp;
            };
            sig_o = __builtin_clzll(val.w64);
            val.w64 = shift_left(val.w64, sig_o);

            ord_in = (1023 + 63) - sig_o;
            val.w64 = mult64_do(val.w64, &ord_in, ord);
            ord = 10;
            if(ord_in <= 0){
                ord -= ord_in;
                ord += 1;
                ord_in = 0;
                if(ord > 63) {ord = 63; val.whi = 0;};
            }else val.whi &= 0x7FFFFFFF;
            if (ord_in >= 0x07FF) goto error;
            val.w64 = shift_right(val.w64, ord);
            tmp = val.wlo & 1;
            val.w64 = shift_right(val.w64, 1);
            val.whi |= ord_in << 20;
            val.w64 += tmp;
            goto exit;
        };
    };
    exit0:
    if (val.whi == 0) raw.tip = raw_u32;
    else raw.tip = raw_u64;
    exit:
    raw.L.u64 = val.w64;
    error:
    raw.txt = txt;
    return raw;
};//247



///-----------------------------------------------------------------------


///-------------------------


void test_printo (void){
printo("\f ",__TIME__);
union w64_all
{
    uint32_t    u_raw[2];
    float       f_raw[2];
    double      d_raw;
    uint64_t    u_64;
}ggtt;
ggtt.u_raw[1] = 0x7F800000;
printo("\n inf   float         = ", ggtt.f_raw[1]);
ggtt.u_raw[1] = 0xFF800000;
printo("\n -inf  float         = ", ggtt.f_raw[1]);
ggtt.u_raw[1] = 0x7F800001;
printo("\n nan   float         = ", ggtt.f_raw[1]);
ggtt.u_raw[1] = 0xFF800001;
printo("\n -nan  float         = ", ggtt.f_raw[1]);
ggtt.u_raw[1] = 0;
printo("\n 0     float         = ", ggtt.f_raw[1]);
ggtt.u_raw[1] = 0x80000000;
printo("\n -0    float         = ", ggtt.f_raw[1]);
printo("\n 1.11111111111d      = ", 1.11111111111d );
printo("\n -1.0f               = ", -1.0f," \t\t -1uL = ", -9.99999940395e-1f );
printo("\n 1.5f                = ", 1.5f," \t\t -1uL = ", 1.49999988079f );
printo("\n -2e+2f              = ", -2e+2f," \t\t -1uL = ", -1.99999984741e+2f );
printo("\n 3e+3f               = ", 3e+3f," \t\t -1uL = ", 2.99999975586e+3f );
printo("\n 4e+4f               = ", 4e+4f," \t\t -1uL = ", 3.99999960938e+4f );
printo("\n 5e+5f               = ", 5e+5f," \t\t -1uL = ", 4.9999996875e+5f );
printo("\n 6e+6f               = ", 6e+6f," \t\t -1uL = ", 5.9999995e+6f );
printo("\n 7e+7f               = ", 7e+7f," \t\t -1uL = ", 6.9999992e+7f );
printo("\n 1e+8f               = ", 1e+8f," \t\t -1uL = ", 9.9999992e+7f );
printo("\n 9.000000512e+9f     = ", 9.000000512e+9f," \t -1uL = ", 8.999999488e+9f );
printo("\n 1e+10f              = ", 1e+10f," \t\t -1uL = ", 9.999998976e+9f );
printo("\n 1.1000000512e+11f   = ", 1.1000000512e+11f," \t -1uL = ", 1.09999996928e+11f );
printo("\n 1.20000007373e+12f  = ", 1.20000007373e+12f," \t -1uL = ", 1.19999994266e+12f );
printo("\n 1.30000009298e+13f  = ", 1.30000009298e+13f," \t -1uL = ", 1.29999998812e+13f );
printo("\n 1.40000003883e+14f  = ", 1.40000003883e+14f," \t -1uL = ", 1.39999995494e+14f );
printo("\n 1.50000001404e+15f  = ", 1.50000001404e+15f," \t -1uL = ", 1.49999987982e+15f );
printo("\n 1.60000008656e+16f  = ", 1.60000008656e+16f," \t -1uL = ", 1.59999997919e+16f );
printo("\n 1.70000011076e+17f  = ", 1.70000011076e+17f," \t -1uL = ", 1.69999993896e+17f );
printo("\n 1.80000004047e+18f  = ", 1.80000004047e+18f," \t -1uL = ", 1.79999990303e+18f );
printo("\n 1.90000007326e+19f  = ", 1.90000007326e+19f," \t -1uL = ", 1.89999985336e+19f );
printo("\n 2.00000004008e+20f  = ", 2.00000004008e+20f," \t -1uL = ", 1.99999986416e+20f );
printo("\n 2.10000007727e+21f  = ", 2.10000007727e+21f," \t -1uL = ", 2.09999993653e+21f );
printo("\n 2.20000008631e+22f  = ", 2.20000008631e+22f," \t -1uL = ", 2.19999986113e+22f );
printo("\n 2.30000003906e+23f  = ", 2.30000003906e+23f," \t -1uL = ", 2.29999985891e+23f );
printo("\n 2.40000009088e+24f  = ", 2.40000009088e+24f," \t -1uL = ", 2.39999994677e+24f );
printo("\n 2.50000006344e+25f  = ", 2.50000006344e+25f," \t -1uL = ", 2.49999983286e+25f );
printo("\n 2.60000013977e+26f  = ", 2.60000013977e+26f," \t -1uL = ", 2.5999999553e+26f );
printo("\n 2.70000005007e+27f  = ", 2.70000005007e+27f," \t -1uL = ", 2.69999975492e+27f );
printo("\n 2.80000017436e+28f  = ", 2.80000017436e+28f," \t -1uL = ", 2.79999993824e+28f );
printo("\n 2.90000015697e+29f  = ", 2.90000015697e+29f," \t -1uL = ", 2.89999996808e+29f );
printo("\n 3.00000019626e+30f  = ", 3.00000019626e+30f," \t -1uL = ", 2.99999989403e+30f );
printo("\n 3.10000012221e+31f  = ", 3.10000012221e+31f," \t -1uL = ", 3.09999988042e+31f );
printo("\n 3.20000014487e+32f  = ", 3.20000014487e+32f," \t -1uL = ", 3.19999995144e+32f );
printo("\n 3.30000028358e+33f  = ", 3.30000028358e+33f," \t -1uL = ", 3.2999999741e+33f );
printo("\n 3.40000008961e+34f  = ", 3.40000008961e+34f," \t -1uL = ", 3.39999984202e+34f );
printo("\n 3.50000034129e+35f  = ", 3.50000034129e+35f," \t -1uL = ", 3.49999994514e+35f );
printo("\n 3.60000030576e+36f  = ", 3.60000030576e+36f," \t -1uL = ", 3.59999998885e+36f );
printo("\n 3.70000006425e+37f  = ", 3.70000006425e+37f," \t -1uL = ", 3.69999981072e+37f );
printo("\n 3.40000015497e+38f  = ", 3.40000015497e+38f," \t -1uL = ", 3.39999995214e+38f );
printo("\n 3.40282346639e+38f  = ", 3.40282346639e+38f," float max");
printo("\n 1.0000000149e-1f    = ", 0.10000000149f," \t -1uL = ", 9.99999940395e-2f );
printo("\n 2.00000014156e-2f   = ", 2.00000014156e-2f," \t -1uL = ", 1.9999999553e-2f );
printo("\n 3.00000002608e-3f   = ", 3.00000002608e-3f," \t -1uL = ", 2.99999979325e-3f );
printo("\n 4.00000018999e-4f   = ", 4.00000018999e-4f," \t -1uL = ", 3.99999989895e-4f );
printo("\n 5.00000023749e-05f  = ", 5.00000023749e-05f," \t -1uL = ", 4.99999987369e-05f );
printo("\n 6.00000021223e-06f  = ", 6.00000021223e-06f," \t -1uL = ", 5.99999975748e-06f );
printo("\n 7.00000043707e-07f  = ", 7.00000043707e-07f," \t -1uL = ", 6.99999986864e-07f );
printo("\n 8.00000066192e-08f  = ", 8.00000066192e-08f," \t -1uL = ", 7.99999995138e-08f );
printo("\n 9.00000074466e-09f  = ", 9.00000074466e-09f," \t -1uL = ", 8.99999985648e-09f );
printo("\n 1.00000001335e-10f  = ", 1.00000001335e-10f," \t -1uL = ", 9.99999943962e-11f );
printo("\n 1.10000003897e-11f  = ", 1.10000003897e-11f," \t -1uL = ", 1.09999995224e-11f );
printo("\n 1.20000006026e-12f  = ", 1.20000006026e-12f," \t -1uL = ", 1.19999995184e-12f );
printo("\n 1.30000001107e-13f  = ", 1.30000001107e-13f," \t -1uL = ", 1.29999987554e-13f );
printo("\n 1.40000002625e-14f  = ", 1.40000002625e-14f," \t -1uL = ", 1.39999994155e-14f );
printo("\n 1.50000005838e-15f  = ", 1.50000005838e-15f," \t -1uL = ", 1.4999999525e-15f );
printo("\n 1.60000010639e-16f  = ", 1.60000010639e-16f," \t -1uL = ", 1.59999997404e-16f );
printo("\n 1.70000008822e-17f  = ", 1.70000008822e-17f," \t -1uL = ", 1.69999992279e-17f );
printo("\n 1.80000002043e-18f  = ", 1.80000002043e-18f," \t -1uL = ", 1.79999981363e-18f );
printo("\n 1.90000000433e-19f  = ", 1.90000000433e-19f," \t -1uL = ", 1.89999987508e-19f );
printo("\n 2.00000009809e-20f  = ", 2.00000009809e-20f," \t -1uL = ", 1.99999993653e-20f );
printo("\n 2.10000013531e-21f  = ", 2.10000013531e-21f," \t -1uL = ", 2.09999993336e-21f );
printo("\n 2.20000003116e-22f  = ", 2.20000003116e-22f," \t -1uL = ", 2.19999977872e-22f );
printo("\n 2.30000001536e-23f  = ", 2.30000001536e-23f," \t -1uL = ", 2.29999985759e-23f );
printo("\n 2.4000000469e-24f   = ", 2.4000000469e-24f, " \t -1uL = ", 2.39999984968e-24f );
printo("\n 2.50000004885e-25f  = ", 2.50000004885e-25f," \t -1uL = ", 2.49999980233e-25f );
printo("\n 2.60000018639e-26f  = ", 2.60000018639e-26f," \t -1uL = ", 2.59999987824e-26f );
printo("\n 2.70000009282e-27f  = ", 2.70000009282e-27f," \t -1uL = ", 2.69999990023e-27f );
printo("\n 2.80000000888e-28f  = ", 2.80000000888e-28f," \t -1uL = ", 2.79999976814e-28f );
printo("\n 2.9000000092e-29f   = ", 2.9000000092e-29f,"  \t -1uL = ", 2.89999970827e-29f );
printo("\n 3.00000000951e-30f  = ", 3.00000000951e-30f," \t -1uL = ", 2.99999982143e-30f );
printo("\n 3.10000006625e-31f  = ", 3.10000006625e-31f," \t -1uL = ", 3.09999983116e-31f );
printo("\n 3.20000007598e-32f  = ", 3.20000007598e-32f," \t -1uL = ", 3.1999997821e-32f );
printo("\n 3.30000018855e-33f  = ", 3.30000018855e-33f," \t -1uL = ", 3.29999982121e-33f );
printo("\n 3.40000008991e-34f  = ", 3.40000008991e-34f," \t -1uL = ", 3.39999986032e-34f );
printo("\n 3.50000027825e-35f  = ", 3.50000027825e-35f," \t -1uL = ", 3.49999999126e-35f );
printo("\n 3.600000237e-36f    = ", 3.600000237e-36f," \t -1uL = ", 3.59999987827e-36f );
printo("\n 3.70000012401e-37f  = ", 3.70000012401e-37f," \t -1uL = ", 3.6999998998e-37f );
printo("\n 3.80000025764e-38f  = ", 3.80000025764e-38f," \t -1uL = ", 3.79999997738e-38f );
printo("\n 3.90000140021e-39f  = ", 3.90000140021e-39f," \t -1uL = ", 3.89999999891e-39f );
printo("\n 4.00000646642e-40f  = ", 4.00000646642e-40f," \t -1uL = ", 3.99999245343e-40f );
printo("\n 4.10005917677e-41f  = ", 4.10005917677e-41f," \t -1uL = ", 4.09991904692e-41f );
printo("\n 4.20109279605e-42f  = ", 4.20109279605e-42f," \t -1uL = ", 4.19969149758e-42f );
printo("\n 4.00771360797e-43f  = ", 4.00771360797e-43f," \t -1uL = ", 3.99370062333e-43f );
printo("\n 1.40129846432e-45f  = ", 1.40129846432e-45f," float min");
printo("\n\n double");
printo("\n algorithm limitation - 32 high bits of mantissa out of 52 for double");
ggtt.u_64 = 0x7FF0000000000000ULL;
printo("\n inf   double         = ", ggtt.d_raw);
ggtt.u_64 = 0xFFF0000000000000ULL;
printo("\n -inf  double         = ", ggtt.d_raw);
ggtt.u_64 = 0x7FF0010000000001ULL;
printo("\n nan   double         = ", ggtt.d_raw);
ggtt.u_64 = 0xFFF0000010000010ULL;
printo("\n -nan  double         = ", ggtt.d_raw);
ggtt.u_64 = 0x0000000000000000ULL;
printo("\n 0     double         = ", ggtt.d_raw);
ggtt.u_64 = 0x8000000000000000ULL;
printo("\n -0    double         = ", ggtt.d_raw);
ggtt.u_64 = 0x7FEFFFFFFFFFFFFFULL;
printo("\n 1.79769313486e+308   = ", ggtt.d_raw," double max");
ggtt.u_64 = 0x0000000000000001ULL;
printo("\n 4.940656458e-324     = ", ggtt.d_raw," double min");
ggtt.u_64 = 0x0000000180000001ULL;
printo("\n 3.1829936864e-314   = \t", 3.1829936864e-314d);
printo("\n 2.000000001d        = \t", 2.000000001d);
printo("\n 1.999999999d        = \t", 1.999999999999d);
printo("\n 9.999999999d        = \t", 9.999999999999d);
printo("\n 5.555555555e-51d    = \t", 5.555555555555e-51d);
printo("\n 1.111111111e111d    = \t", 1.111111111111e111d );
printo("\n 2.222222222e222d    = \t", 2.222222222222e222d );
printo("\n -4.444444444e-44d   = \t", -4.444444444444e-44d );
printo("\n 8.888888888e88d     = \t", 8.88888888888e88d);
printo("\n\n whole numbers");
printo("\n 9223372036854775807 = ", 9223372036854775807LL," max int64_t");
printo("\n -9223372036854775807= ", -9223372036854775807LL," min int64_t");
printo("\n 18446744073709551615= ", 18446744073709551615ULL," max uint64_t");
printo("\n 2147483647          = ", 2147483647L," max int32_t");
printo("\n -2147483648         = ", -2147483648L," min int32_t");
printo("\n 4294967295          = ", 4294967295UL," max uint32_t");

printo("\n\n 12345678901234567890= ", 12345678901234567890ULL);
printo("\n 1234567890123456789 = ", 1234567890123456789ULL);
printo("\n 123456789012345678  = ", 123456789012345678ULL);
printo("\n 12345678901234567   = ", 12345678901234567ULL);
printo("\n 1234567890123456    = ", 1234567890123456ULL);
printo("\n 123456789012345     = ", 123456789012345ULL);
printo("\n 12345678901234      = ", 12345678901234ULL);
printo("\n 1234567890123       = ", 1234567890123ULL);
printo("\n 123456789012        = ", 123456789012ULL);
printo("\n 12345678901         = ", 12345678901ULL);
printo("\n 1234567890          = ", 1234567890UL);
printo("\n 123456789           = ", 123456789UL);
printo("\n 12345678            = ", 12345678UL);
printo("\n 1234567             = ", 1234567UL);
printo("\n 123456              = ", 123456UL);
printo("\n 12345               = ", 12345UL);
printo("\n 1234                = ", 1234UL);
printo("\n 123                 = ", 123UL);
printo("\n 12                  = ", 12UL);
printo("\n 1                   = ", 1UL);
printo("\n 0                   = ", 0UL);

printo("\n\n printing structure fields");
printo("\n struct{");
printo("\n    double dou;");
printo("\n    float  fl[2];");
printo("\n    uint64_t u64;");
printo("\n    int32_t i32;");
printo("\n    int8_t i8;");
printo("\n    char t;");
printo("\n    char txt[20];");
printo("\n }test;");

struct
{
    double dou;
    float  fl[2];
    uint64_t u64;
    int32_t i32;
    int8_t i8;
    char t;
    char txt[20];
}test;
test.dou = 5.5555555555e55;
test.fl[0] = 1.00000003137e-22f;
test.fl[1] = 9.99999968266e-23f;
test.i32 = -1234567890;
test.u64 = 1234567891234567890;
test.i8 = -123;
test.t = 'A';
int32_t tmp = 0;
do{test.txt[tmp] = "hello Omsk"[tmp];}while("hello Omsk"[tmp++]!=0);
printo("\n test.dou   5.5555555555e55d    = ", test.dou);
printo("\n test.fl[0] 1.00000003137e-22f  = ", test.fl[0]);
printo("\n test.fl[1] 9.99999968266e-23f  = ", test.fl[1]);
printo("\n test.i32   -1234567890         = ", test.i32);
printo("\n test.u64   1234567891234567890 = ", test.u64);
printo("\n test.i8    -123                = ", test.i8);
printo("\n test.t     'A'                 = ", test.t);
printo("\n test.txt   hello Omsk          = ", test.txt);

printo("\n\n address printing");
printo("\n char* float_char(char* text, float value )");
printo("\n (uintptr_t) &float_char        = ",(uintptr_t) &float_char);

printo("\n gooo\n");
};





