/// sPrint.c
/// печать без указания типа параметра, в разнобой
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

const int16_t data_otner[129] = {
-308,-304,-299,-294,-289,-284,-280,-275,-270,-265,-260,-255,-251,-246,-241,-236,-231,-227,-222,-217,-212,
-207,-202,-198,-193,-188,-183,-178,-174,-169,-164,-159,-154,-150,-145,-140,-135,-130,-125,-121,-116,-111,
-106,-101,-97,-92,-87,-82,-77,-72,-68,-63,-58,-53,-48,-44,-39,-34,-29,-24,-19,-15,-10,-5,0,5,9,14,19,24,
29,34,38,43,48,53,58,62,67,72,77,82,86,91,96,101,106,111,115,120,125,130,135,139,144,149,154,159,164,168,
173,178,183,188,192,197,202,207,212,217,221,226,231,236,241,245,250,255,260,265,270,274,279,284,289,294,
298,303,308};

const uint32_t data_of10raw[129] = {
222507385,1458224403,955661945,626302612,410453680,268994923,1762885132,1155324400,757153399,496208051,
325194908,213119735,1396701497,915342293,599878725,393136521,257645950,1688508503,1106580932,725208879,
475272891,311474842,204128152,1337774260,876723739,574569669,376549978,246775794,1617269844,1059893965,
694612109,455220991,298333629,1955159272,1281333180,839734513,550328410,360663227,236364252,1549036765,
1015176734,665306225,436015087,285746847,1872670541,1227273366,804305873,527109897,345446742,226391976,
1483682460,972346137,637236764,417619485,273691106,1793662034,1175494350,770371977,504870979,330872245,
216840434,1421085471,931322574,610351562,400000000,262144000,1717986918,1125899906,737869762,483570327,
316912650,207691874,1361129467,892029807,584600654,383123885,251084069,1645504557,1078397866,706738825,
463168356,303542014,1989292945,1303703024,854394814,559936185,366959778,240490760,1576080247,1032899951,
676921312,443627151,290735489,1905364105,1248699420,818347651,536312317,351477640,230344386,1509584969,
989321605,648361807,424910394,278469275,1824976247,1196016433,783821329,513685146,336648697,220626090,
1445895146,947581843,621007236,406983302,266720577,1747979975,1145556156,750751682,492012622,322445392,
211317812,1384892415,907603093,594806763,389812560,255467559,1674232198,1097224813,719077253};

const char txt_NaN[] = "NaN";
char* floating_char(uint32_t massa, uint32_t of10raw, int16_t feeze, int16_t order10, char* txt);

char* nex_char(uint32_t value, char* text)  /// 68  байт
{
    int32_t tmp;
    volatile char* tex; tex = text;
    *(tex--) = 0;
    do
    {
        tmp = value & 0x0F;
        if (tmp > 9 ) *(tex--) = tmp + 55;
            else *(tex--) = tmp + '0';
        value >>= 4;
    }while (value);
    *(tex--) = 'x';
    *tex = '0';
    return tex;
};

char * i32_char(int32_t value, char* text)  /// 108  байт
{
    if (value < 0)
    {
        value = 0 - value;
        text = u32_char(value, text);
        *(--text) = '-';
    }else text = u32_char(value, text);
    return text;
};

char* u32_char (uint32_t value, char* text)  /// 52  байт
{
    *text = 0;
    do{
        *(--text) = (value % 10) + '0';
        value /= 10;
    }while (value != 0);
    return text;
};

char* i64_char(int64_t value, char* text)
{
    if (value < 0)
    {
        value = 0 - value;
        text = u64_char(value, text);
        *(--text) = '-';
    }else text = u64_char(value, text);
    return text;
};

char* u64_char(uint64_t value, char* text) /// 108  байт
{
    if ((value >> 32) == 0)
    {
        text = u32_char(value, text);
    }else
    {
        *text = 0;
        do{
            *(--text) = (value % 10) + '0';
            value /= 10;
        }while (value != 0);
    };
    return text;
};

void compress_char(char* tex_in, char* tex_out) /// 60  байт
{
    int32_t ovr;
    ovr =  (tex_out - tex_in) - OUT_TXT_SIZE_FLOATING;
    if (ovr > 0)
    {
        ovr +=2;
        if(ovr <= 9)
        {
            *(tex_in + (OUT_TXT_SIZE_FLOATING - 2)) = 'E';
            *(tex_in + (OUT_TXT_SIZE_FLOATING - 1)) = ovr + '0';
        }else
        {
            ovr++;
            *(tex_in + (OUT_TXT_SIZE_FLOATING - 3)) = 'E';
            *(tex_in + (OUT_TXT_SIZE_FLOATING - 2)) = (ovr / 10) + '0';
            *(tex_in + (OUT_TXT_SIZE_FLOATING - 1)) = (ovr % 10) + '0';
        };
        *(tex_in + OUT_TXT_SIZE_FLOATING) = 0;
    };
};

char* float_char(float value, char* text) /// 200 байт
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
            of10raw >>= 1; ftemp.u_raw <<= 1;
            if (of10raw < 200000000 )
                {
                    order10 -= 1;
                    of10raw *=10;
                };
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

char* double_char(double value, char* text)    /// 252 байт
{
    union double_raw    dtemp;
    dtemp.d_raw = value;
    int_fast8_t sign;
    uint16_t order;
    uint32_t massa, of10raw;
    int16_t feeze, order10, cis;
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
            of10raw >>= 1; dtemp.u64_raw <<= 1;
            if (of10raw < 200000000 )
                {
                    order10 -= 1;
                    of10raw *=10;
                };
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

char* floating_char(uint32_t massa, uint32_t of10raw, int16_t feeze, int16_t order10, char* txt) /// 304 байт
{
    if (feeze > 7)
    {
        while ( (feeze++) != 16)
        {
            of10raw >>= 1;
            if (of10raw < 200000000 )
            {
                 order10 -= 1; of10raw *=10;
            };
        };
    }else
    {
        while ( (feeze--) != 0)
        {
            of10raw <<= 1;
            if (of10raw > 2000000000 )
                {
                    order10 += 1; of10raw /=10;
                };
        };
    };
    union double_raw of10;
    of10.u64_raw = (uint64_t) massa * of10raw;
    char *mas_s, *mas_f, *ord_s, *ord_f; int32_t tmp, tff;
    mas_f = txt + 12;
    mas_s = u32_char (of10.u_raw[1], mas_f);
    of10.u_raw[1] = of10.u_raw[0] / 429496729;
    *(mas_f++) = of10.u_raw[1] + '0';
    of10.u_raw[0] = of10.u_raw[0] - (of10.u_raw[1] * 429496729);
    of10.u_raw[1] = of10.u_raw[0] / 42949672;
    *(mas_f++) = of10.u_raw[1] + '0';
    of10.u_raw[0] = of10.u_raw[0] - (of10.u_raw[1] * 42949672);
    of10.u_raw[1] = of10.u_raw[0] / 4294967;
    *(mas_f++) = of10.u_raw[1] + '0';
    *mas_f = 0;

    tmp = mas_f - mas_s; tmp -= 12;
    order10 += tmp;
    if ((-12 <= order10)&&(order10 < 15))
    {
        tmp = (order10 + 12) / 3; tmp *= 3; tmp -= 12;
        tff = order10 - tmp; order10 = tmp;
    } else tff = 0;
    if (order10 != 0)
    {
        ord_f = txt + 23;
        ord_s = i32_char(order10, ord_f);
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



/*
///  858993459200000000
/// 8589934592000000000
 void tabl_grabl(void)
{
    volatile uint32_t cis;
    volatile uint64_t of10raw;
    volatile int16_t ofreze;
    volatile int32_t cis2;
    volatile uint32_t cis3;
    volatile int32_t cis4;
    cis4 = -1;
    for(cis2 = 0; cis2 !=129; cis2++)
    {
        cis = cis2 <<4;
        of10raw = 858993459200000000;
        ofreze = 0;
    if (cis > 1023)
    {
        while ( (cis--) != 1023){
            of10raw <<= 1;
            if (of10raw > 8589934592000000000 )
                {
                    ofreze += 1;
                    of10raw /=10;
                };
        };
    } else if (cis < 1023)
    {
        while ( (cis++) != 1023){
            of10raw >>= 1;
            if (of10raw < 858993459200000000 )
                {     //
                    ofreze -= 1;
                    of10raw *=10;
                };
        };
    };
    cis3 = (uint32_t) (of10raw >> 32);
    cis4++;
    if (cis4 > 9) {cis4 = 0; printo("\n");}; printo( cis3,",");
   // if (cis4 > 20) {cis4 = 0; printo("\n");}; printo(ofreze,",");
  //
    };

os_Delay_ms(20000);

};

*/
///-------------------------
/*
printo ("\f");
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


printo("\n 4567891234567891234 =  ", 4567891234567891234ULL);
printo("\n -4567891234567891234 = ", -4567891234567891234LL);
printo("\n -456789123456789123 =  ", -456789123456789123LL);
printo("\n -45678912345678912 =   ", -45678912345678912LL);
printo("\n -4567891234567891 =    ", -4567891234567891LL);
printo("\n -456789123456789 =     ", -456789123456789LL);
printo("\n -45678912345678 =      ", -45678912345678LL);
printo("\n -4567891234567 =       ", -4567891234567LL);
printo("\n -456789123456 =        ", -456789123456LL);
printo("\n -45678912345 =         ", -45678912345LL);
printo("\n -4567891234 =          ", -4567891234LL);
printo("\n -456789123 =           ", -456789123L);
printo("\n -45678912 =            ", -45678912L);
printo("\n -4567891 =             ", -4567891L);
printo("\n -456789 =              ", -456789L);
printo("\n -45678 =               ", -45678L);
printo("\n -4567 =                ", -4567L);
printo("\n -456 =                 ", -456L);
printo("\n -45 =                  ", -45L);
printo("\n -4 =                   ", -4L);



printo("\n 4567891234567891234 =  ", 4567891234567891234ULL);
printo("\n 456789123456789123 =   ", 456789123456789123ULL);
printo("\n 45678912345678912 =    ", 45678912345678912ULL);
printo("\n 4567891234567891 =     ", 4567891234567891ULL);
printo("\n 456789123456789 =      ", 456789123456789ULL);
printo("\n 45678912345678 =       ", 45678912345678ULL);
printo("\n 4567891234567 =        ", 4567891234567ULL);
printo("\n 456789123456 =         ", 456789123456ULL);
printo("\n 45678912345 =          ", 45678912345ULL);
printo("\n 4567891234 =           ", 4567891234ULL);
printo("\n 456789123 =            ", 456789123UL);
printo("\n 45678912 =             ", 45678912UL);
printo("\n 4567891 =              ", 4567891UL);
printo("\n 456789 =               ", 456789UL);
printo("\n 45678 =                ", 4567UL);
printo("\n 456 =                  ", 456UL);
printo("\n 45 =                   ", 45UL);
printo("\n 4 =                    ", 4UL);

char ntst_texx[] = "helou words";
printo(ntst_texx, "\n");
printo("test tekst \n");
volatile uint16_t *teat_adress = 55;
printo("\n uint16_t *teat_adress = ", teat_adress);
printo("\n uint16_t &teat_adress = ", &teat_adress);
volatile int64_t i64temp1 = -9223372036854775808;
printo("\n int64_t -9223372036854775808 = ", i64temp1);
i64temp1 = -1;
printo("\n int64_t -1                   = ", i64temp1);
i64temp1 = 9223372036854775807;
printo("\n int64_t 9223372036854775807 = ", i64temp1);
i64temp1 = 2;
printo("\n int64_t 2                   = ", i64temp1);
i64temp1 = 0;
printo("\n int64_t 0                   = ", i64temp1);

volatile int32_t i32temp2 = -1;
printo("\n int32_t -1           = ", i32temp2);
i32temp2 = 1;
printo("\n int32_t 1            = ", i32temp2);
i32temp2 = -2147483648;
printo("\n int32_t -2147483648  = ", i32temp2);
i32temp2 = -1;
printo("\n int32_t i32temp2     = ", i32temp2);
i32temp2 = 0;
printo("\n int32_t 0            = ", i32temp2);


volatile int16_t ui8temp3 = -32768;
printo("\n int16_t -32768   = ", ui8temp3);
ui8temp3 = -1;
printo("\n int16_t -1       = ", ui8temp3);
ui8temp3 = 0;
printo("\n int16_t 0        = ", ui8temp3);
ui8temp3 = 1;
printo("\n int16_t 1        = ", ui8temp3);
ui8temp3 = 32767;
printo("\n int16_t 32767    = ", ui8temp3);

volatile uint8_t ui8tegg = 0;
printo("\n uint8_t 0    = ", ui8tegg);
ui8tegg = 1;
printo("\n uint8_t 1    = ", ui8tegg);
ui8tegg = 255;
printo("\n uint8_t 255  = ", ui8tegg);

volatile uint16_t ui16tegg = 0;
printo("\n uint16_t 0    = ", ui16tegg);
ui16tegg = 1;
printo("\n uint16_t 1    = ", ui16tegg);
ui16tegg = 65535;
printo("\n uint16_t 65535= ", ui16tegg);

volatile uint32_t ui32tegghd = 0;
printo("\n uint32_t 0            = ", ui32tegghd);
ui32tegghd = 1;
printo("\n uint32_t 1            = ", ui32tegghd);
ui32tegghd = 4294967295;
printo("\n uint32_t 4294967295   = ", ui32tegghd);

volatile uint64_t ui64temp4 = 0;
printo("\n uint64_t 0                     = ", ui64temp4);
ui64temp4 = 18446744073709551614;
printo("\n uint64_t 18446744073709551614  = ", ui64temp4);
ui64temp4 = 1;
printo("\n uint64_t 1                     = ", ui64temp4);
*/

