
/// древнее зло на новый лад
/// в процессе %0,4

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
#define PI      3.1415927410125732421875f   /// 0x40490fdb
#define Pi      3.141592502593994140625f    /// 0x40490fda
#define Pi_c    3.14159297943115234375f     /// 0x40490fdc
#define Pi2     6.283185482025146484375     /// 0x40c90fdb
//#define PIerror1_5  2.264936966867026058025658130645751953125E-7
///original pi 3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679

// 0x42652ee1 +-
// 0x42652ee0 +
// 0x42652ee2 -

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
    asm volatile ("vabs.f32 %0,%1" : "=t"(rep) : "t"(value));
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

/// ошибка
float error_value(float x, float y)
{
    if (x>y) return abs_f(x-y);
        else return abs_f(y-x);
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
    rad = fmod_f(value_deg, 360.0f);
    rad *= 1.11701071262f;
	rad /=64.0f;
    return rad;
}

/// tab = 1.0f; s = 2.0f;
/// (tab *= s++; tab *= s++;) x(n) = -1.0f/tab;
const uint32_t table_const_factorial[14] =
{
0xBE2AAAAB, 0x3C088889, 0xB9500D01, 0x3638EF1D, 0xB2D7322B, 0x2F309231, 0xAB573FA0, 0x274A963C, 0xA317A4DB, 0x1EB8DC78,
0x9A3B0DA0, 0x159F9E66, 0x90E8D58D, 0x0C12CFCB,
};

/// error 0,0%, 8~14 steps(8-10-12-14)
/// sin input is radian 0:+2pi, output 1.0:-1.0.
float sin_f(float value_rad)
{
    float rep, ret, rev;
    if (value_rad < 0) value_rad = value_rad + Pi2;
    if (value_rad >= (PI+ Pi/2.0f)) value_rad -= Pi2;
        else if (value_rad > Pi/2.0f) value_rad = PI - value_rad;
    if  (abs_f(value_rad) < 1.0e-35 ) return value_rad;
    rep = value_rad;
    float* tab; tab = (float*) table_const_factorial;
    ret = rep; rev = rep * rep;
    int32_t nex = 0;
    float rem[14];
    do
    {
        ret *= rev;
        rem[nex] = ret * tab[nex++];
    }while (nex != 14);
    while (nex) rep += rem[--nex];
    return rep;
}

/// table sin 0:pi/2
const uint32_t  table_const_sin_0_pi05[256]={
0x00000000, 0x3BC9D961, 0x3C49D866, 0x3C976113, 0x3CC9D47A, 0x3CFC45EC, 0x3D175A75, 0x3D30907B, 0x3D49C4CC, 0x3D62F725,
0x3D7C274B, 0x3D8AAA7F, 0x3D973FFF, 0x3DA3D407, 0x3DB06677, 0x3DBCF733, 0x3DC98616, 0x3DD61305, 0x3DE29DE1, 0x3DEF2688,
0x3DFBACDC, 0x3E04185F, 0x3E0A5908, 0x3E109859, 0x3E16D642, 0x3E1D12B3, 0x3E234DA0, 0x3E2986F5, 0x3E2FBEA4, 0x3E35F4A0,
0x3E3C28D7, 0x3E425B38, 0x3E488BB7, 0x3E4EBA44, 0x3E54E6CE, 0x3E5B1148, 0x3E6139A0, 0x3E675FC7, 0x3E6D83AF, 0x3E73A54A,
0x3E79C485, 0x3E7FE155, 0x3E82FDD3, 0x3E8609B7, 0x3E89144D, 0x3E8C1D8F, 0x3E8F2574, 0x3E922BF5, 0x3E95310B, 0x3E9834AD,
0x3E9B36D4, 0x3E9E377A, 0x3EA13697, 0x3EA43422, 0x3EA73016, 0x3EAA2A6A, 0x3EAD2315, 0x3EB01A13, 0x3EB30F5B, 0x3EB602E5,
0x3EB8F4AD, 0x3EBBE4A6, 0x3EBED2CC, 0x3EC1BF18, 0x3EC4A983, 0x3EC79205, 0x3ECA7896, 0x3ECD5D2E, 0x3ED03FCA, 0x3ED3205E,
0x3ED5FEE7, 0x3ED8DB5B, 0x3EDBB5B4, 0x3EDE8DE8, 0x3EE163F5, 0x3EE437D1, 0x3EE70976, 0x3EE9D8DD, 0x3EECA5FE, 0x3EEF70D1,
0x3EF23952, 0x3EF4FF79, 0x3EF7C33E, 0x3EFA8499, 0x3EFD4389, 0x3F000000, 0x3F015CFF, 0x3F02B8BB, 0x3F041330, 0x3F056C5F,
0x3F06C441, 0x3F081AD6, 0x3F097017, 0x3F0AC403, 0x3F0C1695, 0x3F0D67CA, 0x3F0EB7A1, 0x3F100614, 0x3F115322, 0x3F129EC7,
0x3F13E8FC, 0x3F1531C5, 0x3F167918, 0x3F17BEF7, 0x3F19035C, 0x3F1A4643, 0x3F1B87AE, 0x3F1CC792, 0x3F1E05F2, 0x3F1F42C9,
0x3F207E15, 0x3F21B7D0, 0x3F22EFFA, 0x3F24268F, 0x3F255B8B, 0x3F268EED, 0x3F27C0AE, 0x3F28F0D0, 0x3F2A1F4F, 0x3F2B4C25,
0x3F2C7752, 0x3F2DA0D2, 0x3F2EC8A2, 0x3F2FEEBF, 0x3F311326, 0x3F3235D6, 0x3F3356CA, 0x3F347601, 0x3F359376, 0x3F36AF28,
0x3F37C914, 0x3F38E136, 0x3F39F78D, 0x3F3B0C16, 0x3F3C1ECD, 0x3F3D2FB1, 0x3F3E3EBD, 0x3F3F4BF2, 0x3F40574A, 0x3F4160C3,
0x3F42685D, 0x3F436E13, 0x3F4471E2, 0x3F4573C9, 0x3F4673C6, 0x3F4771D4, 0x3F486DF2, 0x3F49681F, 0x3F4A6054, 0x3F4B5695,
0x3F4C4ADD, 0x3F4D3D27, 0x3F4E2D73, 0x3F4F1BBD, 0x3F500807, 0x3F50F248, 0x3F51DA84, 0x3F52C0B5, 0x3F53A4DA, 0x3F5486F1,
0x3F5566F8, 0x3F5644ED, 0x3F5720CB, 0x3F57FA93, 0x3F58D242, 0x3F59A7D5, 0x3F5A7B4C, 0x3F5B4CA3, 0x3F5C1BD7, 0x3F5CE8EA,
0x3F5DB3D7, 0x3F5E7C9D, 0x3F5F433A, 0x3F6007AA, 0x3F60C9EF, 0x3F618A05, 0x3F6247E7, 0x3F63039A, 0x3F63BD17, 0x3F64745D,
0x3F65296C, 0x3F65DC41, 0x3F668CDB, 0x3F673B35, 0x3F67E752, 0x3F68912E, 0x3F6938C7, 0x3F69DE1C, 0x3F6A812E, 0x3F6B21F8,
0x3F6BC078, 0x3F6C5CAD, 0x3F6CF697, 0x3F6D8E33, 0x3F6E2382, 0x3F6EB680, 0x3F6F472E, 0x3F6FD586, 0x3F70618A, 0x3F70EB3A,
0x3F717292, 0x3F71F791, 0x3F727A37, 0x3F72FA84, 0x3F737871, 0x3F73F403, 0x3F746D34, 0x3F74E407, 0x3F755878, 0x3F75CA88,
0x3F763A35, 0x3F76A77C, 0x3F771260, 0x3F777ADC, 0x3F77E0F0, 0x3F78449D, 0x3F78A5DE, 0x3F7904B8, 0x3F796125, 0x3F79BB27,
0x3F7A12BB, 0x3F7A67E1, 0x3F7ABA98, 0x3F7B0AE1, 0x3F7B58B7, 0x3F7BA420, 0x3F7BED15, 0x3F7C3397, 0x3F7C77A7, 0x3F7CB944,
0x3F7CF869, 0x3F7D351B, 0x3F7D6F57, 0x3F7DA71E, 0x3F7DDC6C, 0x3F7E0F44, 0x3F7E3FA5, 0x3F7E6D8D, 0x3F7E98FD, 0x3F7EC1F1,
0x3F7EE86E, 0x3F7F0C72, 0x3F7F2DFA, 0x3F7F4D09, 0x3F7F699D, 0x3F7F83B2, 0x3F7F9B4F, 0x3F7FB06F, 0x3F7FC315, 0x3F7FD33E,
0x3F7FE0EA, 0x3F7FEC1C, 0x3F7FF4D0, 0x3F7FFB07, 0x3F7FFEC1, 0x3F800000,};


/// error 0,0002%, 0,0064% coincidence of
/// sin input is radian -2pi:+2pi, output 1.0:-1.0.
float sin_f_fast(float value_rad)
{
    float res, rrg; int32_t nxi;
    float* tab; tab = (float*) table_const_sin_0_pi05;
    if (value_rad < 0) value_rad = value_rad + Pi2;
    if (value_rad >= (PI+ Pi/2.0f)) value_rad -= Pi2;
        else if (value_rad > Pi/2.0f) value_rad = PI - value_rad;
    if  (abs_f(value_rad) < 1.0e-35 ) return value_rad;

    rrg = abs_f(value_rad * 162.338043212890625f);//((1/(PI/2)) * 255.0f); /// 255.0f = the size of the array of constants of sin (0:pi/2)
    nxi = (int32_t)rrg; rrg -= (float)nxi ;
    res =  rrg * tab[nxi+1] + (1.0f - rrg) * tab[nxi]
            + ( (tab[nxi+1]-tab[nxi]) * (0.25f-(0.5f-rrg)*(0.5f-rrg)) / (512.0f) ); /// + coincidence
    if (value_rad < 0) res *= -1.0f;
    return res;
}

/// cos input is radian -2pi:+2pi, output 1.0:-1.0.
float cos_f(float value_rad)
{
    if ((value_rad + Pi/2.0f) >= Pi2) return sin_f(value_rad - (PI+ Pi/2.0f));
    else return sin_f(value_rad + Pi/2.0f);
}

/// cos input is radian 0:+2pi, output 1.0:-1.0.
float cos_f_fast(float value_rad)
{
    if ((value_rad + Pi/2.0f) >= Pi2) return sin_f_fast(value_rad - (PI+ Pi/2.0f));
    else return sin_f_fast(value_rad + Pi/2.0f);
}



double sin_d(double value_rad)
{
    double rep, rep_z, ret, rev, fac, fac_i;
    rep = value_rad; ret = rep; rev = rep * rep;
    fac = 1.0d; fac_i = 2.0d;
    do
    {
        rep_z = rep;
        ret *= rev; fac *= fac_i++; fac *= fac_i++;
        rep -= ret / fac;
        ret *= rev; fac *= fac_i++; fac *= fac_i++;
        rep += ret / fac;
    }while (rep_z != rep);
    return rep;
}



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







const float atanhi[] = {
  4.6364760399e-01f, /* atan(0.5)hi 0x3eed6338 */
  7.8539812565e-01f, /* atan(1.0)hi 0x3f490fda */
  9.8279368877e-01f, /* atan(1.5)hi 0x3f7b985e */
  1.5707962513e+00f, /* atan(inf)hi 0x3fc90fda */
};


const float atanlo[] = {
  5.0121582440e-09f, /* atan(0.5)lo 0x31ac3769 */
  3.7748947079e-08f, /* atan(1.0)lo 0x33222168 */
  3.4473217170e-08f, /* atan(1.5)lo 0x33140fb4 */
  7.5497894159e-08f, /* atan(inf)lo 0x33a22168 */
};


const uint32_t table_const_aT[11] = {
    0x3c8569d7, //[10] 1.6285819933e-02f
    0x3d4bda59, //[8]  4.9768779427e-02f
    0x3d886b35, //[6]  6.6610731184e-02f
    0x3dba2e6e, //[4]  9.0908870101e-02f
    0x3e124925, //[2]  1.4285714924e-01f
    0x3eaaaaaa, //[0]  3.3333334327e-01f
    0xbd15a221, //[9] -3.6531571299e-02f
    0xbd6ef16b, //[7] -5.8335702866e-02f
    0xbd9d8795, //[5] -7.6918758452e-02f
    0xbde38e38, //[3] -1.1111110449e-01f
    0xbe4ccccd //[1] -2.0000000298e-01f
};



static const float huge   = 1.0e30f;


float atan_f(float value)
{
	float rep,pif,qif,zik;
    int_fast8_t sign, idx;
    float* tab; tab = (float*) table_const_aT;
	union f__raw ata;
	ata.f_raw = value; sign = ata.sign;
	ata.sign = 0;
	if(ata.i_raw >= 0x50800000)             /// if |value| >= 2^34
    {
        if((ata.order == 0xFF)&&(ata.massa !=0))
        {
            ata.i_raw = 0x01FF << 22; ata.sign = sign;
            return ata.f_raw;               /// NaN
        };
        if(sign != 0)
            return  (-atanhi[3] - atanlo[3]);
        else
            return (+atanhi[3]+atanlo[3]);
    }else if (ata.i_raw  < 0x3ee00000)      /// |value| < 0.4375
	{
	    if (ata.i_raw  < 0x31000000)        /// |value| < 2^-29
	    {
            if(1.0e30f + value > 1.0f)      /// raise inexact
                return value;
	    };
	    idx = -1;
	} else
	{
        value = ata.f_raw;
        if (ata.i_raw  < 0x3f980000)        /// |value| < 1.1875
        {
            if (ata.i_raw  < 0x3f300000)    /// 7/16 <=|value|<11/16
            {
                idx = 0; value = (2.0f * value - 1.0f)/(2.0f + value);
            }else                           /// 11/16<=|value|< 19/16
            {
                idx = 1; value  = (value - 1.0f)/(value + 1.0f);
            }
        } else
        {
            if (ata.i_raw  < 0x401c0000)    /// |value| < 2.4375
            {
                idx = 2; value  = (value - 1.5f)/(1.0f + 1.5f * value);
            } else                          /// 2.4375 <= |value| < 2^66
            {
                idx = 3; value  = -1.0f / value;
            };
        };
    };
    /// end of argument reduction
	zik = value * value;
	rep = zik * zik;
    /// break sum from i=0 to 10 tab[i]zik**(i+1) into odd and even poly
	pif = zik * (tab[5] + rep * (tab[4] + rep * (tab[3] + rep * (tab[2] + rep * (tab[1] + rep * tab[0])))) );
	qif = rep * (tab[10] + rep * (tab[9] + rep * (tab[8] + rep * (tab[7] + rep * tab[6]))) );
	if (idx < 0) return value - value * (pif + qif);
	else
    {
	    zik = atanhi[idx] - ((value * (pif + qif) - atanlo[idx]) - value);
        if(sign != 0) return -zik; else return zik;
	};
}


/// Returns the angle in radians 0: 2pi
/// Возвращает значение угла в радианах 0:2pi
float atan2_f(float value_sin, float value_cos)
{
    union f__raw ata_si; union f__raw ata_co;
    ata_si.f_raw = value_sin; ata_co.f_raw = value_cos;
    int_fast8_t sign_si, sign_co; float rep;
    sign_si = ata_si.sign; sign_co = ata_co.sign;
    ata_si.sign = 0; ata_co.sign = 0;
    if (ata_co.i_raw < 0x01800000)                  /// |co| < 4.7e-38
    {
        if (ata_si.i_raw < 0x01800000) return 0.0f; /// |si| < 4.7e-38
        if (sign_si != 0) return (PI+ Pi/2.0f); else return Pi/2.0f;
    }else if (ata_si.i_raw < 0x01800000)            /// |si| < 4.7e-38
    {
        if (sign_co != 0) return PI; else return 0.0f;
    };
    rep = atan_f( value_sin / value_cos);
    if (sign_co != 0) return (PI + rep);
        else if (sign_si != 0) return ( Pi2 + rep);
            else return (rep);
}


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




/*

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

////////////////////////////////////////////////////////////////////////////
//   float atan(float x)
////////////////////////////////////////////////////////////////////////////
// Description : returns the arctangent value of the value x.
// Date : N/A
//
float atan(float x)
{
   float y, res, r;
   int_fast8_t s, flag;

   s = 0;
   flag = 0;
   y = x;

   if (x < 0)
   {
      s = 1;
      y = -y;
   }

   if (y > 1.0)
   {
      y = 1.0/y;
      flag = 1;
   }

   res = pat[0]*y*y + pat[1];
   res = res*y*y + pat[2];
   res = res*y*y + pat[3];

   r = qat[0]*y*y + qat[1];
   r = r*y*y + qat[2];
   r = r*y*y + qat[3];

   res = y*res/r;


   if (flag)                              // for |x| > 1
      res = PI_DIV_BY_TWO - res;
   if (s)
      res = -res;

   return(res);
}


/////////////////////////////////////////////////////////////////////////////
//   float atan2(float y, float x)
/////////////////////////////////////////////////////////////////////////////
// Description :computes the principal value of arc tangent of y/x, using the
// signs of both the arguments to determine the quadrant of the return value
// Returns : returns the arc tangent of y/x.
// Date : N/A
//

float atan2(float y,float x)
{
   float z;
   int_fast8_t sign;
   uint8_t quad;
   sign=0;
   quad=0; //quadrant
   quad=((y<=0.0)?((x<=0.0)?3:4):((x<0.0)?2:1));
   if(y<0.0)
   {
      sign=1;
      y=-y;
   }
   if(x<0.0)
   {
      x=-x;
   }
   if (x==0.0)
   {
      if(y==0.0)
      {
      #ifdef _ERRNO
      {
         errno=EDOM;
      }
      #endif
      }
      else
      {
         if(sign)
         {
         return (-(PI_DIV_BY_TWO));
         }
         else
         {
         return (PI_DIV_BY_TWO);
         }
      }
   }
   else
   {
      z=y/x;
      switch(quad)
      {
         case 1:
         {
            return atan(z);
            break;
         }
         case 2:
         {
//            return (atan(z)+PI_DIV_BY_TWO);  //2L3122
            return (PI-atan(z));
            break;
         }
         case 3:
         {
            return (atan(z)-PI);
            break;
         }
         case 4:
         {
            return (-atan(z));
            break;
         }
      }
   }
}



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
*/

#endif
