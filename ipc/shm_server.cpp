//http://www.ibm.com/developerworks/linux/library/l-shared/index.html
 #include <sys/types.h>
 #include <sys/ipc.h>
 #include <sys/shm.h>
 #include <errno.h>
 #include <stdio.h>
 #include <iostream>

 #include "common.h"

 void * A::m_sArena = NULL;

 void * A::operator new (unsigned int size)
 {
     switch (size)
     {
     case sizeof(A):
         return m_sArena;

     case sizeof(B):
         return (void *)((int)m_sArena + 1024);

     case sizeof(C):
         return (void *)((int)m_sArena + 2048);

     default:
         cerr << __FILE__ << ":" << __LINE__ << " Critical error" << endl;
       }
}

void A::WhoAmI() {
    cout << "Object type: A" << endl;
}


void B::WhoAmI() {
    cout << "Object type: B" << endl;
}


void C::WhoAmI() {
    cout << "Object type: C" << endl;
}


void GetObjects(A ** pA, B ** pB, C ** pC) {
    *pA = (A *)A::m_sArena;
    *pB = (B *)((int)A::m_sArena + 1024);
    *pC = (C *)((int)A::m_sArena + 2048);
}


class Initializer {
public:
    int m_shmid;
    Initializer();

    static Initializer m_sInitializer;
};

Initializer Initializer::m_sInitializer;

Initializer::Initializer()
{
 key_t key = 1234;
 bool bCreated = false;

 m_shmid = shmget(key, 3*1024, 0666);

 if (-1 == m_shmid) {

     if (ENOENT != errno) {
         cerr << __FILE__ << ":" << __LINE__ << " Critical error" << endl;
            return;
        }

        m_shmid = shmget(key, 3*1024, IPC_CREAT | 0666);

        if (-1 == m_shmid) {
            cerr << __FILE__ << ":" << __LINE__ << " Critical error" << endl;
            return;
        }

        cout << "Created the shared memory" << endl;
        bCreated = true;
    }

    A::m_sArena = shmat(m_shmid, NULL, 0);

    if (-1 == (int)A::m_sArena) {
        cerr << __FILE__ << ":" << __LINE__ << " Critical error" << endl;
        return;
    }

    if (bCreated) {
        // Construct objects on the shared memory
        A * pA;
        pA = new A;
        pA->m_nA = 1;
        pA = new B;
        pA->m_nA = 2;
        pA = new C;
        pA->m_nA = 3;
    }

    return;
}
