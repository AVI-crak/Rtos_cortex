/// sPrint.c
/// печать без указани€ типа параметра, в разнобой
/// без зависимостей от внешних библиотек
/// printo("текст", double / float / uint(8-32-64)_t / int(8-32-64)_t )
/// размер - 1907 байт при агрессивной оптимизации
/// на вкус и цвет... добавить собственную функцию печати в
/// soft_print()

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

const int16_t data_otner[129] = {
-308,-303,-298,-293,-289,-284,-279,-274,-269,-264,-260,-255,-250,-245,-240,-236,-231,-226,-221,-216,
-211,-207,-202,-197,-192,-187,-183,-178,-173,-168,-163,-158,-154,-149,-144,-139,-134,-130,-125,-120,
-115,-110,-105,-101,-96,-91,-86,-81,-77,-72,-67,-62,-57,-53,-48,-43,-38,-33,-28,-24,
-19,-14,-9,-4,0,5,10,15,20,25,29,34,39,44,49,53,58,63,68,73,
78,82,87,92,97,102,106,111,116,121,126,131,135,140,145,150,155,159,164,169,
174,179,183,188,193,198,203,208,212,217,222,227,232,236,241,246,251,256,261,265,
270,275,280,285,289,294,299,304,309,};

const uint32_t data_of10raw[129] = {
2225073858,1458224403,955661945,626302612,4104536801,2689949238,1762885132,1155324400,757153399,496208051,
3251949087,2131197353,1396701497,915342293,599878725,3931365215,2576459507,1688508503,1106580932,725208879,
475272891,3114748422,2041281525,1337774260,876723739,574569669,3765499789,2467757941,1617269844,1059893965,
694612109,455220991,2983336292,1955159272,1281333180,839734513,550328410,3606632272,2363642526,1549036765,
1015176734,665306225,436015087,2857468478,1872670541,1227273366,804305873,527109897,3454467422,2263919769,
1483682460,972346137,637236764,4176194859,2736911063,1793662034,1175494350,770371977,504870979,3308722450,
2168404344,1421085471,931322574,610351562,4000000000,2621440000,1717986918,1125899906,737869762,483570327,
3169126500,2076918743,1361129467,892029807,584600654,3831238852,2510840694,1645504557,1078397866,706738825,
463168356,3035420144,1989292945,1303703024,854394814,559936185,3669597785,2404907604,1576080247,1032899951,
676921312,443627151,2907354897,1905364105,1248699420,818347651,536312317,3514776401,2303443862,1509584969,
989321605,648361807,4249103942,2784692759,1824976247,1196016433,783821329,513685146,3366486976,2206260905,
1445895146,947581843,621007236,4069833027,2667205773,1747979975,1145556156,750751682,492012622,3224453925,
2113178124,1384892415,907603093,594806763,3898125604,2554675596,1674232198,1097224813,719077253,};



const char txt_NaN[] = "NaN";
char* floating_char(uint32_t massa, uint32_t of10raw, int32_t feeze, int32_t order10, char* txt);


__attribute__ ((optimize("-O1"))) char* hex_char(char* tail_txt, uint32_t value)//48
{
    int32_t tmp, jump; jump = 0;
    *tail_txt = 0;
    do{
        tmp = value & 0x0F;
        if (tmp > 9 ) tmp += 0x37; else tmp += 0x30;
        *(--tail_txt) = tmp;
        jump += 7;
        value >>= 4;
    }while ((value )||(jump & 1));
    *(volatile char*)--tail_txt = 'x';
    *(volatile char*)--tail_txt = '0';
    return tail_txt;
};

__attribute__ ((optimize("-Os"))) char * i32_char(char* tail_txt, int32_t value)//32
{
    if (value < 0){
        value = 0 - value;
        tail_txt = u32_char(tail_txt, value);
        *(--tail_txt) = '-';
    }else tail_txt = u32_char(tail_txt, value);
    return tail_txt;
};


__attribute__ ((optimize("-Os"))) char* u32_char (char* tail_txt, uint32_t value)//40
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

__attribute__ ((optimize("-Os")))char* u64_char (char* tail_txt, uint64_t value)//104
{
    uint32_t res, t10, mag;
    union divrev rev;
    union divrev rew;
    union divrev reg;
    rev.w64 = value;
    *tail_txt = 0;
    mag = 3435973837UL; t10 = 10;
    do{
        rew.w32[1] = ((uint64_t)rev.w32[1] * mag >> 35);
        rew.w32[0] = 0;
        reg.w64 = (uint64_t)rev.w64 - rew.w64 * t10;
        res = ((uint64_t)reg.w64 >> 8);
        res = ((uint64_t)res * mag >> 35);
        rew.w64 += (uint64_t)res << 8;
        reg.w64 = (uint64_t)rev.w64 - rew.w64 * t10;
        res = ((uint64_t)reg.w32[0] * mag >> 35);
        rew.w64 += (uint64_t)res;
        reg.w32[0] += '0';
        res = (reg.w32[0] - res * t10);
        *(--tail_txt) = res;
        rev.w64 = (uint64_t)rew.w64;
    }while (rev.w32[1]);
    if (rev.w32[0] == 0) return tail_txt;
    do{
        rev.w32[1] = ((uint64_t)rev.w32[0] * mag >> 32);
        res = rev.w32[0] + '0';
        rev.w32[0] = rev.w32[1] >> 3;
        res -=  rev.w32[0] * t10;
        *(--tail_txt) = res;
    }while (rev.w32[0]);
    return tail_txt;
};


__attribute__ ((optimize("-Os"))) void compress_char(char* tex_in, char* tex_out)//68
{
    int32_t ovr;
    ovr =  (int32_t)(tex_out - tex_in) - OUT_TXT_SIZE_FLOATING;
    if (ovr > 0)
    {
        ovr +=2;
        if(ovr <= 9)
        {
            *(tex_in + (OUT_TXT_SIZE_FLOATING - 2)) = 'E';
            *(tex_in + (OUT_TXT_SIZE_FLOATING - 1)) = ovr + '0';
        }else
        {
            ovr++; uint32_t tmp;
            *(tex_in + (OUT_TXT_SIZE_FLOATING - 3)) = 'E';
            tmp = ((uint64_t) ovr * 0x1999999A >> 32);
            *(tex_in + (OUT_TXT_SIZE_FLOATING - 2)) = tmp + '0';
            *(tex_in + (OUT_TXT_SIZE_FLOATING - 1)) = ovr + '0'- tmp * 10;
        };
        *(tex_in + OUT_TXT_SIZE_FLOATING) = 0;
    };
};


__attribute__ ((optimize("-Os"))) char* float_char(float value, char* text)//168
{
    union float_raw    ftemp;
    ftemp.f_raw = value;
    int32_t sign;
    uint32_t order;
    uint32_t massa, of10raw;
    int32_t feeze, order10, cis;

    sign = ftemp.sign; order = ftemp.order;

    feeze = 0;
    if (sign)  text[feeze++] = '-';else text[feeze] = 0;
    if (order == 255)
    {
        if (ftemp.massa == 0)
        {
            text[feeze++] = 'I'; text[feeze++] = 'i';
            text[feeze++] = 'f'; text[feeze] = 0;
            return text;
        }else return (char*)&txt_NaN[0];
    }else if ((order == 0) && (ftemp.massa == 0))
    {
        text[feeze++] = '0';
        text[feeze] = 0;
        return text;
    };

    cis = (order + 896) >> 4;
    feeze = order & 0x000F;
    if (feeze > 7) cis++;
    order10 = data_otner[cis];
    of10raw = data_of10raw[cis];

    if (order == 0)
    {
        ftemp.u_raw <<= 9;
        while (ftemp.sign == 0)
        {
            ftemp.u_raw <<= 1;
            if (of10raw < 858993459)
                {
                    order10--;
                    of10raw *= 5;
                }else of10raw >>= 1;
        };
        massa = ftemp.u_raw;
    }else
    {
        massa = ftemp.u_raw << 8;
        massa |= (uint32_t) 1 << 31;
    };

    text = floating_char( massa, of10raw, feeze,  order10, text);
    return text;
};

__attribute__ ((optimize("-Os"))) char* double_char(double value, char* text)//204
{
    union double_raw    dtemp;
    dtemp.d_raw = value;
    int_fast8_t sign;
    uint32_t order;
    uint32_t massa, of10raw;
    int32_t feeze, order10, cis;
    sign = dtemp.sign; order = dtemp.order;

    feeze = 0;
    if (sign)  text[feeze++] = '-';else text[feeze] = 0;
    if (order == 2047)
    {
        if (dtemp.massa == 0)
        {
            text[feeze++] = 'I'; text[feeze++] = 'i';
            text[feeze++] = 'f'; text[feeze] = 0;
            return text;
        }else return (char*)&txt_NaN[0];
    }else if ((order == 0) && (dtemp.massa == 0))
    {
        text[feeze++] = '0';
        text[feeze] = 0;
        return text;
    };

    cis = order >> 4;
    feeze = order & 0x000F;
    if (feeze > 7) cis++;
    order10 = data_otner[cis];
    of10raw = data_of10raw[cis];

    if (order == 0)
    {
        dtemp.u64_raw <<= 12;
        while (dtemp.sign == 0)
        {
            dtemp.u64_raw <<= 1;
            if (of10raw < 858993459)
                {
                    order10--;
                    of10raw *= 5;
                }else of10raw >>= 1;
        };
        massa = dtemp.u_raw[1];
    }else
    {
        dtemp.u64_raw <<= 11;
        massa = dtemp.u_raw[1];
        massa |= (uint32_t) 1 << 31;
    };

    text = floating_char( massa, of10raw, feeze,  order10, text);
    return text;
};

__attribute__ ((optimize("-Os"))) char* floating_char(uint32_t massa, uint32_t of10raw, int32_t feeze, int32_t order10, char* txt)//264
{
    if (feeze > 7)
    {
        while ( (feeze++) != 16)
        {
            if (of10raw < 858993459)
                {
                    order10--;
                    of10raw *= 5;
                }else of10raw >>= 1;
        };
    }else
    {
        while ( (feeze--) != 0)
        {
            if ((of10raw >> 31 ) !=0)
                {
                    order10++;
                    of10raw = ((uint64_t) of10raw * 3435973837UL >> 34);
                }else of10raw <<= 1;
        };
    };
    massa = ((uint64_t) massa * of10raw >> 32);
    char *mas_s, *mas_f, *ord_s, *ord_f; int32_t tmp, tff;
    mas_f = txt + 12;
    mas_s = u32_char (mas_f, massa);


    tmp = mas_f - mas_s; tmp -= 10;
    order10 += tmp;
    if ((-12 <= order10)&&(order10 < 15))
    {
        tmp = (order10 + 12) / 3; tmp *= 3; tmp -= 12;
        tff = order10 - tmp; order10 = tmp;
    } else tff = 0;
    if (order10 != 0)
    {
        ord_f = txt + 19;
        ord_s = i32_char(ord_f, order10);
        *(--ord_s) = 'e';
        tmp = ord_f - ord_s;
    };
    if (*txt == '-') {*(mas_s - 2) = *txt; txt = mas_s - 2;}else txt = mas_s - 1;
    *(mas_s - 1) = *mas_s;
    if (tff > 0) { *mas_s = *(mas_s + 1); mas_s++; };
    if (tff == 2) { *mas_s = *(mas_s + 1); mas_s++;};
    *mas_s = '.'; mas_s++;
    ord_f = txt + OUT_TXT_SIZE_FLOATING - tmp;
    if (mas_f > ord_f ) mas_f = ord_f;
    *(mas_f--) = 0;
    while ((*(mas_f) == '0') && (mas_f != mas_s)) {*(mas_f) = 0; mas_f--;};
    mas_f++;
    if (order10 != 0 ) do{ tmp = *(ord_s++); *(mas_f++) = tmp; }while(tmp);
    return txt;
};

///-----------------------------------------------------------------------



/**
void tabl_grabl(void)
{
    volatile uint32_t cis;
    volatile uint64_t of10raw;
    volatile int16_t ofreze;
    volatile int32_t cis2;
    volatile uint32_t cis3;
    volatile int32_t cis4;
    volatile int32_t cis5;
    printo("\f\n");

    cis5 = 2;
    while (cis5){
    if (cis5 == 2)printo("const int16_t data_otner[129] = {\n");
    if (cis5 == 1)printo("const uint32_t data_of10raw[129] = {\n");
    cis4 = 0;
    for(cis2 = 0; cis2 !=129; cis2++)
    {
        cis = cis2 <<4;
        of10raw = 8589934592000000000;
        ofreze = 0;
        if (cis > 1023){
            while ( (cis--) != 1023){
                if ((of10raw >> 63 ) !=0){
                    ofreze += 1; of10raw /=5;}
                else of10raw <<= 1;};}
        else if (cis < 1023){
            while ( (cis++) != 1023){
                if (of10raw < 3689348813882916864){
                    ofreze -= 1; of10raw *=5;}
                    else of10raw >>= 1;};};
        cis3 = (uint32_t) (of10raw >> 32);
        if (cis5 == 2){cis4++; printo(ofreze,","); if(cis4 > 19){cis4 = 0; printo("\n");};};
        if (cis5 == 1){cis4++; printo( cis3,","); if(cis4 > 9){cis4 = 0; printo("\n");};};
    }; printo("};\n\n");
    cis5--;};
};
**/

///-------------------------
/**
printo("\f");
printo("\n 5.64231125e+5f = ", 5.64231125e+5f);
printo("\n -8.65399996416e+12f = ", -8.65399996416e+12f);
printo("\n 9.99999949672e+20f  = ", 9.99999949672e+20f);
printo("\n -2.54999995232f     = ", -2.54999995232f);
printo("\n 0.100000001490f     = ", 0.100000001490f);
printo("\n -1.0000000000f      = ", -1.0f);
printo("\n 25.5522403717f      = ", 2.55522403717e1f);
printo("\n 3141592.75f         = ", 3141592.75f);
printo("\n 1.95234170378e-30f  = ", 1.95234170378e-30f);
printo("\n 4.8540005082e-06f  = ", 4.8540005082e-06f);
printo("\n 1.99999988079f      = ", 1.99999988079f);
printo("\n 2.00000023842f      = ", 2.00000023842f);
printo("\n 3.40282346639e+38f  = ", 3.40282346639e+38f);
printo("\n 4.20389539297E-45f  = ", 4.20389539297E-45f);
printo("\n 1.40129846432e-45f  = ", 1.40129846432e-45f);
printo("\n 2.2e-307d           = ", 2.2e-307d);
printo("\n 2.000000001d        = ", 2.000000001d);
printo("\n 1.999999999d        = ", 1.999999999d);
printo("\n 9.999999999d        = ", 9.999999999d);
printo("\n 3.21654987e-3d      = ", 3.21654987e-3d);
printo("\n 1.74554255000e-288d = ", 1.74554255000e-288d);
printo("\n 3.243552454547e+66d = ", 3.243552454547e+66d);
printo("\n 7.345624524111e+24d = ", 7.345624524111e+24d);
printo("\n 1.2345678912345678d = ", 1.2345678912345678d);

printo("\n -4567891234567891234          =", -4567891234567891234LL);
printo("\n -456789123456789123           =", -456789123456789123LL);
printo("\n -45678912345678912            =", -45678912345678912LL);
printo("\n -4567891234567891             =", -4567891234567891LL);
printo("\n -456789123456789              =", -456789123456789LL);
printo("\n -45678912345678               =", -45678912345678LL);
printo("\n -4567891234567                =", -4567891234567LL);
printo("\n -456789123456                 =", -456789123456LL);
printo("\n -45678912345                  =", -45678912345LL);
printo("\n -4567891234                   =", -4567891234LL);
printo("\n -456789123                    =", -456789123L);
printo("\n -45678912                     =", -45678912L);
printo("\n -4567891                      =", -4567891L);
printo("\n -456789                       =", -456789L);
printo("\n -45678                        =", -45678L);
printo("\n -4567                         =", -4567L);
printo("\n -456                          =", -456L);
printo("\n -45                           =", -45L);
printo("\n -4                            =", -4L);
printo("\n 0                             =", 0L);

printo("\n 18446744073709551614          =", 18446744073709551614ULL);
printo("\n 1844674407370955161           =", 1844674407370955161ULL);
printo("\n 184467440737095516            =", 184467440737095516ULL);
printo("\n 18446744073709551             =", 18446744073709551ULL);
printo("\n 1844674407370955              =", 1844674407370955ULL);
printo("\n 184467440737095               =", 184467440737095ULL);
printo("\n 18446744073709                =", 18446744073709ULL);
printo("\n 1844674407370                 =", 1844674407370ULL);
printo("\n 184467440737                  =", 184467440737ULL);
printo("\n 18446744073                   =", 18446744073ULL);
printo("\n 1844674407                    =", 1844674407ULL);
printo("\n 184467440                     =", 184467440ULL);
printo("\n 18446744                      =", 18446744ULL);
printo("\n 1844674                       =", 1844674ULL);
printo("\n 184467                        =", 184467ULL);
printo("\n 18446                         =", 18446UL);
printo("\n 1844                          =", 1844UL);
printo("\n 184                           =", 184UL);
printo("\n 18                            =", 18UL);
printo("\n 1                             =", 1UL);
printo("\n 0                             =", 0UL);



char* ntst_texx = "helou words";
printo("\n char helou words              =", ntst_texx);
printo("\n test tekst");
uint16_t *teat_adress; teat_adress = ntst_texx;
*teat_adress = 5555;
printo("\n uint16_t *teat_adress         =", teat_adress);
printo("\n uint16_t *teat_adress         =", *teat_adress);
printo("\n uint16_t &teat_adress         =", &teat_adress);
volatile int64_t i64temp1 = -9223372036854775808;
printo("\n int64_t -9223372036854775808  =", i64temp1);
i64temp1 = -1;
printo("\n int64_t -1                    =", i64temp1);
i64temp1 = 9223372036854775807;
printo("\n int64_t 9223372036854775807   =", i64temp1);
i64temp1 = 2;
printo("\n int64_t 2                     =", i64temp1);
i64temp1 = 0;
printo("\n int64_t 0                     =", i64temp1);

volatile int32_t i32temp2 = -1;
printo("\n int32_t -1                    =", i32temp2);
i32temp2 = 1;
printo("\n int32_t 1                     =", i32temp2);
i32temp2 = -2147483648;
printo("\n int32_t -2147483648           =", i32temp2);
i32temp2 = -1;
printo("\n int32_t i32temp2              =", i32temp2);
i32temp2 = 0;
printo("\n int32_t 0                     =", i32temp2);


volatile int16_t ui8temp3 = -32768;
printo("\n int16_t -32768                =", ui8temp3);
ui8temp3 = -1;
printo("\n int16_t -1                    =", ui8temp3);
ui8temp3 = 0;
printo("\n int16_t 0                     =", ui8temp3);
ui8temp3 = 1;
printo("\n int16_t 1                     =", ui8temp3);
ui8temp3 = 32767;
printo("\n int16_t 32767                 =", ui8temp3);

volatile uint8_t ui8tegg = 0;
printo("\n uint8_t 0                     =", ui8tegg);
ui8tegg = 1;
printo("\n uint8_t 1                     =", ui8tegg);
ui8tegg = 255;
printo("\n uint8_t 255                   =", ui8tegg);

volatile uint16_t ui16tegg = 0;
printo("\n uint16_t 0                    =", ui16tegg);
ui16tegg = 1;
printo("\n uint16_t 1                    =", ui16tegg);
ui16tegg = 65535;
printo("\n uint16_t 65535                =", ui16tegg);

volatile uint32_t ui32tegghd = 0;
printo("\n uint32_t 0                    =", ui32tegghd);
ui32tegghd = 1;
printo("\n uint32_t 1                    =", ui32tegghd);
ui32tegghd = 4294967295;
printo("\n uint32_t 4294967295           =", ui32tegghd);

volatile uint64_t ui64temp4 = 0;
printo("\n uint64_t 0                    =", ui64temp4);
ui64temp4 = 18446744073709551614;
printo("\n uint64_t 18446744073709551614 =", ui64temp4);
ui64temp4 = 1;
printo("\n uint64_t 1                    =", ui64temp4);

**/



