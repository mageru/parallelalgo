// Justin J. Miller
// Final Project - Closest Pair
// CS 7332 - Parallel Algorithm Design
// 7/28/2012
#include <stdio.h>
#include <stdlib.h>
#include <values.h>
#include <math.h>
#include <string.h>
#include <omp.h>
#include <time.h>
#include <pthread.h>
using namespace std;

typedef struct { double x, y; } point_t, *point;

//Initialize a structure to pass variables to threads
typedef struct {
    point* sx;
    int nx;
    point* sy;
    int ny;
    point *a;
    point *b;
} ClosestInput;

// Calculate distance between two points
inline double dist(point a, point b){
    double dx = a->x - b->x, dy = a->y - b->y;

    return dx * dx + dy * dy;
}

// Compare two double values
inline int cmp_dbl(double a, double b){
    return a < b ? -1 : a > b ? 1 : 0;
}

// Compare two X values
int cmp_x(const void *a, const void *b) {
    return cmp_dbl((*((point*)a))->x, (*((point*)b))->x);
}

// Compare two Y values
int cmp_y(const void *a, const void *b) {
    return cmp_dbl((*((point*)a))->y, (*((point*)b))->y);
}

// Brute force algorithm to find closest points from a set of 8
double bruteForce(point* pts, int max_n, point *a, point *b){
    int i, j;
    double d, min_d = MAXDOUBLE;

    for (i = 0; i < max_n; i++) {
        for (j = i + 1; j < max_n; j++) {
            d = dist(pts[i], pts[j]);
            if (d >= min_d) {
                continue;
            }
            *a = pts[i];
            *b = pts[j];
            min_d = d;
        }
    }
    return min_d;
}

// The findClosest algorithm takes a set of params
void *findClosest(void *vptr_value){
    double *ret = (double *)malloc(sizeof(double));

    ClosestInput ci = *(ClosestInput *)vptr_value;
    pthread_t lside, rside;

    int left, right, i;
    double *min_d;
    double *d;
    double x0, x1, mid, x;
    //void *min_d_vp,*d_vp;
    point a1, b1;
    point *sizeOfYy;

    printf("Value of nx: %d ny: %d\n", ci.nx, ci.ny);

    if (ci.nx <= 8) {
        *ret = bruteForce(ci.sx, ci.nx, ci.a, ci.b);
        pthread_exit((void*)ret);
    }
    sizeOfYy = (point*)malloc(sizeof(point) * ci.ny);
    mid = ci.sx[ci.nx / 2]->x;

    /* add points to the y-sorted list; if a point's x is less than mid,
       add to the begining; if more, add to the end backwards, hence the
       need to reverse it */
    left = -1; right = ci.ny;
    for (i = 0; i < ci.ny; i++) {
        printf("Value of sy # %d (x = %f) Mid: %f left: %d right: %d\n", i, ci.sy[i]->x, mid, left, right);
        if (ci.sy[i]->x < mid) {
            sizeOfYy[++left] = ci.sy[i];
        }
        else { sizeOfYy[--right] = ci.sy[i]; }
    }

    /* reverse the higher part of the list */
    for (i = ci.ny - 1; right < i; right++, i--) {
        a1 = sizeOfYy[right]; sizeOfYy[right] = sizeOfYy[i]; sizeOfYy[i] = a1;
    }

    ClosestInput c1, c2;
    c1.sx = ci.sx;
    c1.nx = (ci.nx / 2);
    c1.sy = sizeOfYy;
    c1.ny = left + 1;
    c1.a = ci.a;
    c1.b = ci.b;
    c2.sx = ci.sx + ci.nx / 2;
    c2.nx = (ci.nx - ci.nx / 2);
    c2.sy = sizeOfYy + left + 1;
    c2.ny = ci.ny - left - 1;
    c2.a = &a1;
    c2.b = &b1;

    pthread_create(&lside, NULL, &findClosest, (void *)&c1);
    pthread_join(lside, (void**)&min_d);
    printf("Value of min_d: %f\n", *min_d);
    pthread_create(&rside, NULL, &findClosest, (void *)&c2);
    pthread_join(rside, (void**)&d);
    printf("Value of d: %f\n", *d);

    if (d < (min_d)) {
        min_d = d; *ci.a = a1; *ci.b = b1;
    }
    *d = sqrt(*min_d);

    /*find points distance d from center line*/
    left = -1; right = ci.ny;
    for (i = 0; i < ci.ny; i++) {
        x = ci.sy[i]->x - mid;
        if (x <= -*d || x >= *d) {
            continue;
        }

        if (x < 0) {
            sizeOfYy[++left] = ci.sy[i];
        }
        else { sizeOfYy[--right] = ci.sy[i]; }
    }

    /* compare points from both sides*/
    while (left >= 0) {
        x0 = sizeOfYy[left]->y + *d;

        while (right < ci.ny && sizeOfYy[right]->y > x0) {
            right++;
        }
        if (right >= ci.ny) {
            break;
        }

        x1 = sizeOfYy[left]->y - *d;
        for (i = right; i < ci.ny && sizeOfYy[i]->y > x1; i++) {
            if ((x = dist(sizeOfYy[left], sizeOfYy[i])) < *min_d) {
                *min_d = x;
                *d = sqrt(*min_d);
                *ci.a = sizeOfYy[left];
                *ci.b = sizeOfYy[i];
            }
        }

        left--;
    }

    free(sizeOfYy);
    *ret = *min_d;
    printf("Returning: %f\n", *ret);
    return ret;
}

// Define the number of points you want to be generated
#define NP 100
int main(){
    pthread_t sorter;
    int i;
    double *result;
    point a, b;
    ClosestInput ci;
    printf("Generating: %d  random points between 0 and 1000\n", NP);

    point pts = (point_t*)malloc(sizeof(point_t) * NP);
    point* sizeOfX = (point*)malloc(sizeof(point) * NP);
    point* sizeOfY = (point*)malloc(sizeof(point) * NP);

    // Generate NP number of random points between 0 and 1000
    srand((unsigned)time(NULL));
#pragma omp parallel for
    for (i = 0; i < NP; i++) {
        sizeOfX[i] = pts + i;
        pts[i].x = 1000 * (double)rand() / RAND_MAX;
        pts[i].y = 1000 * (double)rand() / RAND_MAX;
        printf("Point Generated (%d, %d)\n", (int)pts[i].x, (int)pts[i].y);
    }


    memcpy(sizeOfY, sizeOfX, sizeof(point) * NP);
    qsort(sizeOfX, NP, sizeof(point), cmp_x);
    qsort(sizeOfY, NP, sizeof(point), cmp_y);

    // Init object for closest pair thread
    ci.sx = sizeOfX;
    ci.nx = NP;
    ci.sy = sizeOfY;
    ci.ny = NP;
    ci.a = &a;
    ci.b = &b;

    pthread_create(&sorter, NULL, &findClosest, (void *)&ci);
    pthread_join(sorter, (void**)&result);

    printf("Minimum Distance: %g; ", sqrt(*result));
    printf("between points (%ld,%ld) and (%ld,%ld)\n", (long int)a->x, (long int)a->y, (long int)b->x, (long int)b->y);

    return 0;
}
