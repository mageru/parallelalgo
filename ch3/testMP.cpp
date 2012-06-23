#include <stdio.h>
#include "omp.h"
#ifdef WIN32
#include "stdafx.h"
int _tmain(int argc, _TCHAR* argv[]) {
#else
int main (int argc, char *argv[]) {
#endif
    int rvalue = 0;
#ifdef _OPENMP
    rvalue = 1;
#endif
    printf("_OPENMP %s\n", rvalue ?"passed" :"failed");
    return 0;
}
