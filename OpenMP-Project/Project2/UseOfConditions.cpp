#include <omp.h>
#include <iostream>
#include <ctime>
using namespace std;

const int NMAX = 7500; //50 - 1;
const int LIMIT = 10000;

float a[NMAX][NMAX];

// Изучение использования условий 
// при определении параллельных фрагментов
void main()
{
    srand(time(0));
    clock_t start, stop;
    int i, j;
    float sum;

    for (i = 0; i < NMAX; i++)
        for (j = 0; j < NMAX; j++)
            a[i][j] = i + j;

    // Параллельный фрагмент
    start = clock();
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
    stop = clock();

    if (NMAX > LIMIT)
        cout << "\n  Time of Parallel: " << (stop - start) << endl << endl;
    else
        cout << "\n  Time of NoParallel: " << (stop - start) << endl << endl;
}
