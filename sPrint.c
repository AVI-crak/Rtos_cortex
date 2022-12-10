/**
 @file+   "sPrint.c"
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

#include "sPrint.h"




union float_raw
{
    struct
    {
        uint32_t massa  :23;
        uint32_t order  :8;
        uint32_t sign   :1;
    };
    uint32_t    u_raw;
    float       f_raw;
};

union double_raw
{
    struct
    {
        uint64_t massa  :52;
        uint64_t order  :11;
        uint64_t sign   :1;
    };
    uint32_t    u_raw[2];
    uint64_t    u64_raw;
    double      d_raw;
};

union divrev
{
    uint64_t w64;
    uint32_t w32[2];
};



const int16_t data_otner[133] = {
-327,-322,-317,-313,-308,-303,-298,-293,-289,-284,-279,-274,-269,-264,-260,-255,-250,-245,-240,-236,
-231,-226,-221,-216,-211,-207,-202,-197,-192,-187,-183,-178,-173,-168,-163,-158,-154,-149,-144,-139,
-134,-130,-125,-120,-115,-110,-105,-101,-96,-91,-86,-81,-77,-72,-67,-62,-57,-53,-48,-43,
-38,-33,-28,-24,-19,-14,-9,-4,0,5,10,15,20,25,29,34,39,44,49,53,
58,63,68,73,78,82,87,92,97,102,106,111,116,121,126,131,135,140,145,150,
155,159,164,169,174,179,183,188,193,198,203,208,212,217,222,227,232,236,241,246,
251,256,261,265,270,275,280,285,289,294,299,304,309,};

const uint32_t data_of10raw[133] = {
1206214955,790505035,518065379,3395193265,2225073860,1458224404,955661947,626302614,4104536801,2689949240,
1762885134,1155324400,757153401,496208051,3251949089,2131197354,1396701497,915342294,599878726,3931365217,
2576459509,1688508503,1106580933,725208881,475272893,3114748422,2041281526,1337774260,876723741,574569670,
3765499791,2467757943,1617269845,1059893965,694612109,455220993,2983336292,1955159273,1281333182,839734514,
550328410,3606632274,2363642526,1549036766,1015176736,665306226,436015089,2857468478,1872670543,1227273367,
804305874,527109899,3454467423,2263919770,1483682460,972346138,637236766,4176194860,2736911063,1793662035,
1175494352,770371977,504870979,3308722450,2168404344,1421085472,931322574,610351562,4000000000,2621440000,
1717986920,1125899908,737869764,483570327,3169126500,2076918744,1361129469,892029808,584600655,3831238852,
2510840695,1645504557,1078397867,706738825,463168357,3035420146,1989292946,1303703026,854394816,559936185,
3669597787,2404907604,1576080248,1032899952,676921314,443627153,2907354899,1905364106,1248699421,818347653,
536312317,3514776402,2303443863,1509584969,989321605,648361809,4249103944,2784692759,1824976248,1196016434,
783821329,513685148,3366486977,2206260907,1445895147,947581844,621007236,4069833027,2667205774,1747979977,
1145556157,750751684,492012623,3224453926,2113178124,1384892417,907603094,594806765,3898125605,2554675596,
1674232198,1097224813,719077255,};

const char reserve_char[10] = { '0',0,'i','n','f',0,'N','a','N',0,};
char* floating_char(char* txt, uint32_t massa, int32_t feeze);
char* massa_char (char* tail_txt, uint32_t value);

__attribute__ ((optimize("-Os"))) char* hex_char(char* tail_txt, uintptr_t value)//44
{
    int32_t tmp, jump; jump = 0;
    *tail_txt = 0;
    do{
        tmp = value & 0x0F;
        if (tmp > 9 ) tmp += 0x37; else tmp += 0x30;
        *(--tail_txt) = tmp;
        jump += 4095;
        value >>= 4;
    }while ((value )||((jump & 4096) == 0));
    *(volatile char*)--tail_txt = 'x';
    *(volatile char*)--tail_txt = '0';
    return tail_txt;
};

__attribute__ ((optimize("-Os"))) char * i32_char(char* tail_txt, int32_t value)//22
{
    int32_t mi = value >> 31;
    if (mi) value = 0 - value;
    tail_txt = u32_char(tail_txt, value);
    *(tail_txt - 1) = '-';
    tail_txt += mi;
    return tail_txt;
};

__attribute__ ((optimize("-O3"))) char* u32_char (char* tail_txt, uint32_t value)//40
{
    *tail_txt = 0;
    uint32_t res, tmp;
    do{
        tmp = ((uint64_t) value * 3435973837UL >> 32);
        res = value + '0';
        value = tmp >> 3;
        res -= value * 10;
        *(--tail_txt) = res;
    }while (value > 0);
    return tail_txt;
};

char* massa_char (char* tail_txt, uint32_t value)
{
    *tail_txt = 0;
    uint32_t res, tmp;
    int32_t cn = 9 - ROUND_FLOAT;
    tmp = 499999999UL - (value >> 1);
    cn -= (int32_t) tmp >> 31;
    do{
        tmp = ((uint64_t) value * 3435973837UL >> 32);
        res = value + '0';
        value = tmp >> 3;
        res -= value * 10;
        --cn;
        if (cn < 0) *(--tail_txt) = res;
        else
        {
            *(--tail_txt) = '0';
            if  (res > '5') value++;
        };
    }while (value > 0);
    return tail_txt;
};

__attribute__ ((optimize("-Os"))) char* i64_char(char* tail_txt, int64_t value)//32
{
    if (value >= 0) tail_txt = u64_char(tail_txt, value);
    else
    {
        value = 0 - value;
        tail_txt = u64_char(tail_txt, value);
        *(--tail_txt) = '-';
    };
    return tail_txt;
};

__attribute__ ((optimize("-Os")))char* u64_char (char* tail_txt, uint64_t value)//100
{
    uint32_t res, t10, magh, magl;
    union divrev rev;
    union divrev rew;
    rev.w64 = value;
    *tail_txt = 0;
    magh = 3435973837UL; t10 = 10; magl = 429496730UL;
    do{
        rew.w32[1] = ((uint64_t)rev.w32[1] * magh >> 35);
        rew.w32[0] = 0;
        rev.w64 = (uint64_t)rev.w64 - rew.w64 * t10;
        res = ((uint64_t)rev.w64 >> 16);
        res = ((uint64_t)res * magl >> 32);
        rew.w32[0] = res << 16;
        rev.w64 = (uint64_t)rev.w64 - rew.w32[0] * t10;
        res = ((uint64_t)rev.w32[0] * magl >> 32);
        rew.w32[0] += res;
        rev.w32[0] += '0';
        res = (rev.w32[0] - res * t10);
        *(--tail_txt) = res;
        rev.w64 = (uint64_t)rew.w64;
    }while (rev.w32[1]);
    if (rev.w32[0] == 0) return tail_txt;
    do{
        rev.w32[1] = ((uint64_t)rev.w32[0] * magh >> 32);
        res = rev.w32[0] + '0';
        rev.w32[0] = rev.w32[1] >> 3;
        res -=  rev.w32[0] * t10;
        *(--tail_txt) = res;
    }while (rev.w32[0]);
    return tail_txt;
};

__attribute__ ((optimize("-Os"))) void compress_char(char* tex_in, char* tex_out)//38
{
    int32_t ovr, tmp;
    ovr =  (int32_t)(tex_out - tex_in) - OUT_TXT_SIZE_FLOATING;
    if (ovr > 0)
    {
        *(tex_in + OUT_TXT_SIZE_FLOATING) = 0;
        ovr += 2;
        tmp = ovr - 10;
        if (tmp < 0)
        {
            *(tex_in + (OUT_TXT_SIZE_FLOATING - 1)) = ovr + '0';
            *(tex_in + (OUT_TXT_SIZE_FLOATING - 2)) = 'E';
        }else
        {
            *(tex_in + (OUT_TXT_SIZE_FLOATING - 1)) = tmp + 1 + '0';
            *(tex_in + (OUT_TXT_SIZE_FLOATING - 2)) = '1';
            *(tex_in + (OUT_TXT_SIZE_FLOATING - 3)) = 'E';
        };
    };
};

__attribute__ ((optimize("-Os"))) char* float_char(char* text, float value )//132
{
    union float_raw    ftemp;
    ftemp.f_raw = value;
    uint32_t feeze, of10raw, cis;
    int32_t order10;
    feeze = (int32_t)ftemp.u_raw >> 31;
    cis = 45 & feeze;
    *text = cis;
    ftemp.sign = 0;
    if (ftemp.u_raw != 0)
    {
        if (ftemp.u_raw < 0x7f800000UL)
        {
            feeze = ftemp.u_raw >> 23;
            if (feeze == 0)
            {
                cis = __builtin_clz(ftemp.u_raw);
                cis -= 8;
                ftemp.u_raw <<= cis;
                feeze = 1 - cis;
            };
            feeze += 960;
            ftemp.u_raw <<= 8;
            ftemp.u_raw |= 1 << 31;
            text = floating_char(text ,ftemp.u_raw, feeze);
            return text;
        }else if (ftemp.u_raw > 0x7f800000UL) ftemp.u_raw = 6; // l_NaN;
        else ftemp.u_raw = 2; // l_inf;
    }; // l_zero;
    char *tex; char une;
    tex = text - feeze;
    do{une = reserve_char[ftemp.u_raw++]; *tex++ = une;}while(une);
    return text;
};

__attribute__ ((optimize("-Os"))) char* double_char(char* text, double value )//140
{
    union double_raw    dtemp;
    dtemp.d_raw = value;
    uint32_t feeze, cis, ciz;

    feeze = (int32_t)dtemp.u_raw[1]>> 31;
    cis = 45 & feeze;
    *text = cis;
    dtemp.sign = 0;
    if (dtemp.u64_raw != 0x0000000000000000ULL) // no goto l_zero;
    {
        dtemp.sign = 1;
        if (dtemp.u64_raw < 0xFFF0000000000000ULL)
        {
            feeze = (uint32_t) dtemp.order;
            if (feeze != 0)
            {
                feeze += 64;
                dtemp.u64_raw <<= 11;
                dtemp.u_raw[1] |= 1 << 31;
            }else
            {
                dtemp.sign = 0;
                cis = __builtin_clz((int32_t)dtemp.u_raw[0]);
                ciz = __builtin_clz((int32_t)dtemp.u_raw[1]);
                if (dtemp.u_raw[1] == 0)
                {
                    dtemp.u_raw[1] = dtemp.u_raw[0] << cis;
                    feeze = 44 - cis;
                }else
                {
                    dtemp.u_raw[1] = dtemp.u_raw[1] << ciz;
                    feeze = 76 - ciz;
                    ciz = 32 - ciz;
                    dtemp.u_raw[1] |= dtemp.u_raw[0] >> ciz;
                };
            };
            text = floating_char(text ,dtemp.u_raw[1], feeze );
            return text;
        }else if (dtemp.u64_raw > 0xFFF0000000000000ULL) dtemp.u_raw[0] = 6;// l_NaN;
        else dtemp.u_raw[0] = 2;
    };
    char *tex; char une;
    tex = text - feeze;
    do{ une = reserve_char[dtemp.u_raw[0]++]; *tex++ = une;}while(une);
    return text;
};

__attribute__((optimize("-Os"))) char* floating_char(char* txt, uint32_t massa, int32_t feeze)//208
{

    uint32_t of10raw, cis;
    int32_t order10;
    cis = (uint32_t) feeze >> 4;
    feeze &= 0x0FUL;
    if (feeze > 8) {feeze -= 16;cis += 1;};
    order10 = data_otner[cis];
    of10raw = data_of10raw[cis];

    if (feeze > 0)
    {
        do{
            if ((of10raw >> 31 ) !=0)
            {
                order10++;
                of10raw = ((uint64_t) of10raw * 3435973837UL >> 34);
            }else of10raw <<= 1;
        }while (--feeze);
    }else if (feeze < 0)
    {
        do{
            if (of10raw < 858993459UL)
            {
                order10--;
                of10raw *= 5;
            }else of10raw >>= 1;
        }while (++feeze);
    };
    massa = ((uint64_t) massa * of10raw >> 32);
    char *mas_s, *mas_f, *ord_s, *ord_f;
    int32_t tmp, tff;
    mas_f = txt + 13;
#if ((ROUND_FLOAT > 1)&& (ROUND_FLOAT < 9))
    mas_s = massa_char (mas_f, massa);
#else
    mas_s = u32_char (mas_f, massa);
#endif
    tmp = mas_f - mas_s; tmp -= 10;
    order10 += tmp;
    *(mas_s - 1) = *mas_s; *mas_s = '.';
    if ( *txt == '-')
    {
        mas_s -= 2; *mas_s = '-';
    }else mas_s -= 1;
    if (order10 != 0)
    {
        ord_f = txt + 19;
        ord_s = i32_char(ord_f, order10);
        *(--ord_s) = 'e'; tff = ord_f - ord_s;
    }else tff = 0;
    tmp = mas_f - mas_s + tff;
    if (tmp > OUT_TXT_SIZE_FLOATING)
    {
         mas_f = mas_s + OUT_TXT_SIZE_FLOATING - tff;
    }else ;
    do{tmp = *(--mas_f);} while (tmp == '0');
    if (tmp != '.') mas_f += 1;
    *mas_f = 0;
    if (tff != 0 ) do{ tmp = *(ord_s++); *(mas_f++) = tmp; }while(tmp);
    return mas_s;
};


Raw_TypeDef scano(char* txt)
{
    Raw_TypeDef raw;
    uint64_t tmp; uint32_t fb; int32_t fl;
    raw.L.u64 = 0;
    while(1)
    {
        fb = *(uint8_t*) txt++;
        fl = fb - '0';
        if (!((fl >= 0)&& (fl <= 9)))break;
        tmp =(uint64_t)raw.L.u64 * 10 + fl;
        if (raw.L.u32[1] > (tmp >> 32))goto scano_L_error;
        raw.L.u64 = tmp;
    };

    if (((fb == 'x')|| (fb == 'X')) && (raw.L.u64 == 0))
    {
        int32_t si = 18;  int32_t fs;
        while(1){
            if (--si == 0) goto scano_x_error;
            fb = *(uint8_t*) txt++;
            fl = fb - '0';
            if ((fl >= 0)&&(fl <= 9)) goto scano_x_add;
            fl -= 7; fs = fb - 'A';
            if ((fs >= 0)&&(fs <= 5))goto scano_x_add;
            fl -= 32; fs = fb - 'a';
            if (!((fs >= 0)&&(fs <= 5))) goto scano_x_end;
            scano_x_add:
            raw.L.u64 <<= 4;
            raw.L.u32[0] |= fl;
        };
    };
    scano_x_end:
    raw.txt = txt;
    if ( raw.L.u32[1] == 0) raw.Lsize = 32 - __builtin_clz(raw.L.u32[0]);
    else raw.Lsize = 64 - __builtin_clz(raw.L.u32[1]);

    return raw;

    scano_x_error:
    raw.L.u64 = 0;
    scano_L_error:
    raw.Traw = raw_error;
    raw.Lsize = 0;
    return raw;
};




///-----------------------------------------------------------------------

/*

// code optimizer left chat
void tabl_grabl(void)
{
    volatile uint32_t cis;
    volatile uint64_t of10raw;
    volatile uint64_t of10raw_s;
    volatile uint64_t of10raw_l;
    volatile int16_t ofreze;
    volatile int32_t cis2;
    volatile uint32_t cis3;
    volatile int32_t cis4;
    volatile int32_t cis5;
    volatile int32_t fff;
    printo("\f\n");

    cis5 = 2;
    while (cis5){
    if (cis5 == 2)printo("const int16_t data_otner[133] = {\n");
    if (cis5 == 1)printo("const uint32_t data_of10raw[133] = {\n");
    cis4 = 0;
    for(cis2 = 0; cis2 !=133; cis2++)
    {
        cis = cis2 <<4;
        of10raw = 8589934592000000000ULL;
        ofreze = 0;
        if (cis > 1087){
            while ( (cis--) != 1087){
                if ((of10raw >> 63 ) !=0)
                    {
                       ofreze += 1;
                       do{
                              of10raw_s = (uint64_t)of10raw / 5ULL;
                              of10raw_l = (uint64_t)of10raw_s * 5ULL;
                              if(of10raw_l != of10raw){
                                  of10raw = (uint64_t)of10raw +1; fff = 1;
                              }else
                              {
                                  fff = 0; of10raw = (uint64_t) of10raw_s;
                              }
                       }while(fff);
                    }else of10raw <<= 1;};}
        else if (cis < 1087){
            while ( (cis++) != 1087){
                if (of10raw < 3689348814741910323ULL)
                    {
                       ofreze -= 1; of10raw = (uint64_t)of10raw * 5ULL;
                    }else of10raw >>= 1;};};
        cis3 = (uint32_t) (of10raw >> 32);
     //   if((cis3 & 2047) > 1500) cis3 += 2;
     //   else if((cis3 & 2047) > 750) cis3 += 1;
        if (cis5 == 2){cis4++; printo(ofreze,","); if(cis4 > 19){cis4 = 0; printo("\n");};};
        if (cis5 == 1){cis4++; printo( cis3,","); if(cis4 > 9){cis4 = 0; printo("\n");};};
    }; printo("};\n\n");
    cis5--;};
};
*/




///-------------------------
/**
216MGz
float = 351tik   +1280b          printf float = 5259tik  +8208b
double = 301tik  +1376b          printf double = 1570tik  +8208b
int64_t = 487tik +192b           printf int64_t = 3387tik  +8236b
int32_t = 150tik +140b           printf int32_t = 745tik  +8208b
all_format + 1684b
**/

void test_printo (void){
printo("\f ",__TIME__);
 union float_rawww
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

printo("\n\n compressor check, restriction =",(int32_t) OUT_TXT_SIZE_FLOATING, " symbol");
printo("\n 12345678901234567890= ", 12345678901234567890ULL);
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





