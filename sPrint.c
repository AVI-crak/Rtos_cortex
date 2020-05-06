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

const int16_t data_otner[65] = {
-308,-299,-289,-280,-270,-260,-251,-241,-231,-222,
-212,-202,-193,-183,-174,-164,-154,-145,-135,-125,
-116,-106,-97,-87,-77,-68,-58,-48,-39,-29,
-19,-10,0,9,19,29,38,48,58,67,
77,86,96,106,115,125,135,144,154,164,
173,183,192,202,212,221,231,241,250,260,
270,279,289,298,308};

const uint32_t data_of10raw[65] = {
111253692,477830972,205226840,881442566,378576699,162597454,698350748,299939362,128822975,553290466,
237636445,102064076,438361869,188274989,808634922,347306054,149166814,640666590,275164205,118182126,
507588367,218007543,936335270,402152936,172723371,741841230,318618382,136845553,587747175,252435489,
108420217,465661287,200000000,858993459,368934881,158456325,680564733,292300327,125542034,539198933,
231584178,994646472,427197407,183479889,788040123,338460656,145367744,624349710,268156158,115172193,
494660802,212455197,912488123,391910664,168324348,722947573,310503618,133360288,572778078,246006311,
105658906,453801546,194906280,837116099,359538626};

const char txt_p_Infinity[] = "+Infinity";
const char txt_m_Infinity[] = "-Infinity";
const char txt_NaN[] = "NaN";

char float_text[OUT_TXT_SIZE_FLOATING + 1];

void floating_char(uint32_t massa, uint32_t of10raw, int16_t feeze, int16_t order10, char* out_txt);
void entire_char (char* char_in, char* char_out, int8_t t_ord);

char* nex_char (uint32_t value)
{
    uint32_t tmp = 10;
    uint32_t tmp2 = value;
    float_text[tmp--] = 0;
    do
    {
        if ((tmp2 & 0x0000000F) > 9 )float_text[tmp--] = (tmp2 & 0x0000000F) + 0x37;
            else float_text[tmp--] = (tmp2 & 0x0000000F) + 0x30;
        tmp2 >>= 4;
    }while (tmp2);
    float_text[tmp--] = 'x';
    float_text[tmp] = '0';
    return &float_text[tmp];
}

char * i32_char (int32_t value)  // 112
{
    if (value < 0)
    {
        float_text[0] = '-';
        value = 0 - value;
    }else float_text[0] = '0';
    return  ui32_char (value);
};


char* ui32_char (uint32_t value)
{
    if (value == 0)
    {
        float_text[0] = '0';
        float_text[1] = 0;
    }else
    {
        int8_t t_ord;
        t_ord = 20;
        char text_massa[20];
        while (value != 0)
        {
            text_massa[--t_ord] = value % 10 + '0';
            value /= 10;
        };
        entire_char ( text_massa, float_text,  t_ord);
    };
    return  &float_text[0];
}


char * i64_char (int64_t value)  //180
{
    if (value < 0)
    {
        float_text[0] = '-';
        value = 0 - value;
    }else float_text[0] = '0';
    return  ui64_char (value);

};


char * ui64_char (uint64_t value)
{
    union double_raw  temp;
    temp.u64_raw = value;
    if (temp.u_raw[1] == 0) return  ui32_char (temp.u_raw[0]);
    else
    {
        int8_t t_ord;
        t_ord = 20;
        char text_massa[20];
        while (temp.u64_raw != 0)
        {
            text_massa[--t_ord] = temp.u64_raw % 10 + '0';
            temp.u64_raw /= 10;
        };
        entire_char ( text_massa, float_text,  t_ord);
    };
    return  &float_text[0];
};


void entire_char (char* char_in, char* char_out, int8_t t_ord)   //116
{
    int_fast8_t exxx;
    if (char_out[0] == '-') exxx = 0; else exxx = -1;
    while (( exxx < (OUT_TXT_SIZE_FLOATING - 1)) && (t_ord != 20))
    {
        char_out[++exxx] = char_in[t_ord++];
    };
    t_ord = 20 - t_ord;
    if (t_ord > 0)
    {
        t_ord +=2;
        if (t_ord < 10)
        {
            char_out[exxx -1] = 'E';
            char_out[exxx -0] = t_ord + '0';
        }else
        {
            t_ord++;
            char_out[exxx -2] = 'E';
            char_out[exxx -1] = (t_ord / 10) + '0';
            char_out[exxx -0] = (t_ord % 10) + '0';
        };
    };
    char_out[++exxx] = 0;
};



char* float_char (float value)    /// 268 байт,
{
    union float_raw    ftemp;
    ftemp.f_raw = value;
    int_fast8_t sign;
    uint16_t order;
    uint32_t massa, of10raw;
    int16_t feeze, order10, cis;

    sign = ftemp.sign; order = ftemp.order;

    feeze = 0;
    if (sign)  float_text[feeze++] = '-';
        else float_text[feeze++] = '+';
    if (order == 255)
    {
        if (ftemp.massa == 0)
        {
            if (sign) return (char*)&txt_m_Infinity[0];
                else return (char*)&txt_p_Infinity[0];
        }else return (char*)&txt_NaN[0];
    }else if ((order == 0) && (ftemp.massa == 0))
    {
        float_text[feeze++] = '0';
        float_text[feeze++] = ',';
        float_text[feeze++] = '0';
        float_text[feeze++] = 0;
        return &float_text[0];
    }

    cis = (order + 896) >> 5;
    feeze = order & 0x001F;
    if (feeze > 15) cis++;
    order10 = data_otner[cis];
    of10raw = data_of10raw[cis];

    if (order == 0)
    {
        ftemp.u_raw <<= 9;
        while (ftemp.sign == 0)
        {
            of10raw >>= 1; ftemp.u_raw <<= 1;
            if (of10raw < 100000000 )
                {       //999999999
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

floating_char( massa, of10raw, feeze, order10, float_text);

    return &float_text[0];

}



char* double_char (double value)    /// 296 байт
{
    union double_raw    dtemp;
    dtemp.d_raw = value;
    int_fast8_t sign;
    uint16_t order;
    uint32_t massa, of10raw;
    int16_t feeze, order10, cis;


    sign = dtemp.sign; order = dtemp.order;

    feeze = 0;
    if (sign)  float_text[feeze++] = '-';
        else float_text[feeze++] = '+';
    if (order == 2047)
    {
        if (dtemp.massa == 0)
        {
            if (sign) return (char*) &txt_m_Infinity[0];
                else return (char*) &txt_p_Infinity[0];
        }else return (char*) &txt_NaN[0];
    }else if ((order == 0) && (dtemp.massa == 0))
    {
        float_text[feeze++] = '0';
        float_text[feeze++] = ',';
        float_text[feeze++] = '0';
        float_text[feeze++] = 0;
        return &float_text[0];
    }

    cis = order >> 5;
    feeze = order & 0x001F;
    if (feeze > 15) cis++;
    order10 = data_otner[cis];
    of10raw = data_of10raw[cis];

    if (order == 0)
    {
        dtemp.u64_raw <<= 12;
        while (dtemp.sign == 0)
        {
            of10raw >>= 1; dtemp.u64_raw <<= 1;
            if (of10raw < 100000000 )
                {       //999999999
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

floating_char( massa, of10raw, feeze, order10, float_text);

    return &float_text[0];

}

void floating_char(uint32_t massa, uint32_t of10raw, int16_t feeze, int16_t order10, char* out_txt) /// 528 байт
{
    int_fast8_t sig, t_mas, t_ord, t_ex, verge;
    int_fast16_t  ofree, out_n;
    char text_massa[10];char text_order[5];
    if (feeze <= 15)
    {
        while ( (feeze--) != 0)
        {
            of10raw <<= 1;
            if (of10raw > 999999999 )
                {
                    order10 += 1;
                    of10raw /=10;
                };
        };
    }else if (feeze > 15)
    {
        while ( (feeze++) != 32)
        {
            of10raw >>= 1;
            if (of10raw < 100000000 )
                {
                    order10 -= 1;
                    of10raw *=10;
                };
        };
    };

    of10raw = (uint32_t) (((uint64_t) massa * of10raw + 999999999 ) >> 31);

    t_mas = 10;
    do
    {
        text_massa[--t_mas] = of10raw % 10 + '0';
        of10raw /= 10;
    }while (of10raw);
    for (t_ex = 9; text_massa[t_ex] == '0'; t_ex--);
    order10 += 1 - t_mas;
    if (order10 < 0)
    {
        ofree = 0 - order10;
        sig = '-';
    }else
    {
        ofree = order10;
        sig = '+';
    };
    t_ord = 5;
    while (ofree)
    {
        text_order[--t_ord] = ofree % 10 + '0';
        ofree /= 10;
    };
    text_order[--t_ord] = sig;
    text_order[--t_ord] = 'e';
    if (out_txt[0] == '-')
    {
        verge = OUT_TXT_SIZE_FLOATING - 1;
        out_n = 1;
    }else out_n = 0;

    if (t_ord != 5) verge -= (5 - t_ord);
    if (verge > 7) verge = 7;
    ofree = OUT_TXT_SIZE_FLOATING - (5 - t_ord);
    out_txt[out_n++] = text_massa[t_mas++];
    if ((order10 <= verge) && (order10 >=0))
    {
        while (order10 != 0)
        {
            out_txt[out_n++] = text_massa[t_mas++];
            order10--;
        };
        out_txt[out_n++] = ',';
        do
        {
            out_txt[out_n++] = text_massa[t_mas++];
        }while ((t_mas <= t_ex) && (out_n < OUT_TXT_SIZE_FLOATING));
    }else
    {
        out_txt[out_n++] = ',';
        do
        {
            out_txt[out_n++] = text_massa[t_mas++];
        }while ((out_n < ofree) && (t_mas <= t_ex));
        do
        {
            out_txt[out_n++] = text_order[t_ord++];
        }while (t_ord != 5);
    };
    out_txt[out_n] = 0;
  //  return &out_txt[0];
}

/*
///const int16_t data_otner[65]
///const uint32_t data_of10raw[65]
 void tabl_grabl(void)
{
    volatile uint32_t cis;
    volatile uint64_t of10raw;
    volatile int16_t ofreze;
    volatile int32_t cis2;
    volatile uint32_t cis3;
    for(cis2 = 0; cis2 !=65; cis2++)
    {
        cis = cis2 <<5;
        of10raw = 100000000000000000;
        ofreze = 0;
    if (cis > 1023)
    {
        do{
            of10raw <<= 1;
            if (of10raw > 999999999000000000 )
                {
                    ofreze += 1;
                    of10raw /=10;
                };
        }while ( (--cis) != 1023);
    } else if (cis < 1023)
    {
        do{
            of10raw >>= 1;
            if (of10raw < 100000000000000000 )
                {
                    ofreze -= 1;
                    of10raw *=10;
                };
        }while ( (++cis) != 1023);
    };
    cis3 = of10raw/1000000000;
    printo(",",ofreze);
    //printo(",", cis3);
    };
};
*/



/*
printo("\n 255.552244e+1f = ", 255.5522e+1f);
printo("\n 1.9523415e-30f = ", 1.9523415e-30f);
printo("\n -8.654000e+12f = ", -8.6540e+12f);
printo("\n -2.5500000000f = ", -2.55f);
printo("\n -1.0000000000f = ", -1.0f);
printo("\n 4.85400550e-6f = ", 4.8540055e-6f);
printo("\n 123456789.000f = ", 123456789.0f);
printo("\n -1.2345679e-7f = ", -1.23456789e-7f);
printo("\n 35.0000000e+3f = ", 35.0e+3f);
printo("\n 12.0000000000f = ", 12.0f);
printo("\n -1.366000e-36f = ", -1.3660e-36f);

printo("\n 2.000000e-308d = ", 2.000000e-308d);
printo("\n 555664422.550d = ", 555664422.550d);
printo("\n 1.23456789123d = ", 1.23456789123d);
printo("\n 3.21654987e-3d = ", 3.21654987e-3d);


printo("\n 1.74554255000e-288d = ", 1.74554255000e-288d);
printo("\n 3.243552454547e+66d = ", 3.243552454547e+66d);
printo("\n 7.345624524111e+24d = ", 7.345624524111e+24d);
printo("\n 1.2345678912345678d = ", 1.2345678912345678d);


printo("\n 4567891234567891234 =  ", 4567891234567891234);
printo("\n -4567891234567891234 = ", -4567891234567891234);
printo("\n -456789123456789123 =  ", -456789123456789123);
printo("\n -45678912345678912 =   ", -45678912345678912);
printo("\n -4567891234567891 =    ", -4567891234567891);
printo("\n -456789123456789 =     ", -456789123456789);
printo("\n -45678912345678 =      ", -45678912345678);
printo("\n -4567891234567 =       ", -4567891234567);
printo("\n -456789123456 =        ", -456789123456);
printo("\n -45678912345 =         ", -45678912345);
printo("\n -4567891234 =          ", -4567891234);
printo("\n -456789123 =           ", -456789123);
printo("\n -45678912 =            ", -45678912);
printo("\n -4567891 =             ", -4567891);
printo("\n -456789 =              ", -456789);
printo("\n -45678 =               ", -45678);
printo("\n -4567 =                ", -4567);
printo("\n -456 =                 ", -456);
printo("\n -45 =                  ", -45);
printo("\n -4 =                   ", -4);



printo("\n 4567891234567891234 =  ", 4567891234567891234);
printo("\n 456789123456789123 =   ", 456789123456789123);
printo("\n 45678912345678912 =    ", 45678912345678912);
printo("\n 4567891234567891 =     ", 4567891234567891);
printo("\n 456789123456789 =      ", 456789123456789);
printo("\n 45678912345678 =       ", 45678912345678);
printo("\n 4567891234567 =        ", 4567891234567);
printo("\n 456789123456 =         ", 456789123456);
printo("\n 45678912345 =          ", 45678912345);
printo("\n 4567891234 =           ", 4567891234);
printo("\n 456789123 =            ", 456789123);
printo("\n 45678912 =             ", 45678912);
printo("\n 4567891 =              ", 4567891);
printo("\n 456789 =               ", 456789);
printo("\n 45678 =                ", 4567);
printo("\n 456 =                  ", 456);
printo("\n 45 =                   ", 45);
printo("\n 4 =                    ", 4);

printo("\n uint32_t temp = ", temp);

volatile int64_t i64temp1 = 100;
printo("\n uint16_t post_xz = ", print_ram.post_xz);
printo("\n int64_t i64temp1 = ", i64temp1);
i64temp1 = 0;
printo("\n int64_t i64temp1 = ", i64temp1);
i64temp1 = -9205352140778758016;
printo("\n int64_t i64temp1 = ", i64temp1);
i64temp1 = -1;
printo("\n int64_t i64temp1 = ", i64temp1);


volatile int32_t i32temp2 = 33;
printo("\n int32_t i32temp2 = ", i32temp2);
i32temp2 = 0;
printo("\n int32_t i32temp2 = ", i32temp2);
i32temp2 = -221144;
printo("\n int32_t i32temp2 = ", i32temp2);
i32temp2 = -1;
printo("\n int32_t i32temp2 = ", i32temp2);


volatile uint8_t ui8temp3 = 33;
printo("\n uint8_t ui8temp3 33= ", ui8temp3);
ui8temp3 = 55;
printo("\n uint8_t ui8temp3 55= ", ui8temp3);
ui8temp3 = 240;
printo("\n uint8_t ui8temp3 240 = ", ui8temp3);
ui8temp3 = 0;
printo("\n uint8_t ui8temp3 0= ", ui8temp3);



volatile uint64_t ui64temp4 = 18446744073709551615;
printo("\n int64_t ui64temp4 18446744073709551615= ", ui64temp4);
ui64temp4 = 18446744073709551614;
printo("\n int64_t ui64temp4 18446744073709551614= ", ui64temp4);
ui64temp4 = 184;
printo("\n int64_t ui64temp4 = ", ui64temp4);
*/
