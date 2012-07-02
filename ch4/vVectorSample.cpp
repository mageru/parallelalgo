#include <stdlib.h>
#include <stdio.h>
#define __SSE__
#include "vVector.h"
//for the Kindle
#define vp vTostring

int main( void ) {
vFloat r, x,y,z;
float a[6]={1,2,3,4,5,6};
printf("running in %s mode\n", tVECTOR);
vConstant(x,9.f);
printf("vConstant=%s\n", vp(x));
vZero(x);
printf("vZero=%s\n", vp(x));
vVector(x,1.f,-2.f,3.f,-4.f);
printf("vVector=%s\n", vp(x));
vAbs(y,x);
printf("vAbs(%s)=%s\n", vp(x), vp(y));
vCopy(x,y);
printf("vCopy(x,%s)=%s\n", vp(y), vp(x));
vVector(y,4.f,3.f,2.f,1.f);
vAdd(z,x,y);
printf("vAdd(%s :: %s)=%s\n", vp(x), vp(y), vp(z));
vAddib(z,x,36.f);
printf("vAddib(%s :: %g)=%s\n", vp(x), 36.f, vp(z));
vConstant(y,1);
vSubtract(z,x,y);
printf("vSubtract(%s :: %s)=%s\n", vp(x), vp(x), vp(z));
vConstant(y,3);
vMultiply(z,x,y);
printf("vMultiply(%s :: %s)=%s\n", vp(x), vp(x), vp(z));
vConstant(y,5);
vDivide(z,x,y);
printf("vDivide(%s :: %s)=%s\n", vp(x), vp(x), vp(z));
vReciprocal(z,x);
printf("vReciprocal(%s)=%s\n", vp(x), vp(z));
vReciprocalSquareRoot(z,x);
printf("vReciprocalSquareRoot(%s)=%s\n", vp(x), vp(z));
vSquareRoot(z,x);
printf("vSquareRoot(%s)=%s\n", vp(x), vp(z));
vConstant(y,5.f);
vVector(z,1.f,2.f,3.f,14.f);
vMADD(r,x,y,z);
printf("vMultiplyAdd(%s :: %s :: %s)=%s\n",vp(x),vp(y),vp(z),vp(r));
vMax(y,x,z);
printf("vMax=(%s :: %s)%s\n", vp(x),vp(z),vp(y));
vMin(y,x,z);
printf("vMin=(%s :: %s)%s\n", vp(x),vp(z),vp(y));
vShuffle1(z,x,2,3,0,1);
printf("vShuffle1(%s :: 2 3 0 1)=%s\n", vp(x),vp(z));
vSplat(z,x,2);
printf("vSplat(%s :: 2)=%s\n", vp(x),vp(z));
vVector(y,8.f,9.f,10.f,11.f);
vShuffle(x,y,z,3,0,2,1);
printf("vShuffle(%s :: %s :: 3 0 2 1)=%s\n", vp(y),vp(z),vp(x));
printf("vGreaterEqual(%s :: %s)=%d\n", vp(y),vp(z),vGreaterEqual(y,z));
vSelectGreater(r,x,y,y,z);
printf("vSelectGreater(true=%s :: false=%s :: %s ::%s)= %s\n", vp(x),vp(y),vp(y),vp(z),vp(r));
vPromote(r,x,2);
printf("vPromote(%s :: %d)=%s\n", vp(x), 2, vp(r));
printf("vExtract(%s :: 1)=%g\n", vp(x), vExtract(x,1));
vCopy(r,x);
vInsert(x,2,33.f);
printf("vInsert(%s :: %d :: %g)=%s\n", vp(r), 2, 33.f, vp(x));
vVector(y,5.f,3.f,2.f,0.f);
vGather(r,a,y);
printf("vGather(1 2 3 4 5 6 ::%s)=%s\n", vp(y), vp(r));
vConstant(z,9.f);
vScatter(a,y,z);
printf("vScatter(%s ::%s)=%g %g %g %g %g %g    \n", vp(y), vp(z),a[0],a[1],a[2],a[3],a[4],a[5]);
return 0;
}
