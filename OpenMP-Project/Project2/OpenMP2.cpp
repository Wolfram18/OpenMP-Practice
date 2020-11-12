#include <omp.h>
#include <chrono>
#include <iostream>
using namespace std;

const int NMAX = 50 - 1;
const int LIMIT = 50;

float a[NMAX][NMAX];

// Изучение использования условий 
// при определении параллельных фрагментов
void main()
{
    srand(time(0));
    int i, j;
    float sum;

    for (i = 0; i < NMAX; i++)
        for (j = 0; j < NMAX; j++)
            a[i][j] = rand() % 20;

    // Параллельный фрагмент
    auto start = chrono::system_clock::now();
#pragma omp parallel if (NMAX > LIMIT)
    {
#pragma omp for private(i,j,sum) 
        for (i = 0; i < NMAX; i++)
        {
            sum = 0;
            for (j = 0; j < NMAX; j++)
                sum += a[i][j];
            //cout << "  Line: " << i << " --- Sum: " << sum << endl;
        }
    }
    auto stop = chrono::system_clock::now();

    chrono::duration<double> elaps = (stop - start);

    if (NMAX > LIMIT)
        cout << "\n  Parallel End: " << elaps.count() << endl << endl;
    else
        cout << "\n  NoParallel End: " << elaps.count() << endl << endl;
}
