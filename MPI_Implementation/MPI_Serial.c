#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#ifndef Min
#define Min(X,Y) (((X)<(Y)) ? (X):(Y))
#endif
#ifndef Max
#define Max(X,Y) (((X)<(Y)) ? (Y):(X))
#endif
void NextGameState(_Bool* oldboard,_Bool* newBoard,int rows,int cols);
int GetNeighbours(_Bool* board,int rows,int cols,int width);
_Bool NextCellState(_Bool* board, int row, int col, int width, int neighbours);
void PrintArray(_Bool* ptr,int rows,int cols);


int main(int argc, char const *argv[])
{
	int numProcs,rank;
	int iterations,rows,cols,iter,hcols,hrows;
	_Bool* currentGen;
	_Bool* nextGen;
	MPI_Status stat;
	MPI_Request req;

// INPUT PARSING--------------------------------------------------------------------
	if(argc <4){
		printf("Usage: mpexec -n <nproc> <exefile> <rows> <cols> <iterations>\n");
		return -1;
	}
	rows = atoi(argv[1]);
	cols = atoi(argv[2]);
	iterations = atoi(argv[3]);
	hcols = cols+2;//two additional hollow columns
	hrows = rows+2;//two additional hollow rows

// INPUT PARSING END----------------------------------------------------------------

// CREATE Array here
	currentGen = (_Bool*) malloc((hcols)*(hrows)*sizeof(_Bool));
	nextGen = (_Bool*) malloc((hcols)*(hrows)*sizeof(_Bool));

	for (int i = 0; i < hrows; ++i)
	{
		for (int j = 0; j < hcols; ++j)
		{
			if (i==0 || j==0 || i==(rows+1) || j==(cols+1))
			{
				currentGen[i*(hcols)+j] = 0;
				nextGen[i*(hcols)+j] = 0;
				continue;
			}
			currentGen[i*(hcols)+j] = rand()%2;
		}
	}

// FINISH array creation

	MPI_Init(NULL,NULL);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Barrier(MPI_COMM_WORLD);
	double t1,t2,elapsed;
	elapsed = 0;

if (rank == 0)
{
	printf("NUMPROCS %d\n",numProcs);
	printf("ITERATIONS %d\n",iterations);
	printf("PROBLEM SIZE %d %d\n",rows,cols);
}




for (int generations = 0; generations < iterations; ++generations)
{
	MPI_Comm_size(MPI_COMM_WORLD,&numProcs);
	if (rank == 0)
	{	
		//ONLY MASTER
		// MAIN ALGORITHM START
		t1 = MPI_Wtime();
		NextGameState(currentGen,nextGen,hrows,hcols);
		t2 = MPI_Wtime();
		elapsed += t2 -t1;

		// system("clear");
		printf("elapsed time: %f\n",t2-t1);
		// PrintArray(nextGen,hrows,hcols);
		// sleep(1);
		_Bool* tmp = nextGen;
		nextGen = currentGen;
		currentGen = tmp;

	}

	MPI_Barrier(MPI_COMM_WORLD);

}


if (rank == 0)
{
	printf("Average case: %f\n", elapsed/iterations);
}


	MPI_Finalize();
	free(currentGen);
	free(nextGen);
	return 0;
}


void NextGameState(_Bool* oldboard,_Bool* newBoard,int rows,int cols){
	int neighbours;
	for (int i = 1; i < rows-1; ++i)
	{
		for (int j = 1; j < cols-1; ++j)
		{
			neighbours = GetNeighbours(oldboard,i,j,cols);
			newBoard[(i*cols)+j] = NextCellState(oldboard,i+1,j,cols,neighbours);
		}
	}
	return;
}

int GetNeighbours(_Bool* board,int rows,int cols,int width){
	int count = 0;
	int dy[] = {-1, -1, -1, 0, 0, 1, 1, 1};
	int dx[] = {-1, 0, 1, -1, 1, -1, 0, 1};

	for (int i = 0; i < 8; ++i)
	{
		// printf("yay: %d\n",board[(rows+dy[i])*width+(cols+dx[i])]);
		if (board[(rows+dy[i])*width+(cols+dx[i])] == 1)
		{
			count++;
		}
	}
	return count;
}

_Bool NextCellState(_Bool* board, int row, int col, int width, int neighbours){
	if (board[(row*width)+col] == 1)
	{
		if (neighbours<2)
		{
			return 0;
		}
		else if (neighbours == 2 || neighbours == 3)
		{
			return 1;
		}
		else if (neighbours > 3)
		{
			return 0;
		}
	}
	else if (board[(row*width)+col]==0)
	{
		if (neighbours == 3)
		{
			return 1;
		}
	}
	return board[(row*width)+col];
}

void PrintArray(_Bool* ptr,int rows,int cols){
	for (int i = 0; i < rows; ++i)
	{
		for (int j = 0; j < cols; ++j)
		{
			printf("%d", ptr[(i*cols)+j]);
		}
		printf("\n");
	}
}