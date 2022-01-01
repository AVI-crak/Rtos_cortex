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
2225073860,1458224404,955661947,626302614,4104536801,2689949240,1762885134,1155324400,757153401,496208051,
3251949089,2131197354,1396701497,915342294,599878726,3931365217,2576459509,1688508503,1106580933,725208881,
475272893,3114748422,2041281526,1337774260,876723741,574569670,3765499791,2467757943,1617269845,1059893965,
694612109,455220993,2983336292,1955159273,1281333182,839734514,550328410,3606632274,2363642526,1549036766,
1015176736,665306226,436015089,2857468478,1872670543,1227273367,804305874,527109899,3454467423,2263919770,
1483682460,972346138,637236766,4176194860,2736911063,1793662035,1175494352,770371977,504870979,3308722450,
2168404344,1421085472,931322574,610351562,4000000000,2621440000,1717986920,1125899908,737869764,483570327,
3169126500,2076918744,1361129469,892029808,584600655,3831238852,2510840695,1645504557,1078397867,706738825,
463168357,3035420146,1989292946,1303703026,854394816,559936185,3669597787,2404907604,1576080248,1032899952,
676921314,443627153,2907354899,1905364106,1248699421,818347653,536312317,3514776402,2303443863,1509584969,
989321605,648361809,4249103944,2784692759,1824976248,1196016434,783821329,513685148,3366486977,2206260907,
1445895147,947581844,621007236,4069833027,2667205774,1747979977,1145556157,750751684,492012623,3224453926,
2113178124,1384892417,907603094,594806765,3898125605,2554675596,1674232198,1097224813,719077255,};



const char txt_NaN[] = "NaN";
char* floating_char(uint32_t massa, uint32_t of10raw, int32_t feeze, int32_t order10, char* txt);


__attribute__ ((optimize("-Os"))) char* hex_char(char* tail_txt, uint32_t value)//44
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


__attribute__ ((optimize("-Os")))char* float_char(char* text, float value )//164
{
    union float_raw    ftemp;
    ftemp.f_raw = value;
    char *tex, *ter; tex = text;
    if (ftemp.sign) { tex++; *text = '-';}else *text = 0;
    ftemp.sign = 0;
    if (ftemp.u_raw == 0) ter = "0";
    else if (ftemp.u_raw < 0x7f800000)
    {
        uint32_t order, massa, of10raw, cis;
        int32_t feeze, order10;
        order = ftemp.order;
        massa = ftemp.massa;
        cis = (order + 896) >> 4;
        feeze = order & 0x000F;
        if (feeze > 7) {cis++; feeze -= 16;};
        order10 = data_otner[cis];
        of10raw = data_of10raw[cis];
        if (order == 0)
        {
            massa <<= 9;
            if ((massa & 0x80000000) == 0){
            do{
                if (of10raw < 858993459UL) {order10--; of10raw *= 5UL;}
                else of10raw >>= 1;
                massa <<= 1;
            }while ((massa & 0x80000000) == 0);};
        }else
        {
            massa <<= 8;
            massa |= (uint32_t) 1 << 31;
        };
        text = floating_char(massa, of10raw, feeze, order10, text);
        return text;
    }else if (ftemp.u_raw > 0x7f800000) ter = "NaN";
    else ter = "inf";
    uint8_t tmp;
    do{tmp = *ter++; *tex++ = tmp;}while(tmp);
    return text;
};
/*
__attribute__ ((optimize("-Os"))) char* double_char(char* text, double value )//204
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
    if (feeze > 7) {cis++; feeze -= 16;};
    order10 = data_otner[cis];
    of10raw = data_of10raw[cis];

    if (order == 0)
    {
        dtemp.u64_raw <<= 12;
        if (dtemp.sign == 0){
        do{
            dtemp.u64_raw <<= 1;
            if (of10raw < 858993459UL)
                {
                    order10--;
                    of10raw *= 5;
                }else of10raw >>= 1;
        }while (dtemp.sign == 0);};
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
*/

__attribute__ ((optimize("-Os"))) char* double_char(char* text, double value )//204
{
    union double_raw    dtemp;
    dtemp.d_raw = value;
    char *tex, *ter; tex = text;
    if (dtemp.sign) { tex++; *text = '-';}else *text = 0;
    dtemp.sign = 0;
    if (dtemp.u64_raw == 0) ter = "0";
    else if (dtemp.u64_raw < 0x7FF0000000000000)
    {
        uint32_t order, massa, of10raw, cis;
        int32_t feeze, order10;
        order = dtemp.order;
        cis = order >> 4;
        feeze = order & 0x000F;
        if (feeze > 7) {cis++; feeze -= 16;};
        order10 = data_otner[cis];
        of10raw = data_of10raw[cis];

        if (order == 0)
        {
            dtemp.u64_raw <<= 12;
            if (dtemp.sign == 0){
            do{
                dtemp.u64_raw <<= 1;
                if (of10raw < 858993459UL)
                    {
                        order10--;
                        of10raw *= 5;
                    }else of10raw >>= 1;
            }while (dtemp.sign == 0);};
            massa = dtemp.u_raw[1];
        }else
        {
            dtemp.u64_raw <<= 11;
            massa = dtemp.u_raw[1];
            massa |= (uint32_t) 1 << 31;
        };
        text = floating_char( massa, of10raw, feeze,  order10, text);
        return text;
    }else if (dtemp.u64_raw > 0x7FF0000000000000) ter = "NaN";
    else ter = "inf";
    uint8_t tmp;
    do{tmp = *ter++; *tex++ = tmp;}while(tmp);
    return text;
};

__attribute__((optimize("-Os"))) char* floating_char(uint32_t massa, uint32_t of10raw, int32_t feeze, int32_t order10, char* txt)//204
{
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
    mas_s = u32_char (mas_f, massa);
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
    if (cis5 == 2)printo("const int16_t data_otner[129] = {\n");
    if (cis5 == 1)printo("const uint32_t data_of10raw[129] = {\n");
    cis4 = 0;
    for(cis2 = 0; cis2 !=129; cis2++)
    {
        cis = cis2 <<4;
        of10raw = 8589934592000000000ULL;
        ofreze = 0;
        if (cis > 1023){
            while ( (cis--) != 1023){
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
        else if (cis < 1023){
            while ( (cis++) != 1023){
                if (of10raw < 3689348814741910323ULL)
                    {
                       ofreze -= 1; of10raw = (uint64_t)of10raw * 5ULL;
                    }else of10raw >>= 1;};};
        cis3 = (uint32_t) (of10raw >> 32);
        if((cis3 & 2047) > 1500) cis3 += 2;
        else if((cis3 & 2047) > 750) cis3 += 1;
        if (cis5 == 2){cis4++; printo(ofreze,","); if(cis4 > 19){cis4 = 0; printo("\n");};};
        if (cis5 == 1){cis4++; printo( cis3,","); if(cis4 > 9){cis4 = 0; printo("\n");};};
    }; printo("};\n\n");
    cis5--;};
};
*/




///-------------------------
/** test
printo("\f 0.0f                = ", 0.0f );
printo("\n 1.0f                = ", 1.0f," \t\t -1uL = ", 9.99999940395e-1f );
printo("\n -1.23456788063f     = ", -1.23456788063f," \t -1uL = ", -1.23456776142f );
printo("\n -1.5f               = ", -1.5f," \t\t -1uL = ", 1.49999988079f );
printo("\n 2e+2f               = ", 2e+2f," \t\t -1uL = ", 1.99999984741e+2f );
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
printo("\n 2.4000000469e-24f   = ", 2.4000000469e-24f," \t -1uL = ", 2.39999984968e-24f );
printo("\n 2.50000004885e-25f  = ", 2.50000004885e-25f," \t -1uL = ", 2.49999980233e-25f );
printo("\n 2.60000018639e-26f  = ", 2.60000018639e-26f," \t -1uL = ", 2.59999987824e-26f );
printo("\n 2.70000009282e-27f  = ", 2.70000009282e-27f," \t -1uL = ", 2.69999990023e-27f );
printo("\n 2.80000000888e-28f  = ", 2.80000000888e-28f," \t -1uL = ", 2.79999976814e-28f );
printo("\n 2.9000000092e-29f   = ", 2.9000000092e-29f," \t\t -1uL = ", 2.89999970827e-29f );
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

struct
{
    double dou;
    float  fl[2];
    uint64_t u64;
    int32_t i32;
    int8_t i8;
    char txt[20];
}test;
test.dou = 5.5555555555e55;
test.fl[0] = 1.00000003137e-22f;
test.fl[1] = 9.99999968266e-23f;
test.i32 = -1234567890;
test.u64 = 1234567891234567890;
test.i8 = -100;
int32_t tmp = 0;
do{test.txt[tmp] = "hello Omsk"[tmp];}while("hello Omsk"[tmp++]!=0);
printo("\n test.dou = ", test.dou);
printo("\n test.fl[0] = ", test.fl[0]);
printo("\n test.fl[1] = ", test.fl[1]);
printo("\n test.i32 = ", test.i32);
printo("\n test.u64 = ", test.u64);
printo("\n test.i8 = ", test.i8);
printo("\n test.txt = ", test.txt);

int64_t i64temp1 = -9223372036854775808;
printo("\n int64_t -9223372036854775808  =", i64temp1);
i64temp1 = -1;
printo("\n int64_t -1                    =", i64temp1);
i64temp1 = 9223372036854775807;
printo("\n int64_t 9223372036854775807   =", i64temp1);
i64temp1 = 2;
printo("\n int64_t 2                     =", i64temp1);
i64temp1 = 0;
printo("\n int64_t 0                     =", i64temp1);

int32_t i32temp2;
i32temp2 = 1;
printo("\n int32_t 1                     =", i32temp2);
i32temp2 = -2147483648;
printo("\n int32_t -2147483648           =", i32temp2);
i32temp2 = -1;
printo("\n int32_t -1                    =", i32temp2);
i32temp2 = 0;
printo("\n int32_t 0                     =", i32temp2);


int16_t i16tmp = -32768;
printo("\n int16_t -32768                =", i16tmp);
i16tmp = -1;
printo("\n int16_t -1                    =", i16tmp);
i16tmp = 0;
printo("\n int16_t 0                     =", i16tmp);
i16tmp = 1;
printo("\n int16_t 1                     =", i16tmp);
i16tmp = 32767;
printo("\n int16_t 32767                 =", i16tmp);

uint8_t ui8tegg = 0;
printo("\n uint8_t 0                     =", ui8tegg);
ui8tegg = 1;
printo("\n uint8_t 1                     =", ui8tegg);
ui8tegg = 255;
printo("\n uint8_t 255                   =", ui8tegg);

uint16_t ui16tegg = 0;
printo("\n uint16_t 0                    =", ui16tegg);
ui16tegg = 1;
printo("\n uint16_t 1                    =", ui16tegg);
ui16tegg = 65535;
printo("\n uint16_t 65535                =", ui16tegg);

uint32_t ui32tegghd = 0;
printo("\n uint32_t 0                    =", ui32tegghd);
ui32tegghd = 1;
printo("\n uint32_t 1                    =", ui32tegghd);
ui32tegghd = 4294967295;
printo("\n uint32_t 4294967295           =", ui32tegghd);

uint64_t ui64temp4 = 0;
printo("\n uint64_t 0                    =", ui64temp4);
ui64temp4 = 18446744073709551614;
printo("\n uint64_t 18446744073709551614 =", ui64temp4);
ui64temp4 = 1;
printo("\n uint64_t 1                    =", ui64temp4);

**/

/*
__attribute__ ((optimize("-Og")))uint64_t divu64_10(uint32_t* residue, uint64_t value)//72
{
    uint32_t res, t10, magh, magl;
    union divrev rev;
    union divrev rew;
    rev.w64 = value;
    magh = 3435973837UL; magl = 429496730UL; t10 = 10;
    rew.w32[1] = ((uint64_t)rev.w32[1] * magh >> 35);
    rew.w32[0] = 0;
    rev.w64 = (uint64_t)rev.w64 - rew.w64 * t10;
    res = ((uint64_t)rev.w64 >> 16);
    res = ((uint64_t)res * magl >> 32);
    rew.w32[0] = res << 16;
    rev.w64 = (uint64_t)rev.w64 - rew.w32[0] * t10;
    res = ((uint64_t)rev.w32[0] * magl >> 32);
    rew.w32[0] += res;
    res = rev.w32[0] - res * t10;
    *residue = res;
    return rew.w64;
};
*/



