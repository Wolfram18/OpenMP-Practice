#include <omp.h>
#include <chrono>
#include <iostream>
using namespace std;

const int N = 50;
const int M = 50;

int sumBinary(int date) {
    int sum = 0;
    while (date != 0)
    {
        if (date % 2 == 1)
            sum++;
        date /= 2;
    }
    return sum;
}

// Посчитать сумму единиц в бинарной записи числа 
// всех попарных произведений элементов для каждой строки
void main()
{
    srand(time(0));
    int i, j, k, sum;

    int* a = new int[N * M];
    for (i = 0; i < N * M; i++)
        a[i] = rand() % 20;

    // Параллельный фрагмент
    auto startPar = chrono::system_clock::now();
#pragma omp parallel shared(a) private(i) num_threads(10)
    {
#pragma omp for private(j, k, sum) 
        for (i = 0; i < N * M; i = i + M) // Построчно
        {
            sum = 0;
            for (j = i; j < i + M; j++) // Попарно
                for (k = j + 1; k < i + M; k++)
                    sum += sumBinary(a[j] * a[k]);
            //cout << "  Line: " << i / M + 1 << " --- 1: " << sum << endl;
        }
    }
    auto stopPar = chrono::system_clock::now();

    // Фрагмент без распараллеливания
    auto startNoPar = chrono::system_clock::now();
    for (i = 0; i < N * M; i = i + M) // Построчно
    {
        sum = 0;
        for (j = i; j < i + M; j++) // Попарно
            for (k = j + 1; k < i + M; k++)
                sum += sumBinary(a[j] * a[k]);
        //cout << "  Line: " << i / M + 1 << " --- 1: " << sum << endl;
    }
    auto stopNoPar = chrono::system_clock::now();

    chrono::duration<double> elapsPar = (stopPar - startPar);
    chrono::duration<double> elapsNoPar = (stopNoPar - startNoPar);

    cout << "\n  Time of Parallel: " << elapsPar.count();
    cout << "\n  Time of NoParallel: " << elapsNoPar.count() << endl << endl;

    delete[] a;
}
