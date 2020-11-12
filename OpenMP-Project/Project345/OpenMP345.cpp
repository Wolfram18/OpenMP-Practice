#include <omp.h>
#include <chrono>
#include <iostream>
using namespace std;

const int N = 4500000;

int* a = new int[N];
int* b = new int[N];

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
    int i, sum = 0, sum1 = 0, sum2 = 0, A, B, C;

    // Заполнение массивов
#pragma omp parallel sections shared(a, b) private(i) 
    {
#pragma omp section
        for (i = 0; i < N; i++)
            a[i] = (int)myRand(); //a[i] = rand() % 10 - 5;
#pragma omp section
        for (i = 0; i < N; i++)
            b[i] = (int)myRand(); //b[i] = rand() % 10 - 5;
    }

    // Параллельный фрагмент
    auto startPar = chrono::system_clock::now();
#pragma omp parallel for shared(a, b) private(i, A, B, C) //reduction(+: sum)
    for (i = 0; i < N; i++) {
        A = a[i] + b[i];
        B = 4 * a[i] - b[i];
        C = max(A, B);
        if (C > 1)
#pragma omp atomic
            sum += C;
    }
    auto stopPar = chrono::system_clock::now();
    cout << "\n  SumPar: " << sum;

    // Распараллеливание по секциям (1 способ)
    auto startParSec1 = chrono::system_clock::now();
#pragma omp parallel sections shared(a, b) private(i, A, B, C) //reduction(+: sum1, sum2)
    {
#pragma omp section
        for (i = 0; i < N; i++) {
            A = a[i] + b[i];
            B = 4 * a[i] - b[i];
            if (A >= B && A > 1)
#pragma omp atomic
                sum1 += A;
        }
#pragma omp section 
        for (i = 0; i < N; i++) {
            A = a[i] + b[i];
            B = 4 * a[i] - b[i];
            if (A < B && B > 1)
#pragma omp atomic
                sum2 += B;
        }
    }
    auto stopParSec1 = chrono::system_clock::now();
    cout << "\n  SumParSec1: " << sum1 + sum2;

    sum1 = 0, sum2 = 0;
    // Распараллеливание по секциям (2 способ)
    auto startParSec2 = chrono::system_clock::now();
#pragma omp parallel sections shared(a, b) private(i, A, B, C) //reduction(+: sum1, sum2)
    {
#pragma omp section 
        for (i = 0; i < N / 2; i++) {
            A = a[i] + b[i];
            B = 4 * a[i] - b[i];
            C = max(A, B);
            if (C > 1)
#pragma omp atomic
                sum1 += C;
        }
#pragma omp section 
        for (i = N / 2; i < N; i++) {
            A = a[i] + b[i];
            B = 4 * a[i] - b[i];
            C = max(A, B);
            if (C > 1)
#pragma omp atomic
                sum2 += C;
        }
    }
    auto stopParSec2 = chrono::system_clock::now();
    cout << "\n  SumParSec2: " << sum1 + sum2;

    sum = 0;
    // Фрагмент без распараллеливания
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
    cout << "\n  SumNoPar: " << sum << endl;

    chrono::duration<double> elapsPar = (stopPar - startPar);
    chrono::duration<double> elapsParSec1 = (stopParSec1 - startParSec1);
    chrono::duration<double> elapsParSec2 = (stopParSec2 - startParSec2);
    chrono::duration<double> elapsNoPar = (stopNoPar - startNoPar);

    cout << "\n  Parallel End: " << elapsPar.count();
    cout << "\n  ParallelSec1 End: " << elapsParSec1.count();
    cout << "\n  ParallelSec2 End: " << elapsParSec2.count();
    cout << "\n  NoParallel End: " << elapsNoPar.count() << endl << endl;

    delete[] a;
    delete[] b;
}