//http://www.ibm.com/developerworks/linux/library/l-shared/index.html
#include "common.h"

#include <iostream>
#include <stdlib.h>

int main (int argc, char * argv[])
{
    int jumpTo = 0;

    if (1 < argc) {
        jumpTo = strtol(argv[1], NULL, 10);
    }

    if ((1 > jumpTo) || (6 < jumpTo)) {
        jumpTo = 1;
    }

    A * pA;
    B * pB;
    C * pC;

    GetObjects(&pA, &pB, &pC);

    cout << (int)pA << "\t";
    cout << (int)pB << "\t";
    cout << (int)pC << "\n";

    switch (jumpTo) {
    case 1:
        cout << pA->m_nA << endl;

    case 2:
        pA->WhoAmI();

    case 3:
        cout << pB->m_nA << endl;

    case 4:
        pB->WhoAmI();

    case 5:
        cout << pC->m_nA << endl;

    case 6:
        pC->WhoAmI();
    }

    return 0;
}

#include <pthread.h>

void DoNothingCode() {
    pthread_create(NULL, NULL, NULL, NULL);
}
