#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>

const int INF = 0x1fffffff;

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);
	int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	FILE *fin;
	fin = fopen(argv[1], "rb");
	
	int N, M;
	fread(&N, sizeof(int), 1, fin);
	fread(&M, sizeof(int), 1, fin);
	
	int *vID, *dest, *f, *cnt;
	
	vID = malloc((N/size+1)*sizeof(int));
	dest = malloc(N*sizeof(int));
	f = malloc(N*sizeof(int));
	
	int num = 0;
	for(int i=0 ; i<N ; ++i)
	{
		dest[i] = i%size;
		if(dest[i]==rank)
		{
			vID[num] = i;
			f[i] = num++;
		}	
	}
	
	int *FW[num];
	#pragma omp parallel
	{
		#pragma omp for schedule(static) nowait
		for(int i=0 ; i<num ; ++i){
			FW[i] = malloc(N*sizeof(int));
		}
	}
	
	#pragma omp parallel
	{
		#pragma omp for schedule(static) collapse(2) nowait
		for(int i=0 ; i<num ; ++i){
			for(int j=0 ; j<N ; ++j){
				if(vID[i]==j)
					FW[i][j] = 0;
				else
					FW[i][j] = INF;
			}
		}
	}
	
	for(int i=0 ; i<M ; ++i)
	{
		int u, v, w;
		fread(&u, sizeof(int), 1, fin);
		fread(&v, sizeof(int), 1, fin);
		fread(&w, sizeof(int), 1, fin);
		if(dest[u]==rank && u!=v)
			if(w < FW[f[u]][v])
				FW[f[u]][v] = w;
	}	
	fclose(fin);
	
	int *distK = malloc(N*sizeof(int));
	
	for(int k=0 ; k<N ; ++k)
	{
		if(rank==dest[k])
		{
			#pragma omp parallel
			{
				#pragma omp for schedule(static) nowait
				for(int i=0 ; i<N ; ++i){
					distK[i] = FW[f[k]][i];
				}
			}				
		}	
		MPI_Bcast(distK, N, MPI_INT, dest[k], MPI_COMM_WORLD);
		#pragma omp parallel
		{
			#pragma omp for schedule(static) collapse(2) nowait
			for(int i=0 ; i<num ; ++i){
				for(int j=0 ; j<N ; ++j){
					if(FW[i][k]+distK[j] < FW[i][j])
						FW[i][j] = FW[i][k]+distK[j];
				}
			}
		}
	}
	
	MPI_File fout;
	MPI_File_open(MPI_COMM_WORLD, argv[2], MPI_MODE_CREATE|MPI_MODE_WRONLY, MPI_INFO_NULL, &fout);
	for(int i=0 ; i<num ; ++i)
		MPI_File_write_at(fout, vID[i]*N*sizeof(int), FW[i], N, MPI_INT, MPI_STATUS_IGNORE);
	MPI_File_close(&fout);
	
	MPI_Finalize();
	return 0;
}