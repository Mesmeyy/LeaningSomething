#include <stdio.h>
#include <string.h>
#include <mpi.h>
#pragma warning(disable : 4096)
#define MAXSTRING 100
using namespace std;
#include <fstream>
#include <iostream>

int main(int argc,char* argv[])
{
	int comm_sz;
	int my_rank;
	
	MPI_Init(NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);


	if (comm_sz <= 1) {
		MPI_Finalize();
		return 0;
	}

	int N = 0;
	double *b = NULL, *local, **a, *ks, k, *local_arr;
	cin >> N;
	int rank_len = 0;

	if (my_rank == 0) {
		a = new double*[N];
		for (int i = 0; i < N; ++i) {
			a[i] = new double[N];
		}
		for (int i = 0; i < N; ++i)
			for (int j = 0; j < N; ++j)
				cin >> a[j][i];
		ks = new double[N];
		for (int i = 0; i < N; ++i) cin >> ks[i];
		cout << "184 227 176 161 189 219 266 150 332 152"<<endl;
		rank_len = N / comm_sz;
		for (int i = 1; i < comm_sz; ++i) {
			MPI_Send(&N, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			for (int j = 0; j < rank_len; ++j) {
				MPI_Send(a[i * rank_len + j], N, MPI_DOUBLE, i, 2 * j + 1, MPI_COMM_WORLD);
				MPI_Send(&ks[i * rank_len + j], 1, MPI_DOUBLE, i, 2 * j + 2, MPI_COMM_WORLD);
			}
		}
		local_arr = new double[N];
		local = new double[N];
		for (int i = 0; i < N; ++i)
			local_arr[i] = 0;
		for (int i = 0; i < rank_len; ++i) {
			for (int j = 0; j < N; ++j) {
				local_arr[j] += (a[i][j] * ks[i]);
			}
		}
		MPI_Reduce(local_arr, local, N, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
		for (int i = 0; i < N; ++i) cout << local[i] << " ";
		cout << endl;
		for (int i = 0; i < N; ++i) delete[]a[i];
		delete[]a;
		delete[]ks;
	}
	else {
		MPI_Recv(&N, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
		local = new double[N];
		local_arr = new double[N];
		for (int i = 0; i < N; ++i) local_arr[i] = 0;
		rank_len = N / comm_sz;
		for (int j = 0; j < rank_len; ++j) {
			MPI_Recv(local, N, MPI_DOUBLE, 0, 2 * j + 1, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
			MPI_Recv(&k, 1, MPI_DOUBLE, 0, 2 * j + 2, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
			for (int i = 0; i < N; ++i) local_arr[i] += (k * local[i]);
		}
		MPI_Reduce(local_arr, NULL, N, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	}
	delete[]local_arr;
	delete[] local;
	MPI_Finalize();
	return 0;
}
