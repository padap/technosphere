//OS:  Linux
//Run: ./sort <array_len>

#include <iostream>
#include <string.h>
#include <omp.h>
#include <cstdlib>

using namespace std;
int *a,*b, *tmp;
int numThreads, SIZE; 

//Merge Sort
void mergeSort(int v[], int start, int final, int mode, int tmp[], int numThreads)
{
	if (start == final)
		return;
	auto pos = (final - start) / 2;
	if (mode * numThreads != 0)
	{
		#pragma omp parallel sections num_threads(2)
		{
			#pragma omp section
				mergeSort(v, start, start + pos, mode, tmp, numThreads / 2);
			#pragma omp section
				mergeSort(v, start + pos + 1, final, mode, tmp, numThreads - numThreads / 2);
		}
	}
	else
	{
		mergeSort(v, start, start + pos, mode, tmp, numThreads);
		mergeSort(v, start + pos + 1, final, mode, tmp, numThreads);	
	}
	auto pos1 = start;
	auto pos2 = start + pos + 1;
	auto it = start;

	while (pos1 < start + pos + 1 && pos2 < final + 1)
	{
		if (v[pos1] < v[pos2])
		{
			tmp[it] = v[pos1];
			pos1++;
		}
		else
		{
			tmp[it] = v[pos2];
			pos2++;
		}
		it++;
	}
	while (pos1 < start + pos + 1)
	{
		tmp[it] = v[pos1];
		pos1++;
		it++;
	}
	while (pos2 < final + 1)
	{
		tmp[it] = v[pos2];
		pos2++;
		it++;
	}
	memcpy(v + start, tmp + start, (final - start + 1) * sizeof(int));
}

//Allocate memory fo array
void allocateArray(){
	a = new int[SIZE];
    b = new int[SIZE];
	tmp = new int[SIZE];
	for (int i = 0; i < SIZE; i++)
	{
		auto val = rand() / 2000;
		a[i] = val;
		b[i] = val;
	}
}

//Free memory for array
void freeArray(){
    delete [] a;
	delete [] b;
}

int main(int argc, char * argv[])
{
	SIZE      = atoi(argv[1]);

	#pragma omp parallel
	{
		#pragma omp master
		{
			numThreads = omp_get_num_threads();
		}
	}
    
    cout << "\nThreads:       "<< numThreads<< "\n";
    cout << "Array size:    "<<SIZE<<"\n\n";
    allocateArray();

	srand(time(NULL));
	double t1, t2, t3;
    
   	t1 = omp_get_wtime();
	mergeSort(a, 0, SIZE - 1, 1, tmp, numThreads);

	t2 = omp_get_wtime();
	cout << "Parallel sort: " << double(t2 - t1) << "s\n";

	mergeSort(b, 0, SIZE - 1, 0, tmp, 0);
	t3 = omp_get_wtime();
	cout << "Normal sort:   " << double(t3 - t2) << "s\n";
    
    freeArray();

	return 0;
}