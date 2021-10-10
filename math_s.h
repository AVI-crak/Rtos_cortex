
/// древнее зло на новый лад
/// в процессе %0,5

/// + https://github.com/xboxfanj/math-neon
/// + gcc\math.h
/// + собсвенное мнение

#ifndef MATH_s
#define MATH_s

#ifdef PI
#undef  PI
#undef  Pi
#endif
#include <stdint.h>
#define PI      3.14159274101f   /// 0x40490fdb
#define Pi      3.14159250259f   /// 0x40490fda
#define Pi_d    3.1415926535897932384626433832795d
#define Pi_c    3.14159297943115234375f     /// 0x40490fdc
#define Pi2     6.28318548203f     /// 0x40c90fdb
//#define PIerror1_5  2.264936966867026058025658130645751953125E-7
///original pi 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679



union f__raw
{
    struct
    {
        uint32_t massa  :23;
        uint32_t order  :8;
        uint32_t sign   :1;
    };
    int32_t     i_raw;
    float       f_raw;
};

union d__raw
{
    struct
    {
        uint64_t massa  :52;
        uint64_t order  :11;
        uint64_t sign   :1;
    };
    uint32_t    u_raw[2];
    int64_t     i64_raw;
    double      d_raw;
};


/// rounding up to the smallest integer that is greater than or equal to the |argument|
/// округление до наименьшего целого, которое больше или равно |аргументу|
#ifdef __CM7_REV
float ceil_f(float value)
{
    float rep; int32_t tmp;
    tmp = (int32_t)value;
    if (tmp > 0) asm volatile ("vrintp.f32 %0,%1" : "=t"(rep) : "t"(value));
        else asm volatile ("vrintm.f32 %0,%1" : "=t"(rep) : "t"(value));
    return rep;
}
#else
float ceil_f(float value)
{
    union f__raw Ftemp;
    Ftemp.f_raw = value;
    uint32_t err = 0;
    int_fast8_t ord = (int_fast8_t)Ftemp.order - 127;
    if (ord >= 0)
    {
        ord = 23 - ord;
        if(ord > 0)
        {
            err = Ftemp.i_raw;
            Ftemp.i_raw = ((Ftemp.i_raw >>ord)<<ord);
            err -= Ftemp.i_raw;
            if (err != 0)
            {
                if (Ftemp.sign == 0) return (Ftemp.f_raw + 1.0f);
                    else return (Ftemp.f_raw - 1.0f);
            };
        }else return value;
    }else return 0.0f;
}
#endif


/// drop a fraction below zero
/// отбросить дробное значение ниже нуля
#ifdef __CM7_REV
static inline float floor_f(float value)
{
    float rep;
    asm volatile ("vrintz.f32 %0,%1" : "=t"(rep) : "t"(value));
    return rep;
}
#else
float floor_f(float value)
{
    union f__raw Ftemp;
    Ftemp.f_raw = value;
    int_fast8_t ord = (int_fast8_t)Ftemp.order - 127;
    if (ord < 0) return 0.0f;
    else
    {
        ord = 23 - ord;
        if(ord > 0)
        {
            Ftemp.i_raw = ((Ftemp.i_raw >>ord)<<ord);
        };
    };
    return Ftemp.f_raw;
}
#endif



/// round up to the nearest whole
/// округлить до ближайшего целого
#ifdef __CM7_REV
static inline float round_f(float value)
{
    float rep;
    asm volatile ("vrinta.f32 %0,%1" : "=t"(rep) : "t"(value));
    return rep;
}
#else
float round_f(float value)
{
    union f__raw Ftemp;
    Ftemp.f_raw = value;
    uint32_t err = 0;
    int_fast8_t ord = (int_fast8_t)Ftemp.order - 127;
    if (ord >= 0)
    {
        ord = 23 - ord;
        if(ord > 0)
        {
            err = Ftemp.i_raw;
            Ftemp.i_raw = ((Ftemp.i_raw >>ord)<<ord);
            err -= Ftemp.i_raw;
            err *= 2; err >>= ord;
            if (err != 0)
            {
                if (Ftemp.sign == 0) return (Ftemp.f_raw + 1.0f);
                    else return (Ftemp.f_raw - 1.0f);
            }else return Ftemp.f_raw;
        }else return value;
    }else return 0.0f;
}
#endif


/// return fractional remainder of division
/// вернуть дробный остаток деления
float fmod_f(float value, float divider)
{
   if (divider!=0.0f)
   {
       return( value - ( floor_f(value / divider) * divider));
   } else return 0.0f;
};


/// return |value|
/// сбросить знак
#if defined (__CM7_REV) || defined (__CM4_REV)
static inline float abs_f(float value)
{
    float rep;
    asm volatile ("vabs.f32 %0, %1" : "=t"(rep) : "t"(value));
    return rep;
}
#else
float abs_f(float value)
{
    union f__raw Ftemp;
    Ftemp.f_raw = value;
    Ftemp.sign = 0;
    return Ftemp.f_raw;
};
#endif

/// ошибка в младших битах
int32_t fbit_error(float a_compare, float b_compare)
{
    int32_t out;
    union f__raw  a_com;
    union f__raw  b_com;
    a_com.f_raw = a_compare;
    b_com.f_raw = b_compare;
    if (a_com.i_raw > b_com.i_raw) out = a_com.i_raw - b_com.i_raw;
    else out = b_com.i_raw - a_com.i_raw;
 // out = a_com.i_raw - b_com.i_raw;

    return out;
};

/// ошибка в младших битах
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
    if ( value > 88.7228317f) {Ftemp.i_raw = 0; Ftemp.order = 0xFF; return Ftemp.f_raw;};
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
	if (base_2_10_e != 10) _lut = __log_e_f_lut; else _lut = __log10f_lut;

	Ftemp.f_raw = value;
	ord = Ftemp.order; ord1 = ord;
	ord -= 127; ord1 -= ord;
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



/// returns fractional remainder
/// возвращает дробный остаток
static inline float mod_f(float value)
{
   return(value - floor_f(value));
}


/// exponentiation value^degree
/// Возведение в степень value^degree
float pow_f(float value,float degree)
{
    if(0 > value && fmod_f(degree, 1) == 0)
    {
        if(fmod_f(degree, 2) == 0)
            return (exp_f(log_f( -value, 'e') * degree));
        else return (-exp_f(log_f( -value, 'e') * degree));
    }else if(0 > value && fmod_f( degree, 1) != 0)
    {
      return 0.0f;
    }else
    {
        if(value != 0 || 0 >= degree)
            return (exp_f(log_f(value, 'e') * degree));
    };return 0.0;
};


/// Extraction of the square root value
/// Извлечение квадратного корня value
#if defined (__CM7_REV) || defined (__CM4_REV)
static inline float sqrt_f(float value)
{
    float rep;
    asm volatile ("vsqrt.f32 %0,%1" : "=t"(rep) : "t"(value));
    return rep;
}
#else
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
	rad /=64.0f;
    return rad;
}




/// sin input is radian +-2pi, output 1.0:-1.0.
/// min |x|=2.62879913375e-23 error 2L
float sin_f(float value_rad)
{
    float ret, rev, res;
//  if  (abs_f(value_rad) < 2.62879913375e-23f ) return value_rad;
    ret = value_rad;
    if (ret < (PI/(-2.0f))) ret += Pi2;
    if (ret > (Pi+ PI/2.0f)) ret -= Pi2;
    else if (ret > (PI/2.0f)) ret = PI - ret;
    rev = ret * ret;
    res = rev * -2.50516549727e-08f;
    res += 2.75573984254e-06f;
    res *= rev; res += -0.000198412570171f;
    res *= rev; res += 0.00833333469927f;
    res *= rev; res += -0.166666656733f;
    res *= rev; res *= ret; res += ret;
    return res;
};



/// cos input is radian -2pi:+2pi, output 1.0:-1.0.
float cos_f(float value_rad)
{
    float rew;
    rew = value_rad + (PI/2.0f);
    if (rew > Pi2) rew -= Pi2;
    return sin_f(rew);
};


/// sin input is radian +-2pi, output 1.0:-1.0.
/// min |x|=1.45534809702e-162 error 2L
double sin_d(double value_rad)
{
    double ret, rev, res;
    ret = value_rad;
    if (ret < (Pi_d / (-2.0))) ret += Pi_d * 2.0;
    if (ret > (Pi_d * 1.5)) ret -= Pi_d * 2.0;
    else if (ret > (Pi_d / 2.0)) ret = Pi_d - ret;
    rev = ret * ret;
    res = rev * 1.9572941063391261230847574373505e-20; // +1/21!
    res += -8.2206352466243297169559812368723e-18; // -1/19!
    res *= rev; res += 2.8114572543455207631989455830103e-15; // +1/17
    res *= rev; res += -7.6471637318198164759011319857881e-13; // -1/15!
    res *= rev; res += 1.6059043836821614599392377170155e-10; // +1/13!
    res *= rev; res += -2.5052108385441718775052108385442e-8; // -1/11!
    res *= rev; res += 2.7557319223985890652557319223986e-6; // +1/9!
    res *= rev; res += -1.984126984126984126984126984127e-4; // -1/7!
    res *= rev; res += 8.3333333333333333333333333333333e-3; // +1/5!
    res *= rev; res += -0.16666666666666666666666666666667; // -1/3!
    res *= rev; res *= ret; res += ret;
    return res;
};

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
    register float ret, rev, res;
    ret = value_rad; ret *= (Pi/2.0f);
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

/// tab = 1.0f; s = 2.0f;
/// (tab *= s++; tab *= s++;) x(n) = -1.0f/tab;
const uint32_t table_const_factorial[14] =
{
0xBE2AAAAB, 0x3C088889, 0xB9500D01, 0x3638EF1D, 0xB2D7322B, 0x2F309231, 0xAB573FA0, 0x274A963C, 0xA317A4DB, 0x1EB8DC78,
0x9A3B0DA0, 0x159F9E66, 0x90E8D58D, 0x0C12CFCB,
};

/// error 0,0%, 8~14 steps(8-10-12-14)
/// tan input is radian 0:+2pi, output min:max.
float tan_f(float value_rad)
{
    float rep, rep_c, ret, ret_c, rev, rev_c;
    if (value_rad < 0) value_rad += Pi2;
    if (value_rad >= (PI+ Pi/2.0f)) rep = value_rad - Pi2;
        else if (value_rad > Pi/2.0f) rep = PI - value_rad;
            else rep = value_rad;
        printo( "  si=", rep );// monitor_print("\n");
    if (abs_f(rep) >= (Pi/2.0f)) return 1.0e+36;
    if (value_rad >= PI ) rep_c = (PI+ Pi/2.0f) - value_rad;
        else rep_c = value_rad - Pi/2.0f;
    rep_c *=-1.0f; ret = rep; rev = rep * rep;
    ret_c = rep_c; rev_c = rep_c * rep_c;
    int32_t nex = 0;
    float* tab; tab = (float*) table_const_factorial;
    do
    {
        ret *= rev; ret_c *= rev_c;
        rep += ret * tab[nex];
        rep_c += ret_c * tab[nex++];
    }while (nex != 14); rep /= rep_c;
    return rep;
}

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
const uint32_t table_const_asin_acos[16]={
0x3811ef08,0x3a4f7f04,0xbd241146,0x3e4e0aa8,0xbea6b090,0x3e2aaaab,0x3d9dc62e,0xbf303361,0x4001572d,0xc019d139,
0x3F800000,0x3fc90fdb, 0xb33bbd2f, 0x3f490fdb, 0x3fc90fda, 0x33a22168,
};

/// in -1.0f:+1.0f, out 0:2pi
float asin_f(float value)
{
	float rep,wer,pif,qif,cem,rem,sem;
	int_fast8_t sign, nix;
	float* tab; tab = (float*) table_const_asin_acos;
	union f__raw asi;
	asi.f_raw = value; sign = asi.sign;
	asi.sign = 0;
	if(asi.i_raw == 0x3F800000)             /// asin(1)=+-pi/2 with inexact
	    {
            asi.f_raw = PI/2.0f;
            asi.sign = sign;
            return asi.f_raw;
        }else if(asi.i_raw > 0x3F800000)    /// |value|>= 1
        {
            asi.i_raw |= 0x01FF << 22;
            return asi.f_raw;               /// asin(|value|>1) is NaN
        }else if (asi.i_raw < 0x3f000000)   /// |value|<0.5
        {
            if(asi.i_raw < 0x32000000)      /// if |value| < 7.4505806E-9
            {
                return value;               /// return value with inexact if value!=0
            }else
            {
                rep = value*value; nix = 0; pif = 0; qif = 0;
                do{ pif = (pif + tab[nix++]) * rep; }while (nix != 6);
                do{ qif = (qif + tab[nix++]) * rep; }while (nix != 10);
                qif += tab[nix];
                wer = pif/qif;
                return value+value*wer;
            };
        };                                  /// 1> |value|>= 0.5
	wer = tab[10] - asi.f_raw;
	rep = wer * 0.5f; nix = 0; pif = 0; qif = 0;
    do{ pif = (pif + tab[nix++]) * rep; }while (nix != 6);
    do{ qif = (qif + tab[nix++]) * rep; }while (nix != 10);
    qif += tab[nix];
	sem = sqrt_f(rep);
	if(asi.i_raw >= 0x3F79999A)             /// if |value| > 0.975
	    {
            wer = pif / qif;
            rep = tab[11] - ((float)2.0 * (sem + sem * wer) - tab[12]);
        }else
        {
            asi.f_raw = sem;
            asi.i_raw &= 0xfffff000;
            cem  = (rep - asi.f_raw * asi.f_raw) / (sem + asi.f_raw);
            rem  = pif / qif;
            pif  = (float)2.0f * sem * rem - (tab[12] - (float)2.0f * cem);
            qif  = tab[13] - (float)2.0f * asi.f_raw;
            rep  = tab[13] - (pif - qif);
        };
    if(sign != 0) return -rep; else return rep;
}

/// in -1.0f:+1.0f, out 0:2pi
float acos_f(float value)
{
	float rep,pif,qif,wer,sem,sif,cif;
    int_fast8_t sign, nix;
	float* tab; tab = (float*) table_const_asin_acos;
	union f__raw aco;
	aco.f_raw = value; sign = aco.sign;
	aco.sign = 0;
	if(aco.i_raw == 0x3f800000)             /// |value|==1
	    {
            if(sign == 0) return 0.0;	    /// acos(1) = 0
                else return PI;	            /// acos(-1)= pi
        } else if(aco.i_raw > 0x3f800000)   /// |value| >= 1
        {
            aco.i_raw = 0x01FF << 22;
            return aco.f_raw;               /// acos(|value|>1) is NaN
        };
	if(aco.i_raw<0x3f000000)                /// |value| < 0.5
	    {
            if(aco.i_raw<=0x23000000)       /// if|value|<2**-57
            {
                aco.f_raw = PI/2.0f; aco.sign = sign;
                return aco.f_raw;
            };
            rep = value*value; nix = 0; pif = 0; qif = 0;
            do{ pif = (pif + tab[nix++]) * rep; }while (nix != 6);
            do{ qif = (qif + tab[nix++]) * rep; }while (nix != 10);
            qif += tab[nix]; wer = pif/qif;
            return tab[14] - (value - (tab[15]-value * wer));
        } else if (sign != 0)                /// value < -0.5
        {
            rep = (tab[10] + value) * 0.5f; nix = 0; pif = 0; qif = 0;
            do{ pif = (pif + tab[nix++]) * rep; }while (nix != 6);
            do{ qif = (qif + tab[nix++]) * rep; }while (nix != 10);
            qif += tab[nix]; wer = pif/qif;
            sif = sqrt_f(rep);
            sem = wer*sif - tab[15];
            return (PI - 2.0 * (sif + sem));
        } else                              /// value > 0.5
        {
            rep = (tab[10] - value ) * 0.5f;
            sif = sqrt_f(rep);
            aco.f_raw = sif;
            aco.i_raw &= 0xfffff000;
            cif  = (rep-aco.f_raw * aco.f_raw) / (sif + aco.f_raw);
            nix = 0; pif = 0; qif = 0;
            do{ pif = (pif + tab[nix++]) * rep; }while (nix != 6);
            do{ qif = (qif + tab[nix++]) * rep; }while (nix != 10);
            qif += tab[nix]; wer = pif/qif;
            sem = wer * sif + cif;
            return (2.0f * (aco.f_raw + sem));
        };
}




/// atan input (sin/cos), output pi:-pi.
/// max |x|=16782358 error 2L
float atan_f(float value)
{
    float ret, rex, rez;
    float singl, errors;
    if(value < 0.0f) singl = -1.0f; else singl = 1.0f;
    ret = singl * value;
    if (ret > 16782358.0f) return (1.57079637051f * singl);
    else if(ret > 3.28f){errors = 1.57079637051f; ret = -1.0f / ret;}
    else if(ret > 0.8f){errors = 0.982793807983f; rex = 1.5f * ret + 1.0f; ret -= 1.5f; ret /= rex;}
    else if(ret > 0.26f){errors = 0.463647633791f; rex = 2.0f * ret - 1.0f; ret += 2.0f; ret = rex / ret;}
    else if(ret > 7.09616335826e-15f) errors = 0.0f; else return value;
    rex = ret * ret;
    rez = ret * 0.0588235296309f;  //1/17
    rez *= rex; rez += ret * -0.0666666701436f; // -1/15
    rez *= rex; rez += ret * 0.0769230797887f; // 1/13
    rez *= rex; rez += ret * -0.0909090936184f;  // -1/11
    rez *= rex; rez += ret * 0.111111111939;  // 1/9
    rez *= rex; rez += ret * -0.142857149243f;  // -1/7
    rez *= rex; rez += ret * 0.20000000298f;   // 1/5
    rez *= rex; rez += ret * -0.333333343267f;  // -1/3
    rez *= rex; ret += rez;
    ret += errors; ret *= singl;
    return ret;
};



/// atan2 input +-1.0, output 0:2pi
/// error 3L
float atan2_f(float value_sin, float value_cos)
{
    float rep;
    rep = atan_f( value_sin / value_cos);
    if (value_cos < 0.0f) rep += PI;
    else if (value_sin < 0.0f) rep += Pi2;
    return rep;
};



/*
////////////////////////////// Trig Functions //////////////////////////////
#ifdef PI_DIV_BY_TWO_INV
#undef PI_DIV_BY_TWO_INV
#endif
#define PI_DIV_BY_TWO_INV 0.63661977236758134
#ifdef PI_DIV_BY_TWO
#undef PI_DIV_BY_TWO
#endif
#define PI_DIV_BY_TWO   1.5707963267948966  //pi/2
#ifdef TWOBYPI
#undef TWOBYPI
#endif
#define TWOBYPI          0.6366197723675813   //2/pi






float const pas[3] = {0.49559947, -4.6145309, 5.6036290};
float const qas[3] = {1.0000000,  -5.5484666, 5.6036290};

float ASIN_COS(float x, uint8_t n)
{
   float y, res, r, y2;
   int_fast8_t s;
   #ifdef _ERRNO
   if(x <-1 || x > 1)
   {
      errno=EDOM;
   }
   #endif
   s = 0;
   y = x;

   if (x < 0)
   {
      s = 1;
      y = -y;
   }

   if (y > 0.5)
   {
      y = sqrt((1.0 - y)*0.5);
      n += 2;
   }

   y2=y*y;

   res = pas[0]*y2 + pas[1];
   res = res*y2 + pas[2];

   r = qas[0]*y2 + qas[1];
   r = r*y2 + qas[2];

   res = y*res/r;

   if (n & 2)     // |x| > 0.5
      res = PI_DIV_BY_TWO - 2.0*res;
   if (s)
      res = -res;
   if (n & 1)           // take arccos
      res = PI_DIV_BY_TWO - res;

   return(res);
}



////////////////////////////////////////////////////////////////////////////
//   float asin(float x)
////////////////////////////////////////////////////////////////////////////
// Description : returns the arcsine value of the value x.
// Date : N/A
//
float asin(float x)
{
   float r;

   r = ASIN_COS(x, 0);
   return(r);
}


////////////////////////////////////////////////////////////////////////////
//   float acos(float x)
////////////////////////////////////////////////////////////////////////////
// Description : returns the arccosine value of the value x.
// Date : N/A
//
float acos(float x)
{
   float r;

   r = ASIN_COS(x, 1);
   return(r);
}


float const pat[4] = {0.17630401, 5.6710795, 22.376096, 19.818457};
float const qat[4] = {1.0000000,  11.368190, 28.982246, 19.818457};




//////////////////// Hyperbolic functions ////////////////////

////////////////////////////////////////////////////////////////////////////
//   float cosh(float x)
////////////////////////////////////////////////////////////////////////////
// Description : Computes the hyperbolic cosine value of x
// Returns : returns the hyperbolic cosine value of x
// Date : N/A
//

float cosh(float x)
{
   return ((exp(x)+exp(-x))*0.5);
}


////////////////////////////////////////////////////////////////////////////
//   float sinh(float x)
////////////////////////////////////////////////////////////////////////////
// Description : Computes the hyperbolic sine value of x
// Returns : returns the hyperbolic sine value of x
// Date : N/A
//

float sinh(float x)
{

   return ((exp(x) - exp(-x))*0.5);
}


////////////////////////////////////////////////////////////////////////////
//   float tanh(float x)
////////////////////////////////////////////////////////////////////////////
// Description : Computes the hyperbolic tangent value of x
// Returns : returns the hyperbolic tangent value of x
// Date : N/A
//

float tanh(float x)
{
   return(sinh(x)/cosh(x));
}


////////////////////////////////////////////////////////////////////////////
//   float frexp(float x, signed int *exp)
////////////////////////////////////////////////////////////////////////////
// Description : breaks a floating point number into a normalized fraction and an integral
// power of 2. It stores the integer in the signed int object pointed to by exp.
// Returns : returns the value x, such that x is a double with magnitude in the interval
// [1/2,1) or zero, and value equals x times 2 raised to the power *exp.If value is zero,
// both parts of the result are zero.
// Date : N/A
//

#define LOG2 .30102999566398119521
#define LOG2_INV 3.32192809488736234787
float frexp(float x, int8_t *exp)
{
   float res;
   int_fast8_t sign = 0;
   if(x == 0.0)
   {
      *exp=0;
      return (0.0);
   }
   if(x < 0.0)
   {
     x=-x;
     sign=1;
   }
   if (x > 1.0)
   {
      *exp=(ceil(log10(x)*LOG2_INV));
      res=x/(pow(2, *exp));
      if (res == 1)
      {
         *exp=*exp+1;
          res=.5;
      }
   }
   else
   {
      if(x < 0.5)
      {
         *exp=-1;
         res=x*2;
      }
      else
      {
         *exp=0;
          res=x;
      }
   }
   if(sign)
   {
      res=-res;
   }
   return res;
}



//////////////////////////////////////////////////////////////////////////////
//   float ldexp(float x, signed int *exp)
//////////////////////////////////////////////////////////////////////////////
// Description : multiplies a floating point number by an integral power of 2.
// Returns : returns the value of x times 2 raised to the power exp.
// Date : N/A
//

float ldexp(float value, int8_t exp)
{
   return (value * pow(2,exp));
}



Rad_0x3F2FF940  error_4  atan2_0x3F2FF944  sin_634.527504444e-3  cos_772.900223731e-3
Rad_0x3F3557E8  error_4  atan2_0x3F3557EC  sin_650.598764419e-3  cos_759.42158699e-3
Rad_0x3F394B90  error_4  atan2_0x3F394B94  sin_662.243723869e-3  cos_749.28843975e-3
Rad_0x3F3A71D0  error_4  atan2_0x3F3A71D4  sin_665.601253509e-3  cos_746.307492256e-3
Rad_0x3F3B1750  error_4  atan2_0x3F3B1754  sin_667.48380661e-3  cos_744.624257087e-3
Rad_0x3F497738  error_4  atan2_0x3F49773C  sin_708.221197128e-3  cos_705.990552902e-3
Rad_0x3F4B25C8  error_4  atan2_0x3F4B25CC  sin_712.844133377e-3  cos_701.322436332e-3
Rad_0x3F4D6228  error_4  atan2_0x3F4D622C  sin_718.942046165e-3  cos_695.0699687e-3
Rad_0x3F4D8D20  error_4  atan2_0x3F4D8D24  sin_719.397604465e-3  cos_694.598436355e-3
Rad_0x3F4EFD78  error_4  atan2_0x3F4EFD7C  sin_723.290205001e-3  cos_690.544128417e-3
Rad_0x3F5044D0  error_4  atan2_0x3F5044D4  sin_726.730346679e-3  cos_686.922788619e-3
Rad_0x3F5519F0  error_4  atan2_0x3F5519F4  sin_739.567160606e-3  cos_673.082709312e-3
Rad_0x3F597430  error_4  atan2_0x3F597434  sin_750.90354681e-3  cos_660.411834716e-3
Rad_0x3F5A1ED0  error_4  atan2_0x3F5A1ED4  sin_752.620398998e-3  cos_658.454596996e-3
Rad_0x40C90FDA  error_1  atan2_0x40C90FDA  sin_-476.837155e-9  cos_1.0 END



*/

#endif
