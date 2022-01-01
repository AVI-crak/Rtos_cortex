/// древнее зло на новый лад
/// в процессе %0,6

/// + https://github.com/xboxfanj/math-neon
/// + gcc\math.h
/// + собсвенное мнение
#include "math_s.h"

float cos_f05(float value_rad,float sig);
float sin_f05(float value_rad,float sig);

#if (_ARM_CODE_V < 4)
float rint_f(float value)
{
    union f__raw raw;
    int32_t exx;
    raw.f_raw = value;
    exx = 150 - raw.order;
    if (exx > 0) {
        if (exx > 23) exx = 31;
        raw.i_raw >>=  exx;
        raw.i_raw <<= exx;
    };
    return  raw.f_raw;
};
#endif

#if (_ARM_CODE_V < 4)
float ceil_f(float value)
{
    union f__raw raw;
    int32_t exx;
    uint32_t err;
    raw.f_raw = value;
    exx = 150 - raw.order;
    if (exx > 0) {
        if (exx > 23) {
           if (raw.i_raw < 0) raw.i_raw = 0x80000000;
           else if (raw.i_raw)   raw.i_raw = 0x3f800000;
           else raw.i_raw = 0;
           return  raw.f_raw;
        };
        err = raw.i_raw >> exx;
        if (!raw.sign) {
            if (raw.i_raw - (err << exx)) err += 1;
        };
        raw.i_raw = err << exx;
    };
    return  raw.f_raw;
};
#endif

#if (_ARM_CODE_V < 4)
float floor_f(float value)
{
    union f__raw raw;
    int32_t exx, sig;
    uint32_t err;
    raw.f_raw = value;
    sig = raw.sign;
    exx = 150 - raw.order;
    if (exx > 0) {
        if (exx > 23) {
           err = raw.i_raw << 1; exx = 31;
           if ((err != 0) && (sig)) {
                   raw.i_raw = 0xBf800000;
                   return  raw.f_raw;
                   };
        };
        err = raw.i_raw >> exx;
        if (sig) {
            if (raw.i_raw - (err << exx)) err += 1;
        };
        raw.i_raw = err << exx;
    };
    return  raw.f_raw;
};
#endif

#if (_ARM_CODE_V < 4)
float round_f(float value)
{
    union f__raw raw;
    int32_t exx;
    uint32_t ex_mask;
    raw.f_raw = value;
    exx = raw.order - 126;
    if (exx < 0) {
        raw.i_raw &= 0x80000000;
    } else if (exx < 24) {
        ex_mask = 0x00ffffff >> exx;
        raw.i_raw += 0x00800000 >> exx;
        if (exx == 0) ex_mask >>= 1;
        raw.i_raw &= ~ex_mask;
    };
    return  raw.f_raw;
};
#endif

float fmod_f(float value, float divider)
{
    if (divider!=0.0f) {
        return( value - ( rint_f(value / divider) * divider));
    } else return 0.0f;
};

#if (_ARM_CODE_V < 4)
float abs_f(float value)
{
    union f__raw Ftemp;
    Ftemp.f_raw = value;
    Ftemp.sign = 0;
    return Ftemp.f_raw;
};
#endif

int32_t fbit_error(float a_compare, float b_compare)
{
    int32_t out;
    union f__raw  a_com;
    union f__raw  b_com;
    a_com.f_raw = a_compare;
    b_com.f_raw = b_compare;
    //  if (a_com.i_raw > b_com.i_raw) out = a_com.i_raw - b_com.i_raw;
    // else out = b_com.i_raw - a_com.i_raw;
    out = a_com.i_raw - b_com.i_raw;

    return out;
};

int64_t dbit_error(double a_compare, double b_compare)
{
    int64_t out;
    union d__raw  a_com;
    union d__raw  b_com;
    a_com.d_raw = a_compare;
    b_com.d_raw = b_compare;
    if (a_com.i64_raw > b_com.i64_raw) out = a_com.i64_raw - b_com.i64_raw;
    else out = b_com.i64_raw - a_com.i64_raw;
    return out;
};



const float __expf_rng[2] = {
    1.442695041f,
    0.693147180f,
};

const float __expf_lut[8] = {
    0.9999999916728642f,		//p0
    0.04165989275009526f, 	//p4
    0.5000006143673624f, 	//p2
    0.0014122663401803872f, 	//p6
    1.000000059694879f, 		//p1
    0.008336936973260111f, 	//p5
    0.16666570253074878f, 	//p3
    0.00019578093328483123f,	//p7
};

/// Raising the number E to the power of a value
/// Возведение числа E в степень value
float exp_f(float value)
{
    union f__raw Ftemp;
    if ( value > 88.7228317f) {
        Ftemp.i_raw = 0;
        Ftemp.order = 0xFF;
        return Ftemp.f_raw;
    };
    float tmp1, tmp2, tmp3, tmp4, tmp5;
    int8_t ord;


    ord = (int8_t) (value * __expf_rng[0]);
    value = value - ((float) ord) * __expf_rng[1];
    tmp1 = (__expf_lut[4] * value) + (__expf_lut[0]);
    tmp2 = (__expf_lut[6] * value) + (__expf_lut[2]);
    tmp3 = (__expf_lut[5] * value) + (__expf_lut[1]);
    tmp4 = (__expf_lut[7] * value) + (__expf_lut[3]);
    tmp5 = value * value;
    tmp1 = tmp1 + tmp2 * tmp5;
    tmp3 = tmp3 + tmp4 * tmp5;
    tmp5 = tmp5 * tmp5;
    Ftemp.f_raw = tmp1 + tmp3 * tmp5;
    Ftemp.order += ord;
    return Ftemp.f_raw;
}


const float __log10f_lut[9] = {
    -0.99697286229624f, 		//p0
    -1.07301643912502f, 		//p4
    -2.46980061535534f, 		//p2
    -0.07176870463131f, 		//p6
    2.247870219989470f, 		//p1
    0.366547581117400f, 		//p5
    1.991005185100089f, 		//p3
    0.006135635201050f,         //p7
    0.3010299957f,              //rng
};

const float __log_e_f_lut[9] = {
    -2.295614848256274, 	//p0
    -2.470711633419806, 	//p4
    -5.686926051100417, 	//p2
    -0.165253547131978, 	//p6
    +5.175912446351073, 	//p1
    +0.844006986174912, 	//p5
    +4.584458825456749, 	//p3
    +0.014127821926000,		//p7
    0.693147180f,           //rng
};

/// Logarithm value for base 2,10,e
/// Логарифм value по основанию 2,10,e
float log_f(float value, int_fast8_t base_2_10_e)
{
    float tmp1, tmp2, tmp3, tmp4, tmp5;
    int8_t ord, ord1;
    union f__raw Ftemp;
    const float *_lut;
    if (base_2_10_e != 10) _lut = __log_e_f_lut;
    else _lut = __log10f_lut;

    Ftemp.f_raw = value;
    ord = Ftemp.order;
    ord1 = ord;
    ord -= 127;
    ord1 -= ord;
    Ftemp.order = ord1;

    tmp5= Ftemp.f_raw * Ftemp.f_raw;
    tmp1 = (_lut[4] * Ftemp.f_raw) + (_lut[0]);
    tmp2 = (_lut[6] * Ftemp.f_raw) + (_lut[2]);
    tmp3 = (_lut[5] * Ftemp.f_raw) + (_lut[1]);
    tmp4 = (_lut[7] * Ftemp.f_raw) + (_lut[3]);
    tmp1 = tmp1 + tmp2 * tmp5;
    tmp3 = tmp3 + tmp4 * tmp5;
    tmp5 = tmp5 * tmp5;
    Ftemp.f_raw = tmp1 + tmp3 * tmp5;
    Ftemp.f_raw += ((float) ord) * _lut[8];
    if (base_2_10_e == 2) Ftemp.f_raw *= 1.442695040888965f;
    return Ftemp.f_raw;
}






/// exponentiation value^degree
/// Возведение в степень value^degree
float pow_f(float value,float degree)
{
    if(0.0f > value && fmod_f(degree, 1.0f) == 0.0f) {
        if(fmod_f(degree, 2.0f) == 0.0f)
            return (exp_f(log_f( -value, 'e') * degree));
        else return (-exp_f(log_f( -value, 'e') * degree));
    } else if(0.0f > value && fmod_f( degree, 1.0f) != 0.0f) {
        return 0.0f;
    } else {
        if(value != 0.0f || 0.0f >= degree)
            return (exp_f(log_f(value, 'e') * degree));
    };
    return 0.0f;
};


/// Extraction of the square root value
/// Извлечение квадратного корня value
#if (_ARM_CODE_V < 3)
float sqrt_f(float value)
#define magic_VRSQRTE  /// polynomial or magic_VRSQRTE
{
#ifdef magic_VRSQRTE
    float tmp1, tmp2;
    int32_t mas;
    union f__raw Ftemp;
    //fast invsqrt approx
    Ftemp.f_raw = value;
    Ftemp.i_raw = 0x5F3759DF - (Ftemp.i_raw >> 1);      //VRSQRTE
    tmp2 = value * Ftemp.f_raw;
    tmp1 = (3.0f - tmp2 * Ftemp.f_raw) * 0.5f;          //VRSQRTS
    Ftemp.f_raw = Ftemp.f_raw * tmp1;
    tmp2 = value * Ftemp.f_raw;
    tmp1 = (3.0f - tmp2 * Ftemp.f_raw) * 0.5f;
    Ftemp.f_raw = Ftemp.f_raw * tmp1;

    //fast inverse approx
    value = Ftemp.f_raw;
    mas = 0x3F800000 - (Ftemp.i_raw & 0x7F800000);
    Ftemp.i_raw = Ftemp.i_raw + mas;
    Ftemp.f_raw = 1.41176471f - 0.47058824f * Ftemp.f_raw;
    Ftemp.i_raw = Ftemp.i_raw + mas;
    tmp1 = 2.0f - Ftemp.f_raw * value;
    Ftemp.f_raw = Ftemp.f_raw * tmp1;
    tmp1 = 2.0f - Ftemp.f_raw * value;
    Ftemp.f_raw = Ftemp.f_raw * tmp1;

    return Ftemp.f_raw;
#else
    return (pow_f(value, 0.5f ));
#endif
}
#endif

float fact_f(float value)
{
    float ret = 1.0f;
    for (float i=1.0f; i <= value; i++) ret *= i;
    return ret;
}

/// Converting degrees to radians
/// Перевод градусов в радианы
float deg_rad(float value_deg)
{
    float rad;
    rad = abs_f(value_deg);
    if ( rad > 360.0f) rad = fmod_f(value_deg, 360.0f);
    else rad = value_deg;
    rad *= 1.11701071262f;
    rad *= 0.015625f;
    return rad;
};

/// Converting radians to degrees
/// Перевод  в радиан градусы
float rad_deg(float value_rad)
{
    float deg;
    deg = abs_f(value_rad);
    if ( deg >= Pi2) deg = fmod_f(value_rad, Pi2);
    else deg = value_rad;
    deg *= 57.2957801819f;
    return deg;
};



/// cos input is radian +-pi/4
float cos_f05(float value_rad,float sig)///60
{
    float rev, res;
    rev = value_rad * value_rad;
    res = rev * -2.48015876423e-5f;
    res += 0.0013888888061f;
    res *= rev; res += -0.0416666641831f;
    res *= rev; res += 0.5f;
    res *= rev; res = 1.0f - res;
    res *= sig;
    return res;
};

/// sin input is radian +-pi/4
float sin_f05(float value_rad,float sig)///64
{
    float rev, res;
    rev = value_rad * value_rad;
    res = rev * 2.75574029729e-06f;
    res += -0.000198412628379f;
    res *= rev; res += 0.00833333376795f;
    res *= rev; res += -0.166666656733f;
    res *= rev; res *= value_rad;
    res += value_rad; res *= sig;
    return res;
};

/// sin input is radian, output 1.0:-1.0, error 1L
float sin_f(float value_rad)///188
{
    float rev, res, inv;
    int32_t nix;
    if (value_rad < 0.0f) {rev = 0.0f - value_rad; inv = -1.0f;}
    else {rev = value_rad; inv = 1.0f;};
    do{
        nix = (int32_t) (rev * 1.27323949337f);
        if (nix > 7) { rev = fmod_f(rev, Pi2);};
    }while (nix > 7);
    switch (nix) {
    case 0: res = sin_f05(rev, inv); break;
    case 1: res = cos_f05((PI / 2.0f - rev), inv); break;
    case 2: res = cos_f05((PI / 2.0f - rev), inv); break;
    case 3: res = sin_f05(PI - rev, inv); break;
    case 4: res = sin_f05(PI - rev, inv); break;
    case 5: res = cos_f05(rev - (PI + Pi / 2.0f), -inv); break;
    case 6: res = cos_f05(rev - (PI + Pi / 2.0f), -inv); break;
    case 7: res = sin_f05(rev - Pi2, inv); break;
    default: break;};
    return res;
};

/// sin input is radian, output 1.0:-1.0, error 1L
float cos_f(float value_rad)/// 196
{
    float rev, res, inv;
    int32_t nix; inv = 1.0f;
    if (value_rad < 0.0f) rev = 0.0f - value_rad;
    else rev = value_rad;
    do {
        nix = (int32_t) (rev * 1.27323949337f);
        if (nix > 7) rev = fmod_f(rev, Pi2);
    } while (nix > 7);
    switch (nix) {
    case 0: res = cos_f05(rev, inv); break;
    case 1: res = sin_f05((PI / 2.0f - rev), inv); break;
    case 2: res = sin_f05((PI / 2.0f - rev), inv); break;
    case 3: res = cos_f05(PI - rev, -inv); break;
    case 4: res = cos_f05(PI - rev, -inv); break;
    case 5: res = sin_f05(rev - (PI + Pi / 2.0f), inv); break;
    case 6: res = sin_f05(rev - (PI + Pi / 2.0f), inv); break;
    case 7: res = cos_f05(rev - Pi2, inv); break;
    default: break;
    };
    return res;
};



double sin_d(double value_rad)
{
    double rep, rep_z, ret, rev, fac, fac_i;
    ret = value_rad;
    if (ret < (Pi_d / (-2.0))) ret += Pi_d * 2.0;
    if (ret > (Pi_d * 1.5)) ret -= Pi_d * 2.0;
    else if (ret > (Pi_d / 2.0)) ret = Pi_d - ret;
    rep = ret; rev = rep * rep;
    fac = 1.0d; fac_i = 2.0d;
    double exx[26]; int edd = 25;
    do
    {
        ret *= rev; fac *= fac_i++; fac *= -fac_i++;
        exx[edd] = ret / fac; rep_z = exx[edd]; edd--;
    }while ((rep_z != 0.0d)&& (edd != 0)); fac = 0.0d;
    do
    {
        edd++;
        fac += exx[edd];
    }while (edd != 25);
    rep += fac;
    return rep;
}


/// cos input is radian +-2pi, output 1.0:-1.0.
double cos_d(double value_rad)
{
    double rew;
    rew = value_rad + (Pi_d / 2.0);
    if (rew > (Pi_d * 2.0)) rew -= (Pi_d * 2.0);
    rew = sin_d(rew);
    return rew;
};

// error 1/5000
float sin__f(float value_rad)
{
    float ret, rev, res;
    ret = value_rad;
    ret *= (Pi/2.0f);
    if (ret < 0.0f) ret += 4.0f;
    if (ret >= 3.0f) ret -= 4.0f;
    else if (ret > 1.0f) ret -= 2.0f;
    rev = ret * ret;
    res = rev * 0.069696970284f - 0.639393925667f;
    res *= rev;
    res += 1.56969702244f;
    res *= ret;
    return res;
};






/// tan input is radian -pi/2:+pi/2, output min:max.
/// error (-+pi/2) 1L
float tan_f(float value_rad)
{
    float resin, recos, rev, inv;
    int32_t nix;
    rev = value_rad; inv = 1.0f;
    if( abs_f(rev) > (PI/2.0f)) {
        rev -= round_f(rev * 0.318309873343f) * PI;
    };
    nix = ((int32_t) floor_f(rev * 1.27323949337f)) + 2;
    switch (nix) {
    case 0: resin = cos_f05((PI/2.0f + rev), -inv);
            recos = sin_f05((PI/2.0f + rev), inv); break;
    case 1: resin = sin_f05(rev , inv);
            recos = cos_f05(rev , inv); break;
    case 2: resin = sin_f05(rev, inv );
            recos = cos_f05(rev , inv); break;
    case 3: resin = cos_f05((PI/2.0f - rev), inv);
            recos = sin_f05((PI/2.0f - rev), inv); break;
    default: break;
    };
    rev = resin / recos;
    return rev;

};







/*
one =  1.0000000000e+00f, // 0x3F800000  [10]
huge =  1.000e+30f,
asin
pio2_hi = 1.57079637050628662109375f, // 0x3fc90fdb  [11]
pio2_lo = -4.37113900018624283e-8f,  // 0xb33bbd2f ~ 0xb33bbd2e  [12]
pio4_hi = 0.785398185253143310546875f, //0x3f490fdb [13]
acos
pio2_hic =  1.5707962513e+00, // 0x3fc90fda [14]
pio2_loc =  7.5497894159e-08, // 0x33a22168 [15]

	// coefficient for R(x^2)
pS0 =  1.6666667163e-01f, // 0x3e2aaaab  [5]
pS1 = -3.2556581497e-01f, // 0xbea6b090  [4]
pS2 =  2.0121252537e-01f, // 0x3e4e0aa8  [3]
pS3 = -4.0055535734e-02f, // 0xbd241146  [2]
pS4 =  7.9153501429e-04f, // 0x3a4f7f04  [1]
pS5 =  3.4793309169e-05f, // 0x3811ef08  [0]
qS1 = -2.4033949375e+00f, // 0xc019d139  [9]
qS2 =  2.0209457874e+00f, // 0x4001572d  [8]
qS3 = -6.8828397989e-01f, // 0xbf303361  [7]
qS4 =  7.7038154006e-02f; // 0x3d9dc62e  [6]
*/

/// pS5-pS0, qS4-qS1, one, pio2_hi, pio2_lo, pio4_hi, pio2_hic, pio2_loc
const uint32_t table_const_asin_acos[16]= {
    0x3811ef08,0x3a4f7f04,0xbd241146,0x3e4e0aa8,0xbea6b090,0x3e2aaaab,0x3d9dc62e,0xbf303361,0x4001572d,0xc019d139,
    0x3F800000,0x3fc90fdb, 0xb33bbd2f, 0x3f490fdb, 0x3fc90fda, 0x33a22168,
};

/// in -1.0f:+1.0f, out 0:2pi
float asin_f(float value)
{
    float rep,wer,pif,qif,cem,rem,sem;
    int_fast8_t sign, nix;
    float* tab;
    tab = (float*) table_const_asin_acos;
    union f__raw asi;
    asi.f_raw = value;
    sign = asi.sign;
    asi.sign = 0;
    if(asi.i_raw == 0x3F800000) {           /// asin(1)=+-pi/2 with inexact
        asi.f_raw = PI/2.0f;
        asi.sign = sign;
        return asi.f_raw;
    } else if(asi.i_raw > 0x3F800000) {  /// |value|>= 1
        asi.i_raw |= 0x01FF << 22;
        return asi.f_raw;               /// asin(|value|>1) is NaN
    } else if (asi.i_raw < 0x3f000000) { /// |value|<0.5
        if(asi.i_raw < 0x32000000) {    /// if |value| < 7.4505806E-9
            return value;               /// return value with inexact if value!=0
        } else {
            rep = value*value;
            nix = 0; pif = 0; qif = 0;
            do {
                pif = (pif + tab[nix++]) * rep;
            } while (nix != 6);
            do {
                qif = (qif + tab[nix++]) * rep;
            } while (nix != 10);
            qif += tab[nix];
            wer = pif/qif;
            return value+value*wer;
        };
    };                                  /// 1> |value|>= 0.5
    wer = tab[10] - asi.f_raw;
    rep = wer * 0.5f; nix = 0; pif = 0; qif = 0;
    do {
        pif = (pif + tab[nix++]) * rep;
    } while (nix != 6);
    do {
        qif = (qif + tab[nix++]) * rep;
    } while (nix != 10);
    qif += tab[nix];
    sem = sqrt_f(rep);
    if(asi.i_raw >= 0x3F79999A) {           /// if |value| > 0.975
        wer = pif / qif;
        rep = tab[11] - ((float)2.0 * (sem + sem * wer) - tab[12]);
    } else {
        asi.f_raw = sem; asi.i_raw &= 0xfffff000;
        cem  = (rep - asi.f_raw * asi.f_raw) / (sem + asi.f_raw);
        rem  = pif / qif;
        pif  = (float)2.0f * sem * rem - (tab[12] - (float)2.0f * cem);
        qif  = tab[13] - (float)2.0f * asi.f_raw;
        rep  = tab[13] - (pif - qif);
    };
    if(sign != 0) return -rep;
    else return rep;
}

/// in -1.0f:+1.0f, out 0:2pi
float acos_f(float value)
{
    float rep,pif,qif,wer,sem,sif,cif;
    int_fast8_t sign, nix;
    float* tab;
    tab = (float*) table_const_asin_acos;
    union f__raw aco;
    aco.f_raw = value;
    sign = aco.sign;
    aco.sign = 0;
    if(aco.i_raw == 0x3f800000) {           /// |value|==1
        if(sign == 0) return 0.0;	    /// acos(1) = 0
        else return PI;	            /// acos(-1)= pi
    } else if(aco.i_raw > 0x3f800000) { /// |value| >= 1
        aco.i_raw = 0x01FF << 22;
        return aco.f_raw;               /// acos(|value|>1) is NaN
    };
    if(aco.i_raw<0x3f000000) {              /// |value| < 0.5
        if(aco.i_raw<=0x23000000) {     /// if|value|<2**-57
            aco.f_raw = PI/2.0f;
            aco.sign = sign;
            return aco.f_raw;
        };
        rep = value*value; nix = 0; pif = 0; qif = 0;
        do {
            pif = (pif + tab[nix++]) * rep;
        } while (nix != 6);
        do {
            qif = (qif + tab[nix++]) * rep;
        } while (nix != 10);
        qif += tab[nix]; wer = pif/qif;
        return tab[14] - (value - (tab[15]-value * wer));
    } else if (sign != 0) {              /// value < -0.5
        rep = (tab[10] + value) * 0.5f;
        nix = 0; pif = 0; qif = 0;
        do {
            pif = (pif + tab[nix++]) * rep;
        } while (nix != 6);
        do {
            qif = (qif + tab[nix++]) * rep;
        } while (nix != 10);
        qif += tab[nix]; wer = pif/qif;
        sif = sqrt_f(rep);
        sem = wer*sif - tab[15];
        return (PI - 2.0 * (sif + sem));
    } else {                            /// value > 0.5
        rep = (tab[10] - value ) * 0.5f;
        sif = sqrt_f(rep); aco.f_raw = sif;
        aco.i_raw &= 0xfffff000;
        cif  = (rep-aco.f_raw * aco.f_raw) / (sif + aco.f_raw);
        nix = 0; pif = 0; qif = 0;
        do {
            pif = (pif + tab[nix++]) * rep;
        } while (nix != 6);
        do {
            qif = (qif + tab[nix++]) * rep;
        } while (nix != 10);
        qif += tab[nix]; wer = pif/qif;
        sem = wer * sif + cif;
        return (2.0f * (aco.f_raw + sem));
    };
}




/// atan input (sin/cos), output +pi:-pi, error 2L
float atan_f(float value)
{
    float ret, rex, rez;
    float singl, errors;
    if(value < 0.0f) singl = -1.0f;
    else singl = 1.0f;
    ret = singl * value;
    if (ret > 16782358.0f) {
        return (1.57079637051f * singl);
    } else if(ret > 3.28f) {
        errors = 1.57079637051f;
        ret = -1.0f / ret;
    } else if(ret > 0.8f) {
        errors = 0.982793807983f;
        rex = 1.5f * ret + 1.0f;
        ret -= 1.5f;
        ret /= rex;
    } else if(ret > 0.26f) {
        errors = 0.463647603989f;
        rex = 2.0f * ret - 1.0f;
        ret += 2.0f;
        ret = rex / ret;
    } else if(ret > 7.09616335826e-15f) {
        errors = 0.0f;
    } else return value;
    rex = ret * ret; rez = ret * 0.0769230797887f; // 1/13
    rez *= rex; rez += ret * -0.0909090936184f;  // -1/11
    rez *= rex; rez += ret * 0.111111111939;  // 1/9
    rez *= rex; rez += ret * -0.142857149243f;  // -1/7
    rez *= rex; rez += ret * 0.20000000298f;   // 1/5
    rez *= rex; rez += ret * -0.333333343267f;  // -1/3
    rez *= rex; ret += rez; ret += errors;
    ret *= singl;
    return ret;
};


/// atan2 input +-1.0, output 0:2pi, error 3L
float atan2_f(float value_sin, float value_cos)
{
    float rep;
    rep = atan_f( value_sin / value_cos);
    if (value_cos < 0.0f) rep += PI;
    else if (value_sin < 0.0f) rep += Pi2;
    return rep;
};

