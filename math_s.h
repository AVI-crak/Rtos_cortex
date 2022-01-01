/// древнее зло на новый лад
/// в процессе %0,6

/// + https://github.com/xboxfanj/math-neon
/// + gcc\math.h
/// + собсвенное мнение

#ifndef _MATHSTM_
#define _MATHSTM_
#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */



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



union f__raw {
    struct {
        uint32_t massa  :23;
        uint32_t order  :8;
        uint32_t sign   :1;
    };
    int32_t     i_raw;
    float       f_raw;
};

union d__raw {
    struct {
        uint64_t massa  :52;
        uint64_t order  :11;
        uint64_t sign   :1;
    };
    uint32_t    u_raw[2];
    int64_t     i64_raw;
    double      d_raw;
};


#if ((defined (__ARM_FP)) && (__ARM_FP == 14) && ARM_MATH_CM7)
#define _ARM_CODE_V   6
#elif ((defined (__ARM_FP)) && (__ARM_FP == 12) && ARM_MATH_CM7)
#define _ARM_CODE_V   5
#elif ((defined (__ARM_FP)) && (__ARM_FP == 4) && ARM_MATH_CM7)
#define _ARM_CODE_V   4
#elif ((defined (__ARM_FP)) && (__ARM_FP == 4) && ARM_MATH_CM4)
#define _ARM_CODE_V   3
#elif (ARM_MATH_CM3)
#define _ARM_CODE_V   2
#elif (ARM_MATH_CM0)
#define _ARM_CODE_V   1
#else
#define _ARM_CODE_V   0
#endif



/// x=(float)((int32_t)x)
#if (_ARM_CODE_V >= 4)
static inline float rint_f(float value)
{
    float rep;
    asm volatile ("vrintz.f32 %0,%1" : "=t"(rep) : "t"(value));
    return rep;
}
#else
float rint_f(float value);
#endif

/// round towards Plus Infinity
#if (_ARM_CODE_V >= 4)
static inline float ceil_f(float value)
{
    float rep;
    asm volatile ("vrintp.f32 %0,%1" : "=t"(rep) : "t"(value));
    return rep;
}
#else
float ceil_f(float value);
#endif


/// round towards Minus Infinity
#if (_ARM_CODE_V >= 4)
static inline float floor_f(float value)
{
    float rep;
    asm volatile ("vrintm.f32 %0,%1" : "=t"(rep) : "t"(value));
    return rep;
}
#else
float floor_f(float value);
#endif


/// round to the nearest integer
#if (_ARM_CODE_V >= 4)
static inline float round_f(float value)
{
    float rep;
    asm volatile ("vrinta.f32 %0,%1" : "=t"(rep) : "t"(value));
    return rep;
}
#else
float round_f(float value);
#endif




/// return fractional remainder of division
float fmod_f(float value, float divider);


/// return |value|
#if (_ARM_CODE_V >= 4)
static inline float abs_f(float value)
{
    float rep;
    asm volatile ("vabs.f32 %0, %1" : "=t"(rep) : "t"(value));
    return rep;
}
#else
float abs_f(float value);
#endif

/// ошибка в младших битах
int32_t fbit_error(float a_compare, float b_compare);

/// ошибка в младших битах
int64_t dbit_error(double a_compare, double b_compare);





/// Raising the number E to the power of a value
/// Возведение числа E в степень value
float exp_f(float value);





/// Logarithm value for base 2,10,e
/// Логарифм value по основанию 2,10,e
float log_f(float value, int_fast8_t base_2_10_e);




/// returns fractional remainder
/// возвращает дробный остаток
static inline float mod_f(float value)
{
    return(value - rint_f(value));
}


/// exponentiation value^degree
/// Возведение в степень value^degree
float pow_f(float value,float degree);


/// Extraction of the square root value
/// Извлечение квадратного корня value
#if (_ARM_CODE_V >= 3)
static inline float sqrt_f(float value)
{
    float rep;
    asm volatile ("vsqrt.f32 %0,%1" : "=t"(rep) : "t"(value));
    return rep;
}
#else
float sqrt_f(float value);
#endif

float fact_f(float value);


/// Converting degrees to radians
float deg_rad(float value_deg);

/// Converting radians to degrees
float rad_deg(float value_rad);


/// sin input is radian, output 1.0:-1.0, error 1L
float sin_f(float value_rad);

/// sin input is radian, output 1.0:-1.0, error 1L
float cos_f(float value_rad);


double sin_d(double value_rad);

/// cos input is radian +-2pi, output 1.0:-1.0.
double cos_d(double value_rad);


// error 1/5000
float sin__f(float value_rad);






/// tan input is radian -pi/2:+pi/2, output min:max.
/// error (-+pi/2) 1L
float tan_f(float value_rad);


/// in -1.0f:+1.0f, out 0:2pi
float asin_f(float value);


/// in -1.0f:+1.0f, out 0:2pi
float acos_f(float value);




/// atan input (sin/cos), output +pi:-pi, error 2L
float atan_f(float value);

/// atan2 input +-1.0, output 0:2pi, error 3L
float atan2_f(float value_sin, float value_cos);




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

/// tab = 1.0f; s = 2.0f;
/// (tab *= s++; tab *= s++;) x(n) = -1.0f/tab;
const uint32_t table_const_factorial[14] =
{
0xBE2AAAAB, 0x3C088889, 0xB9500D01, 0x3638EF1D, 0xB2D7322B, 0x2F309231, 0xAB573FA0, 0x274A963C, 0xA317A4DB, 0x1EB8DC78,
0x9A3B0DA0, 0x159F9E66, 0x90E8D58D, 0x0C12CFCB,
};




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

/// sin input is radian +-2pi, output 1.0:-1.0.
/// min |x|=2.62879913375e-23 error 1L
float sin_f(float value_rad)
{
    float ret, rev, res;
    if  (abs_f(value_rad) < 2.62879913375e-23f ) return value_rad;
    ret = value_rad;
    if (ret < (PI/(-2.0f))) ret += Pi2;
    if (ret > (Pi+ PI/2.0f)) ret -= Pi2;
    else if (ret > (PI/2.0f)) ret = PI - ret;
    rev = ret * ret;
    res = rev * 1.60590443721e-10f; // 1/13!
    res -= 2.50521079437e-08f; // -1/11!
    res *= rev; res += 2.75573188446e-06f; // 1/9!
    res *= rev; res -= 0.000198412701138f; // 1/7!
    res *= rev; res += 0.00833333469927f; // 1/5!
    res *= rev; res -= 0.166666671634f; // 1/3!
    res *= rev; res *= ret; res += ret;
    return res;
};



*/



#ifdef __cplusplus
}
#endif /* __cplusplus  */
#endif /* _MATHSTM_ */


