#include <omp.h>
#include <chrono>
#include <iostream>
using namespace std;

const int N = 120000000;

int* a = new int[N];
int* b = new int[N];
int* AA = new int[N];
int* BB = new int[N];

uint32_t state = 777;
char myRand()
{
    state = state * 1664525 + 1013904223;
    return state >> 24;
}

// Сумма значений Max(A[i]+B[i], 4A[i]-B[i]), больших 1
void main()
{
    srand(time(0));
    int i, sum, sum1, sum2, A, B, C;

    // Заполнение массивов
#pragma omp parallel sections shared(a, b, AA, BB) private(i) 
    {
#pragma omp section
        for (i = 0; i < N; i++)
            a[i] = (int)myRand(); //a[i] = rand() % 10 - 5;
#pragma omp section
        for (i = 0; i < N; i++)
            b[i] = (int)myRand(); //b[i] = rand() % 10 - 5;
#pragma omp section
        for (i = 0; i < N; i++)
            AA[i] = 0; 
#pragma omp section
        for (i = 0; i < N; i++)
            BB[i] = 0;
    }

    // Параллельный фрагмент
    sum = 0;
    auto startPar = chrono::system_clock::now();
#pragma omp parallel for shared(a, b, sum) private(i, A, B, C) //reduction(+: sum)
    for (i = 0; i < N; i++) {
        A = a[i] + b[i];
        B = 4 * a[i] - b[i];
        C = max(A, B);
        if (C > 1)
#pragma omp atomic
            sum += C;
    }
    auto stopPar = chrono::system_clock::now();
    cout << "\n  Sum of Parallel: " << sum;

    // Распараллеливание по секциям (1 способ)
    sum1 = 0, sum2 = 0;
    auto startParSec1 = chrono::system_clock::now();
#pragma omp parallel sections shared(a, b, sum1, sum2) private(i, A, B, C)
    {
#pragma omp section
        for (i = 0; i < N; i++) {
            A = a[i] + b[i];
            B = 4 * a[i] - b[i];
            if (A >= B && A > 1)
//#pragma omp atomic
                sum1 += A;
        }
#pragma omp section 
        for (i = 0; i < N; i++) {
            A = a[i] + b[i];
            B = 4 * a[i] - b[i];
            if (A < B && B > 1)
//#pragma omp atomic
                sum2 += B;
        }
    }
    auto stopParSec1 = chrono::system_clock::now();
    cout << "\n  Sum of ParallelSec1: " << sum1 + sum2;

    // Распараллеливание по секциям (2 способ)
    sum1 = 0, sum2 = 0;
    auto startParSec2 = chrono::system_clock::now();
#pragma omp parallel sections shared(a, b, sum1, sum2) private(i, A, B, C)
    {
#pragma omp section 
        for (i = 0; i < N / 2; i++) {
            A = a[i] + b[i];
            B = 4 * a[i] - b[i];
            C = max(A, B);
            if (C > 1)
//#pragma omp atomic
                sum1 += C;
        }
#pragma omp section 
        for (i = N / 2; i < N; i++) {
            A = a[i] + b[i];
            B = 4 * a[i] - b[i];
            C = max(A, B);
            if (C > 1)
//#pragma omp atomic
                sum2 += C;
        }
    }
    auto stopParSec2 = chrono::system_clock::now();
    cout << "\n  Sum of ParallelSec2: " << sum1 + sum2;

    // Параллельный фрагмент с замком
    sum = 0;
    omp_lock_t lock;
    omp_init_lock(&lock);
    auto startParLock = chrono::system_clock::now();
#pragma omp parallel for shared(a, b, sum) private(i, A, B, C)
    for (i = 0; i < N; i++) {
        A = a[i] + b[i];
        B = 4 * a[i] - b[i];
        C = max(A, B);
        if (C > 1)
        {
            omp_set_lock(&lock);
            if (C > 1)
                sum += C;
            omp_unset_lock(&lock);
        }
    }
    auto stopParLock = chrono::system_clock::now();
    omp_destroy_lock(&lock);
    cout << "\n  Sum of ParallelLock: " << sum;

    // Параллельный фрагмент с барьерной синхронизацией
    sum = 0;
    auto startParBarrier = chrono::system_clock::now();
#pragma omp parallel sections shared(a, b, AA, BB) private(i)
    {
#pragma omp section
        for (i = 0; i < N; i++) {
            AA[i] = a[i] + b[i];
        }
#pragma omp section         
        for (i = 0; i < N; i++) {
            BB[i] = 4 * a[i] - b[i];
        }
    }
#pragma omp barrier
#pragma omp parallel for shared(AA, BB, sum) private(i, C)
    for (i = 0; i < N; i++) {
        C = max(AA[i], BB[i]);
        if (C > 1)
#pragma omp atomic
            sum += C;
    }
    auto stopParBarrier = chrono::system_clock::now();
    cout << "\n  Sum of ParallelBarrier: " << sum;

    // Фрагмент без распараллеливания
    sum = 0;
    auto startNoPar = chrono::system_clock::now();
    for (i = 0; i < N; i++)
    {
        A = a[i] + b[i];
        B = 4 * a[i] - b[i];
        C = max(A, B);
        if (C > 1)
            sum += C;
        /*cout << "  i = " << i << "  a[i]+b[i] = " << a[i] + b[i]
            << "  4*a[i]-b[i] = " << 4 * a[i] - b[i] << "\t  sum = " << sum << "\n";*/
    }
    auto stopNoPar = chrono::system_clock::now();
    cout << "\n  Sum of NoParallel: " << sum << endl;

    chrono::duration<double> elapsPar = (stopPar - startPar);
    chrono::duration<double> elapsParSec1 = (stopParSec1 - startParSec1);
    chrono::duration<double> elapsParSec2 = (stopParSec2 - startParSec2);
    chrono::duration<double> elapsParLock = (stopParLock - startParLock);
    chrono::duration<double> elapsParBarrier = (stopParBarrier - startParBarrier);
    chrono::duration<double> elapsNoPar = (stopNoPar - startNoPar);

    cout << "\n  Time of Parallel: " << elapsPar.count();
    cout << "\n  Time of ParallelSec1: " << elapsParSec1.count();
    cout << "\n  Time of ParallelSec2: " << elapsParSec2.count();
    cout << "\n  Time of ParallelLock: " << elapsParLock.count();
    cout << "\n  Time of ParallelBarrier: " << elapsParBarrier.count();
    cout << "\n  Time of NoParallel: " << elapsNoPar.count() << endl << endl;

    delete[] a;
    delete[] b;
    delete[] AA;
    delete[] BB;
}
