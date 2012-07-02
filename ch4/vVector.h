#ifndef __VECTR__
#define __VECTR__
//#undef __SSE__
//#undef __SSE2__
//#undef __SSE3__
//#undef __SPU__
/* THE ABSOLUTE GUIDELINE is that float[] etc align with vFloat[], [0]==[0] etc.
// For each vector architecture...
// __SPU__ IBM/Sony CBE SPE
// __VEC__ Altivec, Mac PPC, IBM/Sony PPE
// __SSE__ __SSE2__ __SSE3__ Intel Pentium
// __SCALAR__ DEFINED BY THIS INTERFACE IF NO OTHER OPTION IS SELECTED
*/
#ifdef __SCALAR__
#undef __VEC__
#undef __SSE__
#undef __SSE2__
#undef __SS3E__
#undef __SSE4__
#endif
#ifndef __VEC__
#define vALL(x) ((x)==15)
#define vANY(x) ((x)!=0)
#define vNONE(x) ((x)==0)
#endif
#define PSLOT 0		/*preferred slot in vector for scalar operations*/

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))    
#endif

#include <assert.h>
#ifdef WIN32
#define inline __inline
/*MS says that inline is only supported in C++*/
#endif
#include <stdlib.h>
#if !defined(posix_memalign)&&(_POSIX_C_SOURCE < 200112L)&&(_XOPEN_SOURCE < 600)
static int posix_memalign(void **a,int b,int c) {*a=malloc(c); return *a==NULL;}
#endif
#if defined( __SPU__ )
#include <vec_types.h>
#define tVECTOR "spu"
#ifndef abs
#define abs(x) ((x)<0?-(x):(x))
#endif

    typedef union
    {
        float             a[4];
        int               b[4];
        vector float xxx;
        vector signed int yyy;
    } vFloat;
    typedef union
    {
        struct {
        vector double xxx,yyy;
        };
        double             a[4];
        long long          b[4];
    } vDouble;
    typedef union
    {
        int             a[4];
        vector signed int xxx;
        vector signed short yyy;
    } vInt32;
    typedef union
    {
        signed short     a[8];
        vector signed short xxx;
    } vInt16;
	
    #define _vabsd(x) ((vector double)spu_andc((vector unsigned long long)(x),spu_splats(0x8000000000000000ULL)))
    #define _vabs(x) ((vector float)spu_andc((vector unsigned long)(x),spu_splats(0x80000000UL)))
    static __inline vector double _vmaxd(vector double a, vector double b) {
       register vector double temp=spu_sub(a,b); vector signed int temp1;
        temp1=spu_rlmaska((vector signed int)temp,-7);
        temp1=spu_shuffle(temp1,temp1,((vector unsigned char){0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8}));
       return spu_sel(a,b,(vector unsigned char)temp1);
    }
    static __inline vector double _vmind(vector double a, vector double b) {
       register vector double temp=spu_sub(b,a); vector signed int temp1;
        temp1=spu_rlmaska((vector signed int)temp,-7);
        temp1=spu_shuffle(temp1,temp1,((vector unsigned char){0,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8}));
       return spu_sel(a,b,(vector unsigned char)temp1);
    }
static __inline vector unsigned int _vdivui(vector unsigned int dividend, vector unsigned int divisor)
{
  vector unsigned int cnt, cnt_d;
  vector unsigned int quotient;
  vector unsigned int delta, term, cmp;
  vector unsigned int one = spu_splats(1u);

  quotient = spu_splats(0u);
  cnt_d = spu_cntlz(divisor);

  /* If the divisor is 0, then force the dividend to zero and the
   * divisor to 1 so the result is 0.
   */
  cmp = spu_cmpeq(divisor, 0);
  dividend = spu_andc(dividend, cmp);
  divisor = spu_sub(divisor, cmp);

  one  = spu_andc(one, spu_cmpgt(divisor, dividend));
  while (spu_extract(spu_gather(one), 0)) {

    cnt = spu_cntlz(dividend);
    delta = spu_sub(cnt_d, cnt);

    term = spu_rl(divisor, (vector signed int)delta);
    cmp  = spu_cmpgt(term, dividend);
    delta = spu_add(delta, cmp);
    term  = spu_rlmask(term, (vector signed int)cmp);

    dividend = spu_sub(dividend, term);
    quotient = spu_add(quotient, spu_sl(one, delta));

    one = spu_andc(one, spu_cmpgt(divisor, dividend));
  }
  return (quotient);
}
static __inline vector signed int _vdivi(vector signed int dividend, vector signed int divisor)
{  vector signed int sign, sign1, sign2;
  vector signed int quotient;

  sign1 = spu_rlmaska(dividend, -31);
  sign2 = spu_rlmaska(divisor,  -31);
  sign  = spu_xor(sign1, sign2);

  /* Compute absolute values */
  dividend = spu_sub(spu_xor(dividend, sign1), sign1);
  divisor  = spu_sub(spu_xor(divisor,  sign2), sign2);

  quotient = (vector signed int)_vdivui((vector unsigned int)dividend, (vector unsigned int)divisor);

  /* Fix the sign */
  quotient = spu_andc(spu_sub(spu_xor(quotient,  sign), sign),
                      spu_andc(spu_rlmaska(quotient, -31), sign));
  return (quotient);
}
static __inline vector float _vdiv(vector float a, vector float b)
{
  vector float x0, q0, q1, q2, err;
  x0 = spu_re(b);
  q0 = spu_mul(a, x0);
  q1 = spu_nmsub(b, q0, a);
  q1 = spu_madd(x0, q1, q0);

  /* Conditionally bias the result by 1 ulp to 
   *preserve the identity function a = a / 1
   */
  q2 = (vec_float4)spu_add((vec_uint4)(q1), 1);
  err = spu_nmsub(b, q2, a);
  q2 = spu_sel(q1, q2, spu_cmpgt((vec_int4)err, -1));
  return (q2);
}
static __inline vector double _vred(vector double value_d)
{
  vector unsigned long long mask = spu_splats(0x7FF0000000000000ULL);
  vector float  x0;
  vector float  value;
  vector float  two   = spu_splats(2.0f);
  vector double two_d = spu_splats(2.0);
  vector double x1, x2, x3;
  vector double bias;

  /* Bias the divisor to correct for double exponents that are out of single range.
   */
  bias = spu_xor(spu_and(value_d, (vector double)mask), (vector double)mask);
  value = spu_roundtf(spu_mul(value_d, bias));
  x0 = spu_re(value);
  x1 = spu_extend(spu_mul(x0, spu_nmsub(value, x0, two)));
  x1 = spu_mul(x1, bias);
  x2 = spu_mul(x1, spu_nmsub(value_d, x1, two_d));
  x3 = spu_mul(x2, spu_nmsub(value_d, x2, two_d));
  return (x3);
}
static __inline vector float _vsqrt(vector float in)
{
  vector float y0, out;
  /* Perform one iteration of Newton-Raphson */
  y0 = spu_rsqrte(in);
  out = spu_mul(spu_nmsub(in, spu_mul(y0, y0), (vector float)(spu_splats(0x40400001u))),
        spu_mul(y0, spu_mul(in, spu_splats(0.5f))));
  out = spu_andc(out, (vector float)spu_cmpeq(in, spu_splats(0.0f)));
  return (out);
}
static __inline vector double _vsqrtred(vector double value_d)
{
  vector unsigned long long mask = spu_splats(0x7FE0000000000000ULL);
  vector unsigned long long mask2 = spu_splats(0x7FF0000000000000ULL);
  vector unsigned long long addend = spu_splats(0x2000000000000000ULL);
  vector float value, x0;
  vector double x1;
  vector double x2, x3;
  vector double half  = spu_splats(0.5);
  vector double three = spu_splats(3.0);
  vector double bias;

  /* Bias the divisor to correct for double exponents that are out of single range. */
  bias = spu_xor(spu_and(value_d, (vector double)mask), (vector double)mask2);
  value = spu_roundtf(spu_mul(value_d, bias));
  x0 = spu_rsqrte(value);

  bias = (vector double)(spu_add(spu_and(spu_rlmask((vector unsigned int)(bias), -1), (vector unsigned int)(mask2)),
                                 (vector unsigned int)(addend)));

  x1 = spu_extend(spu_mul(spu_mul(spu_splats(0.5f), x0),
                         spu_nmsub(value, spu_mul(x0, x0), spu_splats(3.0f))));  x1 = spu_mul(x1, bias);
  x2 = spu_mul(spu_mul(half, x1), spu_nmsub(value_d, spu_mul(x1, x1), three));
  x3 = spu_mul(spu_mul(half, x2), spu_nmsub(value_d, spu_mul(x2, x2), three));
  return (x3);
}
static __inline vector double _vsqrtd(vector double d)
{
  return _vred(_vsqrtred(d));
}

    #define vZero(x) {(x).xxx = (vector float) spu_splats(0);}
    #define vSplat(x,v,i) {(x).xxx = (vector float) spu_splats((float)((v).a[i]));}
    #define vConstant(x,v) {(x).xxx = (vector float)spu_splats((float)(v));}
    #define vVector(x,a,b,c,d) {(x).xxx = ((vector float){a,b,c,d}) ;}
    #define vShuffle1(x,a,b,c,d,e) {(x).xxx=spu_shuffle((a).xxx,(a).xxx,((vector unsigned char){(e)*4,(e)*4+1,(e)*4+2,(e)*4+3,(d)*4,(d)*4+1,(d)*4+2,(d)*4+3,(c)*4,(c)*4+1,(c)*4+2,(c)*4+3,(b)*4,(b)*4+1,(b)*4+2,(b)*4+3}));}
    #define vShuffle(x,y,a,b,c,d,e) {(x).xxx=spu_shuffle((y).xxx,(a).xxx,((vector unsigned char){(c)*4,(c)*4+1,(c)*4+2,(c)*4+3,(b)*4,(b)*4+1,(b)*4+2,(b)*4+3,(e)*4+16,(e)*4+17,(e)*4+18,(e)*4+19,(d)*4+16,(d)*4+17,(d)*4+18,(d)*4+19}));}
	#define vGreater(a,b) spu_extract(spu_gather(((vector signed int)spu_cmpgt((a).xxx,(b).xxx))),0)
	#define vGreaterEqual(a,b) spu_extract(spu_gather((spu_or((vector signed int)spu_cmpgt((a).xxx,(b).xxx),(vector signed int)spu_cmpeq((a).xxx,(b).xxx)))),0)
    #define vEqual(a,b) spu_extract(spu_gather(((vector signed int)spu_cmpeq((a).xxx,(b).xxx))),0)
	#define vEqual3(a,b) (vEqual(a,b)>=14)
    #define vLess(a,b) spu_extract(spu_gather(((vector signed int)spu_cmpgt((b).xxx,(a).xxx))),0)
    #define vLessEqual(a,b) spu_extract(spu_gather((spu_or((vector signed int)spu_cmpgt((b).xxx,(a).xxx),(vector signed int)spu_cmpeq((a).xxx,(b).xxx)))),0)
    #define vAdd(x,y,z) {(x).xxx = (vector float) spu_add((y).xxx,(z).xxx);}
	#define vAbs(x,y) {(x).xxx = _vabs((y).xxx);}
    #define vTrunc(x,y) {(x).xxx = spu_convtf(spu_convts((y).xxx,0),0);}
	#define vAddib(x,y,z) {(x).xxx = (vector float) spu_add((y).xxx,((vector float){z,z,z,z}));}
    #define vShiftRightib(x,y,z) {(x).xxx = (vector signed int)spu_rlmask((y).yyy,-(z));}
    #define vAndint(x,y,z) {(x).yyy = spu_and((y).yyy,(z).xxx);}
    #define vSubtract(x,y,z) {(x).xxx = (vector float) spu_sub((y).xxx,(z).xxx);}
    #define vMultiply(x,a,b) {(x).xxx=spu_mul((a).xxx,(b).xxx);}
    #define vDivide(x,a,b) {(x).xxx=_vdiv((a).xxx,(b).xxx);}
    #define vMax(x,a,b) {(x).xxx=spu_sel((a).xxx,(b).xxx, spu_cmpgt((b).xxx,(a).xxx));}
    #define vMin(x,a,b) {(x).xxx=spu_sel((b).xxx,(a).xxx, spu_cmpgt((b).xxx,(a).xxx));}
    #define vMADD(x,a,b,c) {(x).xxx=spu_madd((a).xxx,(b).xxx,(c).xxx);}
    #define vSquareRoot(a,b) {(a).xxx=_vsqrt((b).xxx);}
	#define vReciprocal(a,b) {(a).xxx=spu_re((b).xxx);}
    #define vReciprocalSquareRoot(a,b) {(a).xxx=spu_rsqrte((b).xxx);}
    //Int
    #define vZero_int(x) {(x).xxx = (vector signed int) spu_splats(0);}
	#define vSplat_int(x,v,i) {(x).xxx = (vector signed int) spu_splats((signed int)((v).a[i]));}
    #define vConstant_int(x,v) {(x).xxx = (vector signed int)spu_splats((signed int)(v));}
    #define vVector_int(x,a,b,c,d) {(x).xxx = ((vector signed int){a,b,c,d}) ;}
	#define vShuffle1_int vShuffle1
    #define vShuffle_int  vShuffle
	#define vAbs_int(x,a) {(x).xxx=spu_sel(spu_sub(0, (a).xxx), (a).xxx, spu_cmpgt((a).xxx, -1));}
    #define vEqual_int vEqual
	#define vGreater_int vGreater
    #define vGreaterEqual_int vGreaterEqual
    #define vLess_int vLess
	#define vLessEqual_int vLessEqual
    #define vMax_int vMax
	#define vMin_int vMin
    #define vSelectGreater_int(u,v,w,x,y) {(u).xxx = spu_sel((w).xxx,(v).xxx,spu_cmpgt((x).xxx,(y).xxx));}
    #define vSelectEqual(u,v,w,x,y) {(u).xxx = spu_sel((w).xxx,(v).xxx,spu_cmpeq((x).xxx,(y).xxx));}
    #define vShiftLeftib_int(x,y,z) {(x).xxx = (vector signed int) spu_sl((y).xxx,z);}
    #define vShiftLeftgib_int(x,y,z) {(x).xxx = (vector signed int) spu_slqwbyte((y).xxx,(z)/8);}
	#define vShiftRightib_int(x,y,z) {(x).xxx = (vector signed int) spu_rlmask((y).xxx,-(z));}
    #define vAdd_int(x,y,z) {(x).xxx = (vector signed int) spu_add((y).xxx,(z).xxx);}
	#define vAnd_int(x,y,z) {(x).xxx = (vector signed int) spu_and((y).xxx,(z).xxx);}
	#define vOr_int(x,y,z) {(x).xxx = (vector signed int) spu_or((y).xxx,(z).xxx);}
    #define vNand_int(x,y,z) {(x).xxx = (vector signed int) spu_nand((y).xxx,(z).xxx);}
    #define vXor_int(x,y,z) {(x).xxx = (vector signed int) spu_xor((y).xxx,(z).xxx);}
    #define vSubtract_int(x,y,z) {(x).xxx = (vector signed int) spu_sub((y).xxx,(z).xxx);}
	#define vAddib_int(x,y,z) {(x).xxx = (vector signed int) spu_add((y).xxx,z);}
	#define vAndib_int(x,y,z) {(x).xxx = (vector signed int) spu_and((y).xxx,z);}
	#define vOrib_int(x,y,z) {(x).xxx = (vector signed int) spu_or((y).xxx,z);}
    #define vXorib_int(x,y,z) {(x).xxx = (vector signed int) spu_xor((y).xxx,z);}
	#define vSquareRoot_int(a,b) {(a).xxx=(vector signed int)spu_convts(spu_add(_vsqrt(spu_convtf((b).xxx,0)),(vector float)spu_splats(0.5f)),0);}
    #define vDivide_int(x,a,b) {(x).xxx=_vdivi((a).xxx,(b).xxx);}
    #define vMultiplyLow_int(x,a,b) {(x).xxx=spu_mulo((a).yyy,(b).yyy);}/*lower 16 bits of each int*/
    #define vConvert_intto_float(a,b) {(a).xxx=spu_convtf((b).xxx,0);}
    #define vConvert_floatto_int(a,b) {(a).xxx=spu_convts((b).xxx,0);}
    //Double
    #define vZero_double(x) {(x).xxx = (vector double) spu_splats(0);(x).yyy = (vector double) spu_splats(0);}
    #define vSplat_double(x,y,i) {double qzt=y.a[i];(x).xxx = (vector double) spu_splats(qzt);(x).yyy = (vector double) spu_splats(qzt);}
    #define vConstant_double(x,v) {(x).xxx = (vector double)spu_splats((double)(v));(x).yyy = (vector double)spu_splats((double)(v));}
    #define vVector_double(x,a,b,c,d) {(x).xxx = ((vector double){a,b}) ;(x).yyy = ((vector double){c,d}) ;}
    #define vShuffle1_double(x,a,b,c,d,e) {register vector double zqt=spu_shuffle((a).xxx,(a).yyy,((vector unsigned char){(e)*8,(e)*8+1,(e)*8+2,(e)*8+3,(e)*8+4,(e)*8+5,(e)*8+6,(e)*8+7,(d)*8,(d)*8+1,(d)*8+2,(d)*8+3,(d)*8+4,(d)*8+5,(d)*8+6,(d)*8+7}));\
(x).yyy=spu_shuffle((a).xxx,(a).yyy,((vector unsigned char){(c)*8,(c)*8+1,(c)*8+2,(c)*8+3,(c)*8+4,(c)*8+5,(c)*8+6,(c)*8+7,(b)*8,(b)*8+1,(b)*8+2,(b)*8+3,(b)*8+4,(b)*8+5,(b)*8+6,(b)*8+7}));(x).xxx=zqt;}
    #define vShuffle_double(x,y,a,b,c,d,e) {register vector double zqt=spu_shuffle((y).xxx,(y).yyy,((vector unsigned char){(c)*8,(c)*8+1,(c)*8+2,(c)*8+3,(c)*8+4,(c)*8+5,(c)*8+6,(c)*8+7,(b)*8,(b)*8+1,(b)*8+2,(b)*8+3,(b)*8+4,(b)*8+5,(b)*8+6,(b)*8+7}));\(x).yyy=spu_shuffle((a).xxx,(a).yyy,((vector unsigned char){(e)*8,(e)*8+1,(e)*8+2,(e)*8+3,(e)*8+4,(e)*8+5,(e)*8+6,(e)*8+7,(d)*8,(d)*8+1,(d)*8+2,(d)*8+3,(d)*8+4,(d)*8+5,(d)*8+6,(d)*8+7}));(x).xxx=zqt;}
    #define vAbs_double(x,a) {(x).xxx = _vabsd((a).xxx);(x).yyy = _vabsd((a).yyy);}
    #define vAdd_double(x,a,b) {(x).xxx = spu_add((a).xxx,(b).xxx);(x).yyy = spu_add((a).yyy,(b).yyy);}
    #define vAddib_double(x,a,b) {(x).xxx = spu_add((a).xxx,spu_splats((double)(b)));(x).yyy = spu_add((a).yyy,spu_splats((double)(b)));}
    #define vSubtract_double(x,a,b) {(x).xxx = spu_sub((a).xxx,(b).xxx);(x).yyy = spu_sub((a).yyy,(b).yyy);}
    #define vMultiply_double(x,a,b) {(x).xxx = spu_mul((a).xxx,(b).xxx);(x).yyy = spu_mul((a).yyy,(b).yyy);}
    #define vDivide_double(x,a,b) {(x).xxx = spu_mul((a).xxx,_vred((b).xxx));(x).yyy = spu_mul((a).yyy,_vred((b).yyy));}
	#define vMADD_double(x,a,b,c) {(x).xxx = spu_madd((a).xxx,(b).xxx,(c).xxx);(x).yyy = spu_madd((a).yyy,(b).yyy,(c).yyy);}
    #define vMin_double(x,a,b) {(x).xxx=_vmind((a).xxx,(b).xxx);(x).yyy=_vmind((a).yyy,(b).yyy);}
    #define vMax_double(x,a,b) {(x).xxx=_vmaxd((a).xxx,(b).xxx);(x).yyy=_vmaxd((a).yyy,(b).yyy);}
	#define vSquareRoot_double(a,b) {(a).xxx=_vsqrtd((b).xxx);(a).yyy=_vsqrtd((b).yyy);}
	#define vReciprocalSquareRoot_double(a,b) {(a).xxx=_vsqrtred((b).xxx);(a).yyy=_vsqrtred((b).yyy);}
	#define vReciprocal_double(a,b) {(a).xxx=_vred((b).xxx);(a).yyy=_vred((b).yyy);}
     //Int16
    #define vZero_int16(x) {(x).xxx = (vector signed short) spu_splats(0);}
    #define vConstant_int16(x,v) {(x).xxx = (vector signed short)spu_splats((signed short)(v));}
    #define vVector_int16(x,a,b,c,d,e,f,g,h) {(x).xxx = ((vector signed short){a,b,c,d,e,f,g,h}) ;}
#elif defined(__VEC__)
// AltiVec, also IBM/SonyCBE PPE
#include <math.h>
#include <assert.h>
#include <altivec.h>
#define tVECTOR "altivec"
#define vALL(x) ((int)(x)==-1)
#define vANY(x) ((x)!=0)
#define vNONE(x) ((x)==0)
    // Set up a vector type for a float[4] array for each vector  type
    typedef union
    {
	float             a[4];
	signed int		  b[4];
	vector float xxx;
	vector int yyy;
    } vFloat;
    typedef union
    {
	double             a[4];
	vector float xxx[2];
    } vDouble;
    typedef union
    {
	signed int             a[4];
	vector signed int xxx;
    } vInt32;
    typedef union
    {
	signed short	a[8];
	vector signed short xxx;
    } vInt16;

    // Define some macros to map a virtual SIMD language to 
    // each actual SIMD language.
    #define vSplat(x, v, i )  { (x).xxx = vec_splat( v.xxx, i ); } 
    #define vAndint(a,b,c)    { (a).yyy = vec_and((b).yyy,(c).xxx);}
    #define vMADD(x,a,b,c)    { (x).xxx = vec_madd((a).xxx,(b).xxx,(c).xxx);}
    #define vMultiply(x,a,b)    { (x).xxx = vec_madd((a).xxx,(b).xxx,(vector float)vec_splat_u32(0));}
    #define vZero(x) {(x).a[0]=(x).a[1]=(x).a[2]=(x).a[3]=0.0f;}
    #define vConstant(x,y) {(x).a[0]=(x).a[1]=(x).a[2]=(x).a[3]=(float)(y);}
    #define vVector(x,aa,b,c,d) {(x).a[0]=(float)(aa);(x).a[1]=(float)(b);(x).a[2]=(float)(c);(x).a[3]=(float)(d);}
    #define vShiftRightib(x,a,b) {register vector unsigned int qz41={b,b,b,b}; (x).xxx = vec_sr((a).yyy,qz41);}
    #define vAdd(x,a,b)    { (x).xxx = vec_add((a).xxx,(b).xxx);}
    #define vAddib(x,y,b)    { x.a[0]=y.a[0]+b;x.a[1]=y.a[1]+b;x.a[2]=y.a[2]+b;x.a[3]=y.a[3]+b;}
    #define vSubtract(x,a,b)    { (x).xxx = vec_sub((a).xxx,(b).xxx);}
    #define vMax(x,a,b)    { (x).xxx = vec_max((a).xxx,(b).xxx);}
    #define vMin(x,a,b)    { (x).xxx = vec_min((a).xxx,(b).xxx);}
    #define vTrunc(x,a)    { (x).xxx = vec_trunc((a).xxx);}
    #define vAbs(x,a)    { (x).xxx = vec_abs((a).xxx);}
    #define vReciprocal(x,a)    { (x).xxx = vec_vrefp((a).xxx);}
    #define vReciprocalSquareRoot(x,a)    { (x).xxx = vec_vrsqrtefp((a).xxx);}
    #define vConvert_intto_float(a,b) {(a).xxx=vec_ctf((b).xxx,0);}
    #define vConvert_floatto_int(a,b) {(a).xxx=vec_cts((b).xxx,0);}
    //Int
    #define vSplat_int(x, v, i )  { (x).xxx = vec_splat( v.xxx, i ); } 
    #define vMADD_int(x,a,b,c)    { (x).xxx = vec_madd((a).xxx,(b).xxx,(c).xxx);}
    #define vZero_int(x) {(x).xxx = (vector int) vec_splat_s32(0);}
    #define vConstant_int(x,y) {(x).a[0]=(x).a[1]=(x).a[2]=(x).a[3]=(int)(y);}
    #define vVector_int(x,aa,b,c,d) {(x).a[0]=aa;(x).a[1]=b;(x).a[2]=c;(x).a[3]=d;}
    #define vShiftLeftib_int(x,a,b)    { (x).xxx = vec_sl((a).xxx,vec_splat_u32(b));}
    #define vShiftLeftgib_int(x,a,b)    {register vector unsigned char qz47=vec_splat_u8((b)/8); (x).xxx = vec_slo((a).xxx,vec_sl(qz47,vec_splat_u8(3)));}
    #define vShiftRightib_int(x,a,b)    { (x).xxx = vec_sr((a).xxx,vec_splat_u32(b));}
    #define vAdd_int(x,a,b)    { (x).xxx = vec_add((a).xxx,(b).xxx);}
    #define vAddib_int(x,a,b)    { (x).xxx = vec_add((a).xxx,(vector int) vec_splat_s32((int)(b)));}
    #define vMultiplyLow_int(x,a,b)    { (x).xxx = vec_mulo((vector signed short)(a).xxx,(vector signed short)(b).xxx);}
    #define vAbs_int(x,a)    { (x).xxx = vec_abs((a).xxx);}
    #define vAnd_int(x,a,b)    { (x).xxx = vec_and((a).xxx,(b).xxx);}
    #define vAndib_int(x,a,b)    { (x).xxx = vec_and((a).xxx,(vector int) vec_splat_s32((int)(b)));}
    #define vNand_int(x,a,b)    { (x).xxx = vec_xor(vec_and((a).xxx,(b).xxx),(vector int)vec_splat_u32(-1));}
    #define vOr_int(x,a,b)    { (x).xxx = vec_or((a).xxx,(b).xxx);}
    #define vOrib_int(x,a,b)    { (x).xxx = vec_or((a).xxx,(vector int) vec_splat_s32((int)(b)));}
    #define vXor_int(x,a,b)    { (x).xxx = vec_xor((a).xxx,(b).xxx);}
    #define vXorib_int(x,a,b)    { (x).xxx = vec_xor((a).xxx,(vector int) vec_splat_s32((int)(b)));}
    #define vSubtract_int(x,a,b)    { (x).xxx = vec_sub((a).xxx,(b).xxx);}
    #define vMax_int(x,a,b)    { (x).xxx = vec_max((a).xxx,(b).xxx);}
    #define vMin_int(x,a,b)    { (x).xxx = vec_min((a).xxx,(b).xxx);}
    #define vZero_int16(x) {(x).xxx = (vector signed short) vec_splat_s16(0);}
    #define vConstant_int16(x,y) {(x).a[0]=(x).a[1]=(x).a[2]=(x).a[3]=\
	(x).a[4]=(x).a[5]=(x).a[6]=(x).a[7]=(short)(y);}
    #define vVector_int16(x,aa,b,c,d,e,f,g,h) {(x).a[0]=aa;(x).a[1]=b;(x).a[2]=c;(x).a[3]=d;\
	(x).a[4]=e;(x).a[5]=f;(x).a[6]=g;(x).a[7]=h;}
/* double */
    #define vZero_double(x) {(x).xxx[0] = (vector float) vec_splat_u32(0);(x).xxx[1] = (vector float) vec_splat_u32(0);}
    #define vSelectEqual(u,v,w,x,y) {(u).xxx=vec_sel((w).xxx,(v).xxx,vec_cmpeq((x).xxx,(y).xxx));}
    #define vSelectGreater_int(u,v,w,x,y) {(u).xxx=vec_sel((w).xxx,(v).xxx,vec_cmpgt((x).xxx,(y).xxx));}
    static vector unsigned char _qx46={0,0,0,0,0,0,0,0,0,0,0,0,0x10,0x14,0x18,0x1C};
    static vector unsigned char _qx47={0,0,0,0,0,0,0,0,0,0,0,0,0x10,0x14,0x18,0x10};
    static inline int vEqual(vFloat a, vFloat b) {
	vInt32 x;
	(x).xxx=vec_perm(vec_splat_u32(0),(vector unsigned int)vec_cmpeq((a).xxx,(b).xxx),_qx46);
	return x.a[3];
    }
    static inline int vEqual3(vFloat a, vFloat b) {
	vInt32 x;
	(x).xxx=vec_perm(vec_splat_u32(0),(vector unsigned int)vec_cmpeq((a).xxx,(b).xxx),_qx47);
	return x.a[3]==-1;
    }
    static inline int vGreater(vFloat a, vFloat b) {
	vInt32 x;
	(x).xxx=vec_perm(vec_splat_u32(0),(vector unsigned int)vec_cmpgt((a).xxx,(b).xxx),_qx46);
	return x.a[3];
    }
    static inline int vLess(vFloat a, vFloat b) {
	vInt32 x;
	(x).xxx=vec_perm(vec_splat_u32(0),(vector unsigned int)vec_cmplt((a).xxx,(b).xxx),_qx46);
	return x.a[3];
    }
    static inline int vLessEqual(vFloat a, vFloat b) {
	vInt32 x;
	(x).xxx=vec_perm(vec_splat_u32(0),(vector unsigned int)vec_cmple((a).xxx,(b).xxx),_qx46);
	return x.a[3];
    }
    static inline int vGreaterEqual(vFloat a, vFloat b) {
	vInt32 x;
	(x).xxx=vec_perm(vec_splat_u32(0),(vector unsigned int)vec_cmpge((a).xxx,(b).xxx),_qx46);
	return x.a[3];
    }
    static inline int vEqual_int(vInt32 a, vInt32 b) {
	vInt32 x;
	(x).xxx=vec_perm(vec_splat_u32(0),(vector unsigned int)vec_cmpeq((a).xxx,(b).xxx),_qx46);
	return x.a[3];
    }
    static inline int vGreater_int(vInt32 a, vInt32 b) {
	vInt32 x;
	(x).xxx=vec_perm(vec_splat_u32(0),(vector unsigned int)vec_cmpgt((a).xxx,(b).xxx),_qx46);
	return x.a[3];
    }
    static inline int vLess_int(vInt32 a, vInt32 b) {
	vInt32 x;
	(x).xxx=vec_perm(vec_splat_u32(0),(vector unsigned int)vec_cmplt((a).xxx,(b).xxx),_qx46);
	return x.a[3];
    }
    static inline int vLessEqual_int(vInt32 a, vInt32 b) {
	vInt32 x;
	(x).xxx=vec_perm(vec_splat_u32(0),(vector unsigned int)vec_or(vec_cmplt((a).xxx,(b).xxx),vec_cmpeq((a).xxx,(b).xxx)),_qx46);
	return x.a[3];
    }
    static inline int vGreaterEqual_int(vInt32 a, vInt32 b) {
	vInt32 x;
	(x).xxx=vec_perm(vec_splat_u32(0),(vector unsigned int)vec_or(vec_cmpgt((a).xxx,(b).xxx),vec_cmpeq((a).xxx,(b).xxx)),_qx46);
	return x.a[3];
    }
#elif defined( __SSE__ )||defined( __SSE3__ )||defined( __SSE2__ )
// SSE
#define tVECTOR "sse"
    // The header file xmmintrin.h defines C functions for using 
    // SSE and SSE2 according to the Intel C programming interface
    #include <xmmintrin.h>
	#include <emmintrin.h>
    #ifdef __SSE3__
	#ifdef WIN32
	#include <intrin.h>
	#define inline __inline
    #else
    #include <pmmintrin.h>
    #endif
	#endif

    // Set up a vector type for a float[4] array for each vector  type
	typedef union
    {
	signed short             a[8];
	__m128i xxx;
    } vInt16;
    typedef union
    {
	float             a[4];
	__m128 xxx;
        __m128i yyy;
    } vFloat;
    typedef union
    {
	int               a[4];
	__m128i xxx;
	__m128  yyy;
    } vInt32;
	typedef union
    {
	struct {
		__m128d xxx,yyy;
		};
	double               a[4];
    } vDouble;
#ifdef WIN32
#define _mm_set_pd1 _mm_set1_pd
//__m128 _mm_castsi128_ps(__m128i n) { return *(__m128 *) &n; } defined in emmintrin.h
//inline __m128i _mm_castps_si128(__m128 n) { return *(__m128i*) &n; }
#endif         
	#define vAdd(x,a,b) {(x).xxx=_mm_add_ps( (a).xxx, (b).xxx );}	
	#define vConstant(x,c)		{(x).xxx=_mm_set_ps1(c); /*c,c,c,c*/}
	#define vCopy(a,b) {(a).xxx=(b).xxx;}
	#define vAndint(x,a,b) {(x).yyy=_mm_and_si128( (a).yyy,(b).xxx);} /* f f i */
	#define vDivide(x,a,b) {(x).xxx=_mm_div_ps( (a).xxx, (b).xxx );}
	#define vMADD(x,a,b,c) {(x).xxx=_mm_add_ps( (c).xxx, _mm_mul_ps( (a).xxx, (b).xxx ) );}
	#define vMax(x,a,b)			{(x).xxx=_mm_max_ps((a).xxx,(b).xxx);}
	#define vMin(x,a,b)			{(x).xxx=_mm_min_ps((a).xxx,(b).xxx);}
	#define vEqual(a,b)	(_mm_movemask_ps(_mm_cmpeq_ps( (a).xxx, (b).xxx )))
	#define vEqual3(a,b)	((_mm_movemask_ps(_mm_cmpeq_ps( (a).xxx, (b).xxx ))&7)==7)
	#define vGreater(a,b)	(_mm_movemask_ps(_mm_cmpgt_ps( (a).xxx, (b).xxx )))
	#define vLess(a,b)	(_mm_movemask_ps(_mm_cmplt_ps( (a).xxx, (b).xxx )))
	#define vGreaterEqual(a,b)	(_mm_movemask_ps(_mm_cmpge_ps( (a).xxx, (b).xxx )))
	#define vLessEqual(a,b)	(_mm_movemask_ps(_mm_cmple_ps( (a).xxx, (b).xxx )))
	#define vMultiply(x,a,b) {(x).xxx=_mm_mul_ps( (a).xxx, (b).xxx );}
	#define vReciprocal(x,a) {(x).xxx=_mm_rcp_ps( (a).xxx );}
	#define vReciprocalSquareRoot(x,a) {(x).xxx=_mm_rsqrt_ps( (a).xxx );}                                       
	#define vShuffle(x,a,b,c,d,e,f) {(x).xxx=_mm_shuffle_ps((a).xxx,(b).xxx,_MM_SHUFFLE(c,d,e,f)); /*L->R 0 1 2 3 -- 0 1 2 3 [dcfe]*/}
	#define vShuffle1(x,a,c,d,e,f) {(x).xxx=_mm_shuffle_ps((a).xxx,(a).xxx,_MM_SHUFFLE(c,d,e,f)); /*R->L 0 1 2 3 [cdef]*/}
	#define vSplat(x, v, i ) { (x).xxx = _mm_shuffle_ps( v.xxx,  v.xxx, _MM_SHUFFLE(i,i,i,i) ); }
	#define vAddib(x,a,b) {(x).xxx=_mm_add_ps( (a).xxx, _mm_set_ps1((float)(b)));}
	#define vSquareRoot(x,a) {(x).xxx=_mm_sqrt_ps( (a).xxx );}
	#define vAbs(x,a) {(x).xxx=_mm_max_ps( (a).xxx, _mm_sub_ps( _mm_setzero_ps(), (a).xxx ) );}
	#define vSubtract(x,a,b) {(x).xxx=_mm_sub_ps( (a).xxx, (b).xxx );}
	#define vVector(x,a,b,c,d)	{(x).xxx=_mm_setr_ps((float)(a),(float)(b),(float)(c),(float)(d));}
	#define vZero(x)            {(x).xxx=_mm_setzero_ps();}
static __m128 inline _mm_sel_ps( __m128 b, __m128 a, __m128 mask )
{   register __m128 msk=mask;
    return _mm_or_ps( _mm_andnot_ps( msk, a ), _mm_and_ps( b, msk ) );
}
	#define vSelectEqual(x,a,b,c,d) {(x).xxx=_mm_sel_ps((a).xxx,(b).xxx,_mm_cmpeq_ps( (c).xxx, (d).xxx )); }
	#define vSelectGreater(x,a,b,c,d) {(x).xxx=_mm_sel_ps((a).xxx,(b).xxx,_mm_cmpgt_ps( (c).xxx, (d).xxx )); }
	#define vShiftLeftgib_int(x,y,z) {(x).xxx=_mm_srli_si128((y).xxx,(z)/8);}
#ifndef WIN32
	#define vSelectGreater_int(x,a,b,c,d) {(x).xxx=(__m128i)_mm_sel_ps((__m128)(a).xxx,(__m128)(b).xxx,(__m128)_mm_cmpgt_epi32( (c).xxx, (d).xxx )); }
	#define vShiftRightib(x,y,z) {(x).xxx=_mm_srli_epi32((__m128i)((y).xxx),z);}   /* i f ib */
#else
	#define vSelectGreater_int(x,a,b,c,d) {(x).xxx=_mm_castps_si128(_mm_sel_ps(_mm_castsi128_ps((a).xxx),_mm_castsi128_ps((b).xxx),_mm_castsi128_ps(_mm_cmpgt_epi32( (c).xxx, (d).xxx )))); }
	#define vShiftRightib(x,y,z) {(x).xxx=_mm_srli_epi32(_mm_castps_si128((y).xxx),z);}   /* i f ib */
#endif

	//Double
	#define vZero_double(x) { (x).xxx=_mm_setzero_pd();(x).yyy=_mm_setzero_pd();}
	#define vConstant_double(x,c) {(x).xxx=_mm_set_pd1(c);(x).yyy=_mm_set_pd1(c); /*c,c,c,c*/}
	#define vVector_double(x,a,b,c,d) {(x).xxx=_mm_setr_pd(a,b);(x).yyy=_mm_setr_pd(c,d);}
	#define vMin_double(x,a,b) {(x).xxx=_mm_min_pd((a).xxx,(b).xxx);(x).yyy=_mm_min_pd((a).yyy,(b).yyy);}
	#define vMax_double(x,a,b) {(x).xxx=_mm_max_pd((a).xxx,(b).xxx);(x).yyy=_mm_max_pd((a).yyy,(b).yyy);}
	#define vEqual_double(a,b)	(_mm_movemask_pd(_mm_cmpeq_pd( (a).xxx, (b).xxx ))|(_mm_movemask_pd(_mm_cmpeq_pd( (a).yyy, (b).yyy ))<<2))
	#define vGreater_double(a,b)	(_mm_movemask_pd(_mm_cmpgt_pd( (a).xxx, (b).xxx ))|(_mm_movemask_pd(_mm_cmpgt_pd( (a).yyy, (b).yyy ))<<2))
	#define vLess_double(a,b)	(_mm_movemask_pd(_mm_cmplt_pd( (a).xxx, (b).xxx ))|(_mm_movemask_pd(_mm_cmplt_pd( (a).yyy, (b).yyy ))<<2))
	#define vGreaterEqual_double(a,b)	(_mm_movemask_pd(_mm_cmpge_pd( (a).xxx, (b).xxx ))|(_mm_movemask_pd(_mm_cmpge_pd( (a).yyy, (b).yyy ))<<2))
	#define vLessEqual_double(a,b)	(_mm_movemask_pd(_mm_cmple_pd( (a).xxx, (b).xxx ))|(_mm_movemask_pd(_mm_cmple_pd( (a).yyy, (b).yyy ))<<2))
	#define vAbs_double(x,a) {(x).xxx=_mm_max_pd((a).xxx,_mm_sub_pd( _mm_set_pd1(0), (a).xxx ));(x).yyy=_mm_max_pd((a).yyy,_mm_sub_pd( _mm_set_pd1(0), (a).yyy ));}
	#define vAdd_double(x,a,b) {(x).xxx=_mm_add_pd( (a).xxx, (b).xxx );(x).yyy=_mm_add_pd( (a).yyy, (b).yyy );}
	#define vAddib_double(x,a,b) {(x).xxx=_mm_add_pd( (a).xxx, _mm_set_pd1((double)(b)));(x).yyy=_mm_add_pd( (a).yyy, _mm_set_pd1((double)(b)));}
	#define vSubtract_double(x,a,b) {(x).xxx=_mm_sub_pd( (a).xxx, (b).xxx );(x).yyy=_mm_sub_pd( (a).yyy, (b).yyy );}
	#define vMultiply_double(x,a,b) {(x).xxx=_mm_mul_pd( (a).xxx, (b).xxx );(x).yyy=_mm_mul_pd( (a).yyy, (b).yyy );}
	#define vDivide_double(x,a,b) {(x).xxx=_mm_div_pd( (a).xxx, (b).xxx );(x).yyy=_mm_div_pd( (a).yyy, (b).yyy );}
	#define vMADD_double(x,a,b,c) {(x).xxx=_mm_add_pd( (c).xxx, _mm_mul_pd( (a).xxx, (b).xxx ) );(x).yyy=_mm_add_pd( (c).yyy, _mm_mul_pd( (a).yyy, (b).yyy ) );}
	#define vSquareRoot_double(x,a) {(x).xxx=_mm_sqrt_pd( (a).xxx );(x).yyy=_mm_sqrt_pd( (a).yyy );}
	#define vReciprocal_double(x,a) {(x).xxx=_mm_div_pd(_mm_set_pd1((double)(1)), (a).xxx );(x).yyy=_mm_div_pd(_mm_set_pd1((double)(1)), (a).yyy );}
	#define vReciprocalSquareRoot_double(x,a) {(x).xxx=_mm_div_pd(_mm_set_pd1((double)(1)), _mm_sqrt_pd( (a).xxx ) );(x).yyy=_mm_div_pd(_mm_set_pd1((double)(1)), _mm_sqrt_pd( (a).yyy ) );}
	#define vSplat_double(x, v, i ) { (x).yyy=(x).xxx=_mm_set_pd1(v.a[i]); }
	//Int
	#define vZero_int(x)            { (x).xxx=_mm_setzero_si128();}
	#define vAdd_int(x,a,b) {(x).xxx=_mm_add_epi32( (a).xxx, (b).xxx );}
	#define vSubtract_int(x,a,b) {(x).xxx=_mm_sub_epi32( (a).xxx, (b).xxx );}
	#define vAbs_int(x,a) {__m128i tt = _mm_srai_epi32((a).xxx,31);(x).xxx= _mm_sub_epi32(_mm_xor_si128( (a).xxx, tt), tt);}
	#define vAnd_int(x,a,b) {(x).xxx=_mm_and_si128( (a).xxx, (b).xxx );}
	#define vNand_int(x,a,b) {(x).xxx=_mm_xor_si128(_mm_and_si128( (a).xxx, (b).xxx ),_mm_set1_epi32(-1));}
	#define vOr_int(x,a,b) {(x).xxx=_mm_or_si128( (a).xxx, (b).xxx );}
	#define vXor_int(x,a,b) {(x).xxx=_mm_xor_si128( (a).xxx, (b).xxx );}
#ifndef WIN32
	#define vEqual_int(a,b)	(_mm_movemask_ps((__m128)_mm_cmpeq_epi32( (a).xxx, (b).xxx )))
	#define vGreater_int(a,b)	(_mm_movemask_ps((__m128)_mm_cmpgt_epi32( (a).xxx, (b).xxx )))
	#define vLess_int(a,b)	(_mm_movemask_ps((__m128)_mm_cmplt_epi32( (a).xxx, (b).xxx )))
	#define vLessEqual_int(a,b)	(_mm_movemask_ps((__m128)_mm_cmplt_epi32( (a).xxx, (b).xxx ))|_mm_movemask_ps((__m128)_mm_cmpeq_epi32( (a).xxx, (b).xxx )))
	#define vGreaterEqual_int(a,b)	(_mm_movemask_ps((__m128)_mm_cmpgt_epi32( (a).xxx, (b).xxx ))|_mm_movemask_ps((__m128)_mm_cmpeq_epi32( (a).xxx, (b).xxx )))
#else
	#define vEqual_int(a,b)	(_mm_movemask_ps(_mm_castsi128_ps(_mm_cmpeq_epi32( (a).xxx, (b).xxx ))))
	#define vGreater_int(a,b)	(_mm_movemask_ps(_mm_castsi128_ps(_mm_cmpgt_epi32( (a).xxx, (b).xxx ))))
	#define vLess_int(a,b)	(_mm_movemask_ps(_mm_castsi128_ps(_mm_cmplt_epi32( (a).xxx, (b).xxx ))))
	#define vLessEqual_int(a,b)	(_mm_movemask_ps(_mm_castsi128_ps(_mm_cmplt_epi32( (a).xxx, (b).xxx )))|_mm_movemask_ps(_mm_castsi128_ps(_mm_cmpeq_epi32( (a).xxx, (b).xxx ))))
	#define vGreaterEqual_int(a,b)	(_mm_movemask_ps(_mm_castsi128_ps(_mm_cmpgt_epi32( (a).xxx, (b).xxx )))|_mm_movemask_ps(_mm_castsi128_ps(_mm_cmpeq_epi32( (a).xxx, (b).xxx ))))
#endif
	#define vConstant_int(x,c)		{(x).xxx=_mm_set1_epi32(c); /*c,c,c,c*/}
	#define vAddib_int(x,a,b) {(x).xxx=_mm_add_epi32( (a).xxx, _mm_set1_epi32(b) );}
	#define vAndib_int(x,a,b) {(x).xxx=_mm_and_si128( (a).xxx, _mm_set1_epi32(b) );}
	#define vOrib_int(x,a,b) {(x).xxx=_mm_or_si128( (a).xxx, _mm_set1_epi32(b) );}
	#define vXorib_int(x,a,b) {(x).xxx=_mm_xor_si128( (a).xxx, _mm_set1_epi32(b) );}
	#define vMax_int(x,a,b)	{__m128i tt = _mm_cmpgt_epi32((a).xxx,(b).xxx);(x).xxx= _mm_or_si128( _mm_andnot_si128(tt,(b).xxx),_mm_and_si128(tt,(a).xxx));}
    	#define vMin_int(x,a,b)	{__m128i tt = _mm_cmpgt_epi32((a).xxx,(b).xxx);(x).xxx= _mm_or_si128( _mm_andnot_si128(tt,(a).xxx),_mm_and_si128(tt,(b).xxx));}
    	#define vVector_int(x,a,b,c,d)	{(x).xxx=_mm_setr_epi32(a,b,c,d);}
	#define vSplat_int(x, v, i ) { (x).xxx = _mm_shuffle_epi32( v.xxx, _MM_SHUFFLE(i,i,i,i) ); }
	#define vShuffle1_int(x,a,c,d,e,f) {(x).xxx=_mm_shuffle_epi32((a).xxx,_MM_SHUFFLE(c,d,e,f)); /*R->L 0 1 2 3 [cdef]*/}
	#define vShuffle_int(x,a,b,c,d,e,f) {(x).yyy=_mm_shuffle_ps((a).yyy,(b).yyy,_MM_SHUFFLE(c,d,e,f)); /*L->R 0 1 2 3 -- 0 1 2 3 [dcfe]*/}
    	#define vConvert_intto_float(x,y) {(x).xxx=_mm_cvtepi32_ps((y).xxx);}
	#define vConvert_floatto_int(x,y) {(x).xxx=_mm_cvttps_epi32((y).xxx);}
        #define vTrunc(x,y) {(x).xxx=_mm_cvtepi32_ps(_mm_cvttps_epi32((y).xxx));}
	#define vShiftRightib_int(x,y,z) {(x).xxx=_mm_srli_epi32((y).xxx,z);}
	#define vShiftLeftib_int(x,y,z) {(x).xxx=_mm_slli_epi32((y).xxx,z);}
	//Int16
	#define vZero_int16 vZero_int
	#define vConstant_int16(x,c)	{(x).xxx=_mm_set1_epi16(c); /*c,c,c,c*/}
	#define vVector_int16(x,a,b,c,d,e,f,g,h) {(x).xxx=_mm_setr_epi16(a,b,c,d,e,f,g,h);}
#else
#define tVECTOR "scalar"
#ifndef __SCALAR__
#define __SCALAR__
#endif
// Scalar
#include <math.h>
#include <assert.h>
    // Some scalar equivalents to show what the above vector versions accomplish
    
    // A vector, declared as a struct with 4 scalars
    typedef union
    {
        float               a[4];
	int	   	    b[4];
    } vFloat;
    typedef struct
    {
        int                 a[4];
    } vInt32;
	typedef struct
    {
        signed short        a[8];
    } vInt16;
	typedef struct
    {
        double              a[4];
    } vDouble;
#endif
#ifdef __SCALAR__
  // Splat element i across the whole vector and return it
    #define vSplat(x, v, i )  {x.a[0] = x.a[1] = x.a[2] = x.a[3] =v.a[i];}
#endif
#ifdef __SCALAR__
 // Perform a fused-multiply-add operation on architectures that support it 
 // result = X * Y + Z 
    static inline vFloat vMADDx(vFloat X, vFloat Y, vFloat Z )
    {   vFloat result;
        result.a[0] = X.a[0] * Y.a[0] + Z.a[0];
        result.a[1] = X.a[1] * Y.a[1] + Z.a[1];
		result.a[2] = X.a[2] * Y.a[2] + Z.a[2];
		result.a[3] = X.a[3] * Y.a[3] + Z.a[3];
		return result;
    }
	#define vMADD(x,a,b,c) {x=vMADDx(a,b,c);}
#endif
#ifdef __SCALAR__
	static inline vFloat vMultiplyx(vFloat X, vFloat Y)
    {   vFloat result;
        result.a[0] = X.a[0] * Y.a[0];
        result.a[1] = X.a[1] * Y.a[1];
		result.a[2] = X.a[2] * Y.a[2];
		result.a[3] = X.a[3] * Y.a[3];
		return result;
    }
	#define vMultiply(x,a,b) {x=vMultiplyx(a,b);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vFloat vDividex(vFloat X, vFloat Y)
    {   vFloat result;
        result.a[0] = X.a[0] / Y.a[0];
        result.a[1] = X.a[1] / Y.a[1];
		result.a[2] = X.a[2] / Y.a[2];
		result.a[3] = X.a[3] / Y.a[3];
		return result;
    }
	#define vDivide(x,a,b) {x=vDividex(a,b);}
#endif
#if defined(__SCALAR__)
	static inline vFloat vAddx(vFloat X, vFloat Y)
    {   vFloat result;
        result.a[0] = X.a[0] + Y.a[0];
        result.a[1] = X.a[1] + Y.a[1];
		result.a[2] = X.a[2] + Y.a[2];
		result.a[3] = X.a[3] + Y.a[3];
		return result;
    }
	#define vAdd(x,a,b) {x=vAddx(a,b);}
#endif
#if defined(__SCALAR__)
	static inline vFloat vAddibx(vFloat X, float Y)
    {   vFloat result;
        result.a[0] = X.a[0] + Y;
        result.a[1] = X.a[1] + Y;
		result.a[2] = X.a[2] + Y;
		result.a[3] = X.a[3] + Y;
		return result;
    }
	#define vAddib(x,a,b) {x=vAddibx(a,(float)(b));} /*sign extended -128->+127 */
#endif
#if defined(__SCALAR__)
	static inline vFloat vSubtractx(vFloat X, vFloat Y)
    {   vFloat result; 
        result.a[0] = X.a[0] - Y.a[0];
        result.a[1] = X.a[1] - Y.a[1];
		result.a[2] = X.a[2] - Y.a[2];
		result.a[3] = X.a[3] - Y.a[3];
		return result;
    }
	#define vSubtract(x,a,b) {x=vSubtractx(a,b);}
#endif
#if defined(__SCALAR__)
	static inline vFloat vMaxx(vFloat X, vFloat Y)
    {   vFloat result; 
        result.a[0] = max(X.a[0] , Y.a[0]);
        result.a[1] = max(X.a[1] , Y.a[1]);
		result.a[2] = max(X.a[2] , Y.a[2]);
		result.a[3] = max(X.a[3] , Y.a[3]);
		return result;
    }
	#define vMax(x,a,b) {x=vMaxx(a,b);}
#endif
#if defined(__SCALAR__)
	static inline vFloat vMinx(vFloat X, vFloat Y)
    {   vFloat result;
        result.a[0] = min(X.a[0] , Y.a[0]);
        result.a[1] = min(X.a[1] , Y.a[1]);
		result.a[2] = min(X.a[2] , Y.a[2]);
		result.a[3] = min(X.a[3] , Y.a[3]);
		return result;
    }
	#define vMin(x,a,b) {x=vMinx(a,b);}
#endif
#if defined(__SCALAR__)
	static inline int vEqualx(vFloat X, vFloat Y)
    {   int result;
        result = (X.a[0] == Y.a[0])<<3;
        result += (X.a[1] == Y.a[1])<<2;
		result += (X.a[2] == Y.a[2])<<1;
		result += (X.a[3] == Y.a[3]);
		return result;
    }
	#define vEqual(a,b) vEqualx(a,b)
#endif
#if defined(__SCALAR__)
	static inline int vEqual3x(vFloat X, vFloat Y)
    {   int result;
        result = (X.a[0] == Y.a[0]);
        result &= (X.a[1] == Y.a[1]);
		result &= (X.a[2] == Y.a[2]);
		return result;
    }
	#define vEqual3(a,b) vEqual3x(a,b)
#endif

#if defined(__SCALAR__)
	static inline int vGreaterx(vFloat X, vFloat Y)
    {   int result;
        result = (X.a[0] > Y.a[0])<<3;
        result += (X.a[1] > Y.a[1])<<2;
		result += (X.a[2] > Y.a[2])<<1;
		result += (X.a[3] > Y.a[3]);
		return result;
    }
	#define vGreater(a,b) vGreaterx(a,b)
#endif
#if defined(__SCALAR__)
	static inline int vGreaterEqualx(vFloat X, vFloat Y)
    {   int result;
        result = (X.a[0] >= Y.a[0])<<3;
        result += (X.a[1] >= Y.a[1])<<2;
		result += (X.a[2] >= Y.a[2])<<1;
		result += (X.a[3] >= Y.a[3]);
		return result;
    }
	#define vGreaterEqual(a,b) vGreaterEqualx(a,b)
#endif

#if defined(__SCALAR__)
	static inline int vLessx(vFloat X, vFloat Y)
    {   int result;
        result = (X.a[0] < Y.a[0])<<3;
        result += (X.a[1] < Y.a[1])<<2;
		result += (X.a[2] < Y.a[2])<<1;
		result += (X.a[3] < Y.a[3]);
		return result;
    }
	#define vLess(a,b) vLessx(a,b)
#endif
#if defined(__SCALAR__)
	static inline int vLessEqualx(vFloat X, vFloat Y)
    {   int result;
        result = (X.a[0] <= Y.a[0])<<3;
        result += (X.a[1] <= Y.a[1])<<2;
		result += (X.a[2] <= Y.a[2])<<1;
		result += (X.a[3] <= Y.a[3]);
		return result;
    }
	#define vLessEqual(a,b) vLessEqualx(a,b)
#endif
#if defined(__SCALAR__)
	static inline vFloat vReciprocalx(vFloat X)
    {   vFloat result;
        result.a[0] = 1/X.a[0];
        result.a[1] = 1/X.a[1];
		result.a[2] = 1/X.a[2];
		result.a[3] = 1/X.a[3];
		return result;
    }
	#define vReciprocal(a,b) {a=vReciprocalx(b);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vFloat vSquareRootx(vFloat X)
    {   vFloat result;
        result.a[0] = (float)sqrtf(X.a[0]);
        result.a[1] = (float)sqrtf(X.a[1]);
		result.a[2] = (float)sqrtf(X.a[2]);
		result.a[3] = (float)sqrtf(X.a[3]);
		return result;
    }
	#define vSquareRoot(a,b) {a=vSquareRootx(b);}
#endif
#if defined(__SCALAR__)
	static inline vFloat vReciprocalSquareRootx(vFloat X)
    {   vFloat result;
        result.a[0] = (float)(1.0f/sqrtf(X.a[0]));
        result.a[1] = (float)(1.0f/sqrtf(X.a[1]));
		result.a[2] = (float)(1.0f/sqrtf(X.a[2]));
		result.a[3] = (float)(1.0f/sqrtf(X.a[3]));
		return result;
    }
	#define vReciprocalSquareRoot(a,b) {a=vReciprocalSquareRootx(b);}
#endif
#if defined(__SCALAR__)
	static inline vFloat vAbsx(vFloat X)
    {   vFloat result;
        result.a[0] = fabsf(X.a[0]);
        result.a[1] = fabsf(X.a[1]);
		result.a[2] = fabsf(X.a[2]);
		result.a[3] = fabsf(X.a[3]);
		return result;
    }
	#define vAbs(a,b) {a=vAbsx(b);}
#endif

#if defined(__SCALAR__)||defined(__VEC__)
	static inline vFloat vShuffle1x(vFloat t, int a, int b, int c, int d)
    {   vFloat result;
        result.a[0] = t.a[d];
        result.a[1] = t.a[c];
		result.a[2] = t.a[b];
		result.a[3] = t.a[a];
		return result;
    }
	#define vShuffle1(x,y,a,b,c,d) {x=vShuffle1x(y,a,b,c,d);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vFloat vShufflex(vFloat t, vFloat u, int a, int b, int c, int d)
    {   vFloat result;
        result.a[0] = t.a[b];
        result.a[1] = t.a[a];
		result.a[2] = u.a[d];
		result.a[3] = u.a[c];
		return result;
    }
	#define vShuffle(x,y,z,a,b,c,d) {x=vShufflex(y,z,a,b,c,d);}
#endif
#if defined(__SCALAR__)
	#define vZero(x)  { x.a[0] = x.a[1] = x.a[2] = x.a[3] = 0.0f;}
	#define vConstant(x,y)  { x.a[0] = x.a[1] = x.a[2] = x.a[3] = y;}
	#define vVector(x,aa,b,c,d)  { x.a[0] =(float)(aa); x.a[1] =(float)(b); x.a[2] =(float)(c); x.a[3] = (float)(d);}
	//Int
	#define vSplat_int vSplat
	#define vConstant_int vConstant
	#define vVector_int(x,aa,b,c,d)  { x.a[0] =(int)(aa); x.a[1] =(int)(b); x.a[2] =(int)(c); x.a[3] = (int)(d);}
	#define vZero_int(x)  { x.a[0] = x.a[1] = x.a[2] = x.a[3] = 0;}
	//Int16
	#define vZero_int16(x) { x.a[0] = x.a[1] = x.a[2] = x.a[3] = x.a[4] =x.a[5] =x.a[6] =x.a[7] =0;}
	#define vConstant_int16(x,y)  { x.a[0] = x.a[1] = x.a[2] = x.a[3] = x.a[4] = x.a[5] = x.a[6] = x.a[7] = y;}
	#define vVector_int16(x,aa,b,c,d,e,f,g,h)  { x.a[0] =(short)(aa); x.a[1] =(short)(b); x.a[2] =(short)(c); x.a[3] = (short)(d);\
				x.a[4] =(short)(e); x.a[5] =(short)(f); x.a[6] =(short)(g); x.a[7] = (short)(h);}
#endif
#if defined(__SCALAR__)
	static inline vInt32 vAdd_intx(vInt32 X, vInt32 Y)
    {   vInt32 result;
        result.a[0] = X.a[0] + Y.a[0];
        result.a[1] = X.a[1] + Y.a[1];
		result.a[2] = X.a[2] + Y.a[2];
		result.a[3] = X.a[3] + Y.a[3];
		return result;
    }
	#define vAdd_int(x,a,b) {x=vAdd_intx(a,b);}
#endif
#if defined(__SCALAR__)
	static inline vInt32 vAddib_intx(vInt32 X, int Y)
    {   vInt32 result;
        result.a[0] = X.a[0] + Y;
        result.a[1] = X.a[1] + Y;
		result.a[2] = X.a[2] + Y;
		result.a[3] = X.a[3] + Y;
		return result;
    }
	#define vAddib_int(x,a,b) {x=vAddib_intx(a,b);}
#endif
#if defined(__SCALAR__)
	static inline vInt32 vAnd_intx(vInt32 X, vInt32 Y)
    {   vInt32 result;
        result.a[0] = X.a[0] & Y.a[0];
        result.a[1] = X.a[1] & Y.a[1];
		result.a[2] = X.a[2] & Y.a[2];
		result.a[3] = X.a[3] & Y.a[3];
		return result;
    }
	#define vAnd_int(x,a,b) {x=vAnd_intx(a,b);}
#endif
#if defined(__SCALAR__)
	static inline vInt32 vNand_intx(vInt32 X, vInt32 Y)
    {   vInt32 result;
        result.a[0] = ~(X.a[0] & Y.a[0]);
        result.a[1] = ~(X.a[1] & Y.a[1]);
		result.a[2] = ~(X.a[2] & Y.a[2]);
		result.a[3] = ~(X.a[3] & Y.a[3]);
		return result;
    }
	#define vNand_int(x,a,b) {x=vNand_intx(a,b);}
#endif
#if defined(__SCALAR__)
	static inline vInt32 vAndib_intx(vInt32 X, int Y)
    {   vInt32 result;
        result.a[0] = X.a[0] & Y;
        result.a[1] = X.a[1] & Y;
		result.a[2] = X.a[2] & Y;
		result.a[3] = X.a[3] & Y;
		return result;
    }
	#define vAndib_int(x,a,b) {x=vAndib_intx(a,(int)(b));}
#endif
#if defined(__SCALAR__)
	static inline vInt32 vOr_intx(vInt32 X, vInt32 Y)
    {   vInt32 result;
        result.a[0] = X.a[0] | Y.a[0];
        result.a[1] = X.a[1] | Y.a[1];
		result.a[2] = X.a[2] | Y.a[2];
		result.a[3] = X.a[3] | Y.a[3];
		return result;
    }
	#define vOr_int(x,a,b) {x=vOr_intx(a,b);}
#endif
#if defined(__SCALAR__)
	static inline vInt32 vOrib_intx(vInt32 X, int Y)
    {   vInt32 result;
        result.a[0] = X.a[0] | Y;
        result.a[1] = X.a[1] | Y;
		result.a[2] = X.a[2] | Y;
		result.a[3] = X.a[3] | Y;
		return result;
    }
	#define vOrib_int(x,a,b) {x=vOrib_intx(a,(int)(b));}
#endif
#if defined(__SCALAR__)
	static inline vInt32 vXor_intx(vInt32 X, vInt32 Y)
    {   vInt32 result;
        result.a[0] = X.a[0] ^ Y.a[0];
        result.a[1] = X.a[1] ^ Y.a[1];
		result.a[2] = X.a[2] ^ Y.a[2];
		result.a[3] = X.a[3] ^ Y.a[3];
		return result;
    }
	#define vXor_int(x,a,b) {x=vXor_intx(a,b);}
#endif
#if defined(__SCALAR__)
	static inline vInt32 vXorib_intx(vInt32 X, int Y)
    {   vInt32 result;
        result.a[0] = X.a[0] ^ Y;
        result.a[1] = X.a[1] ^ Y;
		result.a[2] = X.a[2] ^ Y;
		result.a[3] = X.a[3] ^ Y;
		return result;
    }
	#define vXorib_int(x,a,b) {x=vXorib_intx(a,(int)(b));}
#endif
#if defined(__SCALAR__)
	static inline vInt32 vSubtract_intx(vInt32 X, vInt32 Y)
    {   vInt32 result;
        result.a[0] = X.a[0] - Y.a[0];
        result.a[1] = X.a[1] - Y.a[1];
		result.a[2] = X.a[2] - Y.a[2];
		result.a[3] = X.a[3] - Y.a[3];
		return result;
    }
	#define vSubtract_int(x,a,b) {x=vSubtract_intx(a,b);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)||defined(__SPU__)
	static inline vInt32 vMultiply_intx(vInt32 X, vInt32 Y)
    {   vInt32 result;
        result.a[0] = X.a[0] * Y.a[0];
        result.a[1] = X.a[1] * Y.a[1];
		result.a[2] = X.a[2] * Y.a[2];
		result.a[3] = X.a[3] * Y.a[3];
		return result;
    }
	#define vMultiply_int(x,a,b) {x=vMultiply_intx(a,b);}
#endif
#if defined(__SCALAR__)
	#define vMultiplyLow_int(x,a,b) {x=vMultiply_intx(a,b);}
#endif
#if defined(__SSE__)||defined(__SSE2__)||defined(__SSE3__)
	#define vMultiplyLow_int(x,a,b) {x=vMultiply_intx(a.xxx,b.xxx);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vInt32 vDivide_intx(vInt32 X, vInt32 Y)
    {   vInt32 result;
        result.a[0] = X.a[0] / Y.a[0];
        result.a[1] = X.a[1] / Y.a[1];
	result.a[2] = X.a[2] / Y.a[2];
	result.a[3] = X.a[3] / Y.a[3];
	return result;
    }
	#define vDivide_int(x,a,b) {x=vDivide_intx(a,b);}
#endif
#if defined(__SSE__)||defined(__SSE2__)||defined(__SSE3__)
	static inline vInt32 vMultiply_intx(__m128i x, __m128i y)
    {   vInt32 result,X;
		result.xxx=x;  X.xxx=y;
        result.a[0] *= X.a[0];
        result.a[1] *= X.a[1];
		result.a[2] *= X.a[2];
		result.a[3] *= X.a[3];
		return result;
    }
	#define vMultiply_int(x,a,b) {x=vMultiply_intx(a.xxx,b.xxx);}
#endif
#if defined(__SSE__)||defined(__SSE2__)||defined(__SSE3__)
	static inline vInt32 vDivide_intx(__m128i x, __m128i y)
    {   vInt32 result,X;
		result.xxx=x;  X.xxx=y;
        result.a[0] /= X.a[0];
        result.a[1] /= X.a[1];
		result.a[2] /= X.a[2];
		result.a[3] /= X.a[3];
		return result;
    }
	#define vDivide_int(x,a,b) {x=vDivide_intx(a.xxx,b.xxx);}
#endif
#if defined(__SCALAR__)
	static inline vInt32 vMax_intx(vInt32 X, vInt32 Y)
    {   vInt32 result; 
        result.a[0] = max(X.a[0] , Y.a[0]);
        result.a[1] = max(X.a[1] , Y.a[1]);
	result.a[2] = max(X.a[2] , Y.a[2]);
	result.a[3] = max(X.a[3] , Y.a[3]);
	return result;
    }
	#define vMax_int(x,a,b) {x=vMax_intx(a,b);}
#endif
#if defined(__SCALAR__)
	static inline vInt32 vMin_intx(vInt32 X, vInt32 Y)
    {   vInt32 result; 
        result.a[0] = min(X.a[0] , Y.a[0]);
        result.a[1] = min(X.a[1] , Y.a[1]);
		result.a[2] = min(X.a[2] , Y.a[2]);
		result.a[3] = min(X.a[3] , Y.a[3]);
		return result;
    }
	#define vMin_int(x,a,b) {x=vMin_intx(a,b);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
#include <math.h>
	static inline vInt32 vSquareRootintx(vInt32 X)
    {   vInt32 result;
        result.a[0] = (int)sqrtf((float)X.a[0]);
        result.a[1] = (int)sqrtf((float)X.a[1]);
		result.a[2] = (int)sqrtf((float)X.a[2]);
		result.a[3] = (int)sqrtf((float)X.a[3]);
		return result;
    }
	#define vSquareRoot_int(a,b) {a=vSquareRootintx(b);}
#endif
#if defined(__SSE__)||defined(__SSE2__)||defined(__SSE3__)
#include <math.h>
	static inline vInt32 vSquareRootintx(__m128i x)
    {   vInt32 result,X;
		X.xxx=x;
        result.a[0] = (int)sqrtf((float)X.a[0]);
        result.a[1] = (int)sqrtf((float)X.a[1]);
		result.a[2] = (int)sqrtf((float)X.a[2]);
		result.a[3] = (int)sqrtf((float)X.a[3]);
		return result;
    }
	#define vSquareRoot_int(a,b) {a=vSquareRootintx(b.xxx);}
#endif
#if defined(__SCALAR__)
	static inline vInt32 vShiftLeftibintx(vInt32 X, int count)
    {   vInt32 result;
        result.a[0] = X.a[0]<<count;
        result.a[1] = X.a[1]<<count;
		result.a[2] = X.a[2]<<count;
		result.a[3] = X.a[3]<<count;
		return result;
    }
	#define vShiftLeftib_int(a,b,c) {a=vShiftLeftibintx(b,c);}
#endif
#if defined(__SCALAR__)
	static inline vInt32 vShiftRightibintx(vInt32 X, int count)
    {   vInt32 result;
        result.a[0] = X.a[0]>>count;
        result.a[1] = X.a[1]>>count;
	result.a[2] = X.a[2]>>count;
	result.a[3] = X.a[3]>>count;
	return result;
    }
	#define vShiftRightib_int(a,b,c) {a=vShiftRightibintx(b,c);}
#endif

#if defined(__SCALAR__)
	static inline vInt32 vAbsintx(vInt32 X)
    {   vInt32 result;
        result.a[0] = (int)abs(X.a[0]);
        result.a[1] = (int)abs(X.a[1]);
		result.a[2] = (int)abs(X.a[2]);
		result.a[3] = (int)abs(X.a[3]);
		return result;
    }
	#define vAbs_int(a,b) {a=vAbsintx(b);}
#endif
#if defined(__SCALAR__)
	static inline int vEqualix(vInt32 X, vInt32 Y)
    {   int result;
        result = (X.a[0] == Y.a[0])<<3;
        result += (X.a[1] == Y.a[1])<<2;
		result += (X.a[2] == Y.a[2])<<1;
		result += (X.a[3] == Y.a[3]);
		return result;
    }
	#define vEqual_int(a,b) vEqualix(a,b)
#endif
#if defined(__SCALAR__)
	static inline int vGreaterix(vInt32 X, vInt32 Y)
    {   int result;
        result = (X.a[0] > Y.a[0])<<3;
        result += (X.a[1] > Y.a[1])<<2;
		result += (X.a[2] > Y.a[2])<<1;
		result += (X.a[3] > Y.a[3]);
		return result;
    }
	#define vGreater_int(a,b) vGreaterix(a,b)
#endif
#if defined(__SCALAR__)
	static inline int vGreaterEqualix(vInt32 X, vInt32 Y)
    {   int result;
        result = (X.a[0] >= Y.a[0])<<3;
        result += (X.a[1] >= Y.a[1])<<2;
		result += (X.a[2] >= Y.a[2])<<1;
		result += (X.a[3] >= Y.a[3]);
		return result;
    }
	#define vGreaterEqual_int(a,b) vGreaterEqualix(a,b)
#endif

#if defined(__SCALAR__)
	static inline int vLessix(vInt32 X, vInt32 Y)
    {   int result;
        result = (X.a[0] < Y.a[0])<<3;
        result += (X.a[1] < Y.a[1])<<2;
		result += (X.a[2] < Y.a[2])<<1;
		result += (X.a[3] < Y.a[3]);
		return result;
    }
	#define vLess_int(a,b) vLessix(a,b)
#endif
#if defined(__SCALAR__)
	static inline int vLessEqualix(vInt32 X, vInt32 Y)
    {   int result;
        result = (X.a[0] <= Y.a[0])<<3;
        result += (X.a[1] <= Y.a[1])<<2;
		result += (X.a[2] <= Y.a[2])<<1;
		result += (X.a[3] <= Y.a[3]);
		return result;
    }
	#define vLessEqual_int(a,b) vLessEqualix(a,b)
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vInt32 vShuffle1intx(vInt32 t, int a, int b, int c, int d)
    {   vInt32 result;
        result.a[0] = t.a[d];
        result.a[1] = t.a[c];
		result.a[2] = t.a[b];
		result.a[3] = t.a[a];
		return result;
    }
	#define vShuffle1_int(x,y,a,b,c,d) {x=vShuffle1intx(y,a,b,c,d);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vInt32 vShuffleintx(vInt32 t, vInt32 u, int a, int b, int c, int d)
    {   vInt32 result;
        result.a[0] = t.a[b];
        result.a[1] = t.a[a];
	result.a[2] = u.a[d];
	result.a[3] = u.a[c];
	return result;
    }
	#define vShuffle_int(x,y,z,a,b,c,d) {x=vShuffleintx(y,z,a,b,c,d);}
#endif
#if defined(__SCALAR__)
    static inline vFloat vConvertitof(vInt32 X)
    {   vFloat result;
        result.a[0] = (float)(X.a[0]);
        result.a[1] = (float)(X.a[1]);
		result.a[2] = (float)(X.a[2]);
		result.a[3] = (float)(X.a[3]);
		return result;
    }
	#define vConvert_intto_float(a,b) {a=vConvertitof(b);}
#endif
#if defined(__SCALAR__)
	static inline vInt32 vConvertftoi(vFloat X)
    {   vInt32 result;
        result.a[0] = (int)(X.a[0]);
        result.a[1] = (int)(X.a[1]);
		result.a[2] = (int)(X.a[2]);
		result.a[3] = (int)(X.a[3]);
		return result;
    }
	#define vConvert_floatto_int(a,b) {a=vConvertftoi(b);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
    //Double
    // Splat element i across the whole vector and return it
    #define vSplat_double(x, v, i )  {x.a[0] = x.a[1] = x.a[2] = x.a[3] =v.a[i];} 
    static inline vDouble vMADDdx(vDouble X, vDouble Y, vDouble Z )
    {   vDouble result;
        result.a[0] = X.a[0] * Y.a[0] + Z.a[0];
        result.a[1] = X.a[1] * Y.a[1] + Z.a[1];
		result.a[2] = X.a[2] * Y.a[2] + Z.a[2];
		result.a[3] = X.a[3] * Y.a[3] + Z.a[3];
		return result;
    }
	#define vMADD_double(x,a,b,c) {x=vMADDdx(a,b,c);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vDouble vMultiplydx(vDouble X, vDouble Y)
    {   vDouble result;
        result.a[0] = X.a[0] * Y.a[0];
        result.a[1] = X.a[1] * Y.a[1];
		result.a[2] = X.a[2] * Y.a[2];
		result.a[3] = X.a[3] * Y.a[3];
		return result;
    }
	#define vMultiply_double(x,a,b) {x=vMultiplydx(a,b);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vDouble vDividedx(vDouble X, vDouble Y)
    {   vDouble result;
        result.a[0] = X.a[0] / Y.a[0];
        result.a[1] = X.a[1] / Y.a[1];
		result.a[2] = X.a[2] / Y.a[2];
		result.a[3] = X.a[3] / Y.a[3];
		return result;
    }
	#define vDivide_double(x,a,b) {x=vDividedx(a,b);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vDouble vAdddx(vDouble X, vDouble Y)
    {   vDouble result;
        result.a[0] = X.a[0] + Y.a[0];
        result.a[1] = X.a[1] + Y.a[1];
		result.a[2] = X.a[2] + Y.a[2];
		result.a[3] = X.a[3] + Y.a[3];
		return result;
    }
	#define vAdd_double(x,a,b) {x=vAdddx(a,b);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vDouble vAddibdx(vDouble X, double Y)
    {   vDouble result;
        result.a[0] = X.a[0] + Y;
        result.a[1] = X.a[1] + Y;
		result.a[2] = X.a[2] + Y;
		result.a[3] = X.a[3] + Y;
		return result;
    }
	#define vAddib_double(x,a,b) {x=vAddibdx(a,(double)(b));} /*sign extended -128->+127 */
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vDouble vSubtractdx(vDouble X, vDouble Y)
    {   vDouble result; 
        result.a[0] = X.a[0] - Y.a[0];
        result.a[1] = X.a[1] - Y.a[1];
		result.a[2] = X.a[2] - Y.a[2];
		result.a[3] = X.a[3] - Y.a[3];
		return result;
    }
	#define vSubtract_double(x,a,b) {x=vSubtractdx(a,b);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vDouble vMaxdx(vDouble X, vDouble Y)
    {   vDouble result; 
        result.a[0] = max(X.a[0] , Y.a[0]);
        result.a[1] = max(X.a[1] , Y.a[1]);
		result.a[2] = max(X.a[2] , Y.a[2]);
		result.a[3] = max(X.a[3] , Y.a[3]);
		return result;
    }
	#define vMax_double(x,a,b) {x=vMaxdx(a,b);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vDouble vMindx(vDouble X, vDouble Y)
    {   vDouble result;
        result.a[0] = min(X.a[0] , Y.a[0]);
        result.a[1] = min(X.a[1] , Y.a[1]);
		result.a[2] = min(X.a[2] , Y.a[2]);
		result.a[3] = min(X.a[3] , Y.a[3]);
		return result;
    }
	#define vMin_double(x,a,b) {x=vMindx(a,b);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vDouble vReciprocaldx(vDouble X)
    {   vDouble result;
        result.a[0] = 1/X.a[0];
        result.a[1] = 1/X.a[1];
		result.a[2] = 1/X.a[2];
		result.a[3] = 1/X.a[3];
		return result;
    }
	#define vReciprocal_double(a,b) {a=vReciprocaldx(b);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vDouble vSquareRootdx(vDouble X)
    {   vDouble result;
        result.a[0] = sqrt(X.a[0]);
        result.a[1] = sqrt(X.a[1]);
		result.a[2] = sqrt(X.a[2]);
		result.a[3] = sqrt(X.a[3]);
		return result;
    }
	#define vSquareRoot_double(a,b) {a=vSquareRootdx(b);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vDouble vReciprocalSquareRootdx(vDouble X)
    {   vDouble result;
        result.a[0] = 1/sqrt(X.a[0]);
        result.a[1] = 1/sqrt(X.a[1]);
		result.a[2] = 1/sqrt(X.a[2]);
		result.a[3] = 1/sqrt(X.a[3]);
		return result;
    }
	#define vReciprocalSquareRoot_double(a,b) {a=vReciprocalSquareRootdx(b);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vDouble vAbsdbx(vDouble X)
    {   vDouble result;
        result.a[0] = fabs(X.a[0]);
        result.a[1] = fabs(X.a[1]);
		result.a[2] = fabs(X.a[2]);
		result.a[3] = fabs(X.a[3]);
		return result;
    }
	#define vAbs_double(a,b) {a=vAbsdbx(b);}
#endif
#if defined(__VEC__)
	static inline int vEqualdbx(vDouble X, vDouble Y)
    {   int result;
        result = (X.a[0] == Y.a[0])*0xff;
        result += (X.a[1] == Y.a[1])*0xff00;
	result += (X.a[2] == Y.a[2])*0xff0000;
	result += (X.a[3] == Y.a[3])*0xff000000;
	return result;
    }
	#define vEqual_double(a,b) vEqualdbx(a,b)
#endif
#if defined(__VEC__)
	static inline int vGreaterdbx(vDouble X, vDouble Y)
    {   int result;
        result = (X.a[0] > Y.a[0])*0xff;
        result += (X.a[1] > Y.a[1])*0xff00;
	result += (X.a[2] > Y.a[2])*0xff0000;
	result += (X.a[3] > Y.a[3])*0xff000000;
	return result;
    }
	#define vGreater_double(a,b) vGreaterdbx(a,b)
#endif
#if defined(__VEC__)
	static inline int vGreaterEqualdbx(vDouble X, vDouble Y)
    {   int result;
        result = (X.a[0] >= Y.a[0])*0xff;
        result += (X.a[1] >= Y.a[1])*0xff00;
	result += (X.a[2] >= Y.a[2])*0xff0000;
	result += (X.a[3] >= Y.a[3])*0xff000000;
	return result;
    }
	#define vGreaterEqual_double(a,b) vGreaterEqualdbx(a,b)
#endif
#if defined(__VEC__)
	static inline int vLessdbx(vDouble X, vDouble Y)
    {   int result;
        result = (X.a[0] < Y.a[0])*0xff;
        result += (X.a[1] < Y.a[1])*0xff00;
	result += (X.a[2] < Y.a[2])*0xff0000;
	result += (X.a[3] < Y.a[3])*0xff000000;
	return result;
    }
	#define vLess_double(a,b) vLessdbx(a,b)
#endif
#if defined(__VEC__)
	static inline int vLessEqualdbx(vDouble X, vDouble Y)
    {   int result;
        result = (X.a[0] <= Y.a[0])*0xff;
        result += (X.a[1] <= Y.a[1])*0xff00;
	result += (X.a[2] <= Y.a[2])*0xff0000;
	result += (X.a[3] <= Y.a[3])*0xff000000;
	return result;
    }
	#define vLessEqual_double(a,b) vLessEqualdbx(a,b)
#endif
#if defined(__SCALAR__)||defined(__SPU__)
	static inline int vEqualdbx(vDouble X, vDouble Y)
    {   int result;
        result = (X.a[0] == Y.a[0])<<3;
        result += (X.a[1] == Y.a[1])<<2;
		result += (X.a[2] == Y.a[2])<<1;
		result += (X.a[3] == Y.a[3]);
		return result;
    }
	#define vEqual_double(a,b) vEqualdbx(a,b)
#endif
#if defined(__SCALAR__)||defined(__SPU__)
	static inline int vGreaterdbx(vDouble X, vDouble Y)
    {   int result;
        result = (X.a[0] > Y.a[0])<<3;
        result += (X.a[1] > Y.a[1])<<2;
		result += (X.a[2] > Y.a[2])<<1;
		result += (X.a[3] > Y.a[3]);
		return result;
    }
	#define vGreater_double(a,b) vGreaterdbx(a,b)
#endif
#if defined(__SCALAR__)||defined(__SPU__)
	static inline int vGreaterEqualdbx(vDouble X, vDouble Y)
    {   int result;
        result = (X.a[0] >= Y.a[0])<<3;
        result += (X.a[1] >= Y.a[1])<<2;
		result += (X.a[2] >= Y.a[2])<<1;
		result += (X.a[3] >= Y.a[3]);
		return result;
    }
	#define vGreaterEqual_double(a,b) vGreaterEqualdbx(a,b)
#endif

#if defined(__SCALAR__)||defined(__SPU__)
	static inline int vLessdbx(vDouble X, vDouble Y)
    {   int result;
        result = (X.a[0] < Y.a[0])<<3;
        result += (X.a[1] < Y.a[1])<<2;
		result += (X.a[2] < Y.a[2])<<1;
		result += (X.a[3] < Y.a[3]);
		return result;
    }
	#define vLess_double(a,b) vLessdbx(a,b)
#endif
#if defined(__SCALAR__)||defined(__SPU__)
	static inline int vLessEqualdbx(vDouble X, vDouble Y)
    {   int result;
        result = (X.a[0] <= Y.a[0])<<3;
        result += (X.a[1] <= Y.a[1])<<2;
	result += (X.a[2] <= Y.a[2])<<1;
	result += (X.a[3] <= Y.a[3]);
	return result;
    }
	#define vLessEqual_double(a,b) vLessEqualdbx(a,b)
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vDouble vShuffle1dx(vDouble t, int a, int b, int c, int d)
    {   vDouble result;
        result.a[0] = t.a[d];
        result.a[1] = t.a[c];
	result.a[2] = t.a[b];
	result.a[3] = t.a[a];
	return result;
    }
	#define vShuffle1_double(x,y,a,b,c,d) {x=vShuffle1dx(y,a,b,c,d);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vDouble vShuffledx(vDouble t, vDouble u, int a, int b, int c, int d)
    {   vDouble result;
        result.a[0] = t.a[b];
        result.a[1] = t.a[a];
	result.a[2] = u.a[d];
	result.a[3] = u.a[c];
	return result;
    }
	#define vShuffle_double(x,y,z,a,b,c,d) {x=vShuffledx(y,z,a,b,c,d);}
#endif
#if defined(__SSE__)||defined(__SSE2__)||defined(__SSE3__)
	static inline vDouble vShuffle1dx(__m128d x, __m128d y, int a, int b, int c, int d)
    {   vDouble result,t;
	    t.xxx=x;  t.yyy=y;
        result.a[0] = t.a[d];
        result.a[1] = t.a[c];
		result.a[2] = t.a[b];
		result.a[3] = t.a[a];
		return result;
    }
	#define vShuffle1_double(x,y,a,b,c,d) {x=vShuffle1dx(y.xxx,y.yyy,a,b,c,d);}
#endif
#if defined(__SSE__)||defined(__SSE2__)||defined(__SSE3__)
	static inline __m128d vShuffledx(__m128d w, __m128d x, int a, int b)
    {   vDouble result,t;
		t.xxx=w; t.yyy=x;
        result.a[0] = t.a[b];
        result.a[1] = t.a[a];
		return result.xxx;
    }
	static inline __m128d vShuff2edx(__m128d y, __m128d z, int c, int d)
    {   vDouble result,u;
		u.xxx=y;  u.yyy=z;
		result.a[0] = u.a[d];
		result.a[1] = u.a[c];
		return result.xxx;
    }
	#define vShuffle_double(x,y,z,a,b,c,d) {x.xxx=vShuffledx(y.xxx,y.yyy,a,b);x.yyy=vShuff2edx(z.xxx,z.yyy,c,d);}
#endif
#if defined(__SCALAR__)
	#define vZero_double(x)  { x.a[0] = x.a[1] = x.a[2] = x.a[3] = 0.0;}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	#define vConstant_double(x,y)  { x.a[0] = x.a[1] = x.a[2] = x.a[3] = y;}
	#define vVector_double(x,aa,b,c,d)  { x.a[0] =aa; x.a[1] =b; x.a[2] =c; x.a[3] = d;}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vDouble vConvertitod(vInt32 X)
    {   vDouble result;
        result.a[0] = (double)(X.a[0]);
        result.a[1] = (double)(X.a[1]);
	result.a[2] = (double)(X.a[2]);
	result.a[3] = (double)(X.a[3]);
	return result;
    }
	#define vConvert_intto_double(a,b) {a=vConvertitod(b);}
#endif
#if defined(__SSE__)||defined(__SSE2__)||defined(__SSE3__)
	static inline vDouble vConvertitod(__m128i x)
    {   vDouble result; vInt32 X;
		X.xxx=x;
        result.a[0] = (double)(X.a[0]);
        result.a[1] = (double)(X.a[1]);
	result.a[2] = (double)(X.a[2]);
	result.a[3] = (double)(X.a[3]);
	return result;
    }
	#define vConvert_intto_double(a,b) {a=vConvertitod(b.xxx);}
#endif
#if defined(__SPU__)
	static inline vDouble vConvertitod(vInt32 X)
    {   vDouble result;
        register vInt32 r2, r5, r6, r7, r8;  register vector double d,e;
        r2.xxx=spu_xor(X.xxx,0x80000000);
	r5.xxx=(vector signed int)spu_cntlz(r2.xxx);
        r6.xxx=spu_sl(r2.xxx,(vector unsigned int)r5.xxx);
        r7.xxx=(vector signed int)spu_cmpeq(r5.xxx,32);
        r8.xxx=spu_sub(0x41e,r5.xxx);
	r6.xxx=spu_add(r6.xxx,r6.xxx);
	r8.xxx=spu_andc(r8.xxx,r7.xxx);
	d=(vector double)spu_shuffle(r8.xxx,r6.xxx,((vector unsigned char){2,3,16,17,18,19,128,128,6,7,20,21,22,23,128,128}));
        d=spu_slqw(d,4);
	e=spu_splats(32768.0*32768.0*2.0);
        result.xxx=spu_sub(d,e);
	d=(vector double)spu_shuffle(r8.xxx,r6.xxx,((vector unsigned char){10,11,24,25,26,27,128,128,14,15,28,29,30,31,128,128}));
        d=spu_slqw(d,4);
        result.yyy=spu_sub(d,e);
	return result;
    }
	#define vConvert_intto_double(a,b) {a=vConvertitod(b);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)||defined(__SPU__)
	static inline vInt32 vConvertdtoi(vDouble X)
    {   vInt32 result;
        result.a[0] = (int)(X.a[0]);
        result.a[1] = (int)(X.a[1]);
	result.a[2] = (int)(X.a[2]);
	result.a[3] = (int)(X.a[3]);
	return result;
    }
	#define vConvert_doubleto_int(a,b) {a=vConvertdtoi(b);}
#endif
#if defined(__SSE__)||defined(__SSE2__)||defined(__SSE3__)
	static inline vInt32 vConvertdtoi(__m128d x,__m128d y)
    {   vInt32 result; vDouble X;
		X.xxx=x; X.yyy=y;
        result.a[0] = (int)(X.a[0]);
        result.a[1] = (int)(X.a[1]);
	result.a[2] = (int)(X.a[2]);
	result.a[3] = (int)(X.a[3]);
	return result;
    }
	#define vConvert_doubleto_int(a,b) {a=vConvertdtoi(b.xxx,b.yyy);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vDouble vConvertftod(vFloat X)
    {   vDouble result;
        result.a[0] = (double)(X.a[0]);
        result.a[1] = (double)(X.a[1]);
		result.a[2] = (double)(X.a[2]);
		result.a[3] = (double)(X.a[3]);
		return result;
    }
	#define vConvert_floatto_double(a,b) {a=vConvertftod(b);}
#endif
#if defined(__SSE__)||defined(__SSE2__)||defined(__SSE3__)
	static inline vDouble vConvertftod(__m128 x)
    {   vDouble result; vFloat X;
		X.xxx=x;
        result.a[0] = (double)(X.a[0]);
        result.a[1] = (double)(X.a[1]);
		result.a[2] = (double)(X.a[2]);
		result.a[3] = (double)(X.a[3]);
		return result;
    }
	#define vConvert_floatto_double(a,b) {a=vConvertftod(b.xxx);}
#endif
#if defined(__SPU__)
#if 0
	static inline vDouble vConvertftod(vFloat X)
    {   vDouble result;
	register vInt32 rexph, rzero, rsign;
        rexph.xxx=(vector signed int)spu_rlmask(X.yyy,-27);
	rzero.xxx=(vector signed int)spu_cmpeq(X.yyy,0);
        rsign.xxx=(vector signed int)spu_rlmask(X.yyy,-31);
	rexph.xxx=spu_and(rexph.xxx,0xf);
        rsign.xxx=spu_sl(rsign.xxx,7);
	rexph.xxx=spu_add(rexph.xxx,0x38);
	rexph.xxx=spu_andc(rexph.xxx,rzero.xxx);
	rexph.xxx=spu_or(rexph.xxx,rsign.xxx);
	rsign.xxx=(vector signed int)spu_sl(X.yyy,5);
	rsign.xxx=spu_andc(rsign.xxx,rzero.xxx);
	result.xxx=(vector double)spu_shuffle(rsign.xxx,rexph.xxx,((vector unsigned char){19,0,1,2,3,128,128,128,23,4,5,6,7,128,128,128}));
	result.yyy=(vector double)spu_shuffle(rsign.xxx,rexph.xxx,((vector unsigned char){27,8,9,10,11,128,128,128,31,12,13,14,15,128,128,128}));
	return result;
    }
#endif
	static inline vDouble vConvertftod(vFloat X)
    {   vDouble result;
	register vFloat y;
        y.xxx=spu_shuffle(X.xxx,X.xxx,((vector unsigned char){0,1,2,3,128,128,128,128,4,5,6,7,128,128,128,128}));
        result.xxx=spu_extend(y.xxx);
        y.xxx=spu_shuffle(X.xxx,X.xxx,((vector unsigned char){8,9,10,11,128,128,128,128,12,13,14,15,128,128,128,128}));
        result.yyy=spu_extend(y.xxx);
        return result;
    }
	#define vConvert_floatto_double(a,b) {a=vConvertftod(b);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)
	static inline vFloat vConvertdtof(vDouble X)
    {   vFloat result;
        result.a[0] = (float)(X.a[0]);
        result.a[1] = (float)(X.a[1]);
	result.a[2] = (float)(X.a[2]);
	result.a[3] = (float)(X.a[3]);
	return result;
    }
	#define vConvert_doubleto_float(a,b) {a=vConvertdtof(b);}
#endif
#if defined(__SSE__)||defined(__SSE2__)||defined(__SSE3__)
	static inline vFloat vConvertdtof(__m128d x, __m128d y)
    {   vFloat result; vDouble X;
		X.xxx=x; X.yyy=y;
        result.a[0] = (float)(X.a[0]);
        result.a[1] = (float)(X.a[1]);
	result.a[2] = (float)(X.a[2]);
	result.a[3] = (float)(X.a[3]);
	return result;
    }
	#define vConvert_doubleto_float(a,b) {a=vConvertdtof(b.xxx,b.yyy);}
#endif
#if defined(__SPU__)
	static inline vFloat vConvertdtof(vDouble X)
    {   register vFloat result, temp;
        result.xxx=spu_roundtf(X.xxx);
	temp.xxx=spu_roundtf(X.yyy);
	result.xxx=spu_shuffle(result.xxx,temp.xxx,((vector unsigned char){0,1,2,3,8,9,10,11,16,17,18,19,24,25,26,27}));
	return result;
    }
	#define vConvert_doubleto_float(a,b) {a=vConvertdtof(b);}
#endif
#if defined(__SCALAR__)||defined(__SSE__)||defined(__SSE2__)||defined(__SSE3__)||defined(__VEC__)||defined(__SPU__)
	#define vExtract(aa,b) ((aa).a[b])
#endif
#if defined(__SCALAR__)||defined(__SSE__)||defined(__SSE2__)||defined(__SSE3__)||defined(__VEC__)||defined(__SPU__)
	#define vInsert(p,m,n) {(p).a[m]=n;}
#endif
#if defined(__SCALAR__)||defined(__SSE__)||defined(__SSE2__)||defined(__SSE3__)||defined(__VEC__)||defined(__SPU__)
	#define vExtract_int vExtract
#endif
#if defined(__SCALAR__)||defined(__SSE__)||defined(__SSE2__)||defined(__SSE3__)||defined(__VEC__)||defined(__SPU__)
	#define vInsert_int vInsert
#endif
#if defined(__SCALAR__)||defined(__SSE__)||defined(__SSE2__)||defined(__SSE3__)||defined(__VEC__)||defined(__SPU__)
	#define vExtract_double vExtract
#endif
#if defined(__SCALAR__)||defined(__SSE__)||defined(__SSE2__)||defined(__SSE3__)||defined(__VEC__)||defined(__SPU__)
	#define vInsert_double vInsert
#endif
#if defined(__SCALAR__)||defined(__SSE__)||defined(__SSE2__)||defined(__SSE3__)||defined(__VEC__)||defined(__SPU__)
	#define vInsert_int16 vInsert
#endif
#if defined(__SCALAR__)||defined(__SSE__)||defined(__SSE2__)||defined(__SSE3__)||defined(__VEC__)||defined(__SPU__)
	#define vExtract_int16 vExtract
#endif
#if defined(__SCALAR__)||defined(__VEC__)||defined(__SPU__)
static inline vFloat vPromotex(vFloat y, int i) {
	vFloat x;
	x.a[0]=y.a[i];
	return x;
}
#define vPromote(x,y,i) {x=vPromotex(y,i);}
#endif
static inline vFloat vPromoteix(int f) {
	vFloat x;
	x.a[PSLOT]=(float)f;
	return x;
}
#define vPromote_int(x,y) {x=vPromoteix((int)(y));}
#if defined(__SSE__)||defined(__SSE2__)||defined(__SSE3__)
static inline vFloat vPromotex(__m128 y, int i) {
	vFloat x,yy;
	yy.xxx=y;
	x.a[0]=yy.a[i];
	return x;
}
#define vPromote(x,y,i) {x=vPromotex(y.xxx,i);}
#endif
#define vGather(x,y,z) {(x).a[0]=(y)[(int)(z).a[0]]; (x).a[1]=(y)[(int)(z).a[1]]; (x).a[2]=(y)[(int)(z).a[2]]; (x).a[3]=(y)[(int)(z).a[3]];}
#define vScatter(y,z,x) {(y)[(int)(z).a[0]]=(x).a[0]; (y)[(int)(z).a[1]]=(x).a[1]; (y)[(int)(z).a[2]]=(x).a[2]; (y)[(int)(z).a[3]]=(x).a[3]; }
#if defined(__SCALAR__)
static inline vFloat vSelectEqualx(vFloat tru, vFloat fls, vFloat left, vFloat right) {
	vFloat x;
	x.a[0]=(left.a[0]==right.a[0]) ? tru.a[0] : fls.a[0];
	x.a[1]=(left.a[1]==right.a[1]) ? tru.a[1] : fls.a[1];
	x.a[2]=(left.a[2]==right.a[2]) ? tru.a[2] : fls.a[2];
	x.a[3]=(left.a[3]==right.a[3]) ? tru.a[3] : fls.a[3];
	return x;
}
#define vSelectEqual(x,a,b,c,d) {x=vSelectEqualx(a,b,c,d);}
static inline vFloat vSelectGreaterx(vFloat tru, vFloat fls, vFloat left, vFloat right) {
	vFloat x;
	x.a[0]=(left.a[0]>right.a[0]) ? tru.a[0] : fls.a[0];
	x.a[1]=(left.a[1]>right.a[1]) ? tru.a[1] : fls.a[1];
	x.a[2]=(left.a[2]>right.a[2]) ? tru.a[2] : fls.a[2];
	x.a[3]=(left.a[3]>right.a[3]) ? tru.a[3] : fls.a[3];
	return x;
}
#define vSelectGreater(x,a,b,c,d) {x=vSelectGreaterx(a,b,c,d);}
static inline vInt32 vSelectGreaterix(vInt32 tru, vInt32 fls, vInt32 left, vInt32 right) {
	vInt32 x;
	x.a[0]=(left.a[0]>right.a[0]) ? tru.a[0] : fls.a[0];
	x.a[1]=(left.a[1]>right.a[1]) ? tru.a[1] : fls.a[1];
	x.a[2]=(left.a[2]>right.a[2]) ? tru.a[2] : fls.a[2];
	x.a[3]=(left.a[3]>right.a[3]) ? tru.a[3] : fls.a[3];
	return x;
}
#define vSelectGreater_int(x,a,b,c,d) {x=vSelectGreaterix(a,b,c,d);}
#endif
#if defined(__SCALAR__)
static inline vFloat vAndintx(vFloat X, vInt32 mask)
    {   vFloat result;
	result.b[0] = X.b[0]&mask.a[0];
	result.b[1] = X.b[1]&mask.a[1];
	result.b[2] = X.b[2]&mask.a[2];
	result.b[3] = X.b[3]&mask.a[3];
	return result;
    }
#define vAndint(a,b,c) {a=vAndintx(b,c);}
#endif
#if defined(__SCALAR__)
static inline vInt32 vShiftRightibx(vFloat X, const int count)
    {   vInt32 result;
        result.a[0] = X.b[0]>>count;
	result.a[1] = X.b[1]>>count;
	result.a[2] = X.b[2]>>count;
	result.a[3] = X.b[3]>>count;
	return result;
    }
#define vShiftRightib(x,b,c) {x=vShiftRightibx(b,c);}
static inline vInt32 vShiftLeftgibx(vInt32 X, const int count)
/*7       6       3       0
  4       1       6       3 vMin8 gcc ps3 sl32 <-this results in 7 vs 1 in slot 0, only with -O3??
36      5       46      29 13   57      24      95 7
*/
    {   vInt32 result;
        if (count==64) {
	  result.a[0] = X.a[2];
	  result.a[1] = X.a[3];
	  result.a[2] = 0;
	  result.a[3] = 0;
        } else if (count==32) {
	  result.a[0] = X.a[1];
	  result.a[1] = X.a[2];
	  result.a[2] = X.a[3];
	  result.a[3] = 0;
        } else if (count<32) {
	  result.a[0] = (X.a[0]<<count)|(((unsigned int)X.a[1])>>(32-count));
	  result.a[1] = (X.a[1]<<count)|(((unsigned int)X.a[2])>>(32-count));
	  result.a[2] = (X.a[2]<<count)|(((unsigned int)X.a[3])>>(32-count));
	  result.a[3] = (X.a[3]<<count);
	} else assert(0);
	return result;
    }
#define vShiftLeftgib_int(x,b,c) {x=vShiftLeftgibx(b,c);}
#endif
#if defined(__SCALAR__)
static inline vFloat vTruncx(vFloat X)
    {   vFloat result;
	result.a[0] = (float)((int)X.a[0]);
	result.a[1] = (float)((int)X.a[1]);
	result.a[2] = (float)((int)X.a[2]);
	result.a[3] = (float)((int)X.a[3]);
	return result;
    }
#define vTrunc(a,b) {a=vTruncx(b);}
#endif
#if defined(__SCALAR__)||defined(__VEC__)||defined(__SPU__)
#define vCopy(x,y) {x=y;}
#endif
#if defined(__SSE__)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
static char * vTostringx(const float a[4]) {
	char *s=(char *)malloc(64);
	sprintf(s,"%g\t%g\t%g\t%g",a[0],a[1],a[2],a[3]);
	return s;
}
#define vTostring(x) vTostringx((x).a)
static char * vTostringix(const int a[4]) {
	char *s=(char *)malloc(64);
	sprintf(s,"%d\t%d\t%d\t%d",a[0],a[1],a[2],a[3]);
	return s;
}
#define vTostring_int(x) vTostringix((x).a)
#endif
#if defined(__SCALAR__)||defined(__VEC__)||defined(__SPU__)
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
static char * vTostring(const vFloat m) {
	char *s=(char *)malloc(64); char t[16]; int i;
	*s=0;
	for (i=0; i<4; i++) {
		sprintf(t,"%g",vExtract(m,i));
		if (i!=3) strcat(t,"\t");
		strcat(s,t);
	}
	return s;
}
static char * vTostring_int(const vInt32 m) {
	char *s=(char *)malloc(64); char t[16]; int i;
	*s=0;
	for (i=0; i<4; i++) {
		sprintf(t,"%ld",vExtract(m,i));
		if (i!=3) strcat(t,"\t");
		strcat(s,t);
	}
	return s;
}
static char * vTostring_int16(const vInt16 m) {
	char *s=(char *)malloc(80); char t[16]; int i;
	*s=0;
	for (i=0; i<8; i++) {
		sprintf(t,"%d",vExtract(m,i));
		if (i!=7) strcat(t,"\t");
		strcat(s,t);
	}
	return s;
}
static char * vTostring_double(const vDouble m) {
	char *s=(char *)malloc(64); char t[16]; int i;
	*s=0;
	for (i=0; i<4; i++) {
		sprintf(t,"%lg",vExtract(m,i));
		if (i!=3) strcat(t,"\t");
		strcat(s,t);
	}
	return s;
}
#endif
#endif  /*__VECTR__*/

