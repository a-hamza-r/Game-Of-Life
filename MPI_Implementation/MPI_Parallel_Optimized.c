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

int main(int argc, char const *argv[]){
	int numProcs,rank;
	int iterations,rows,cols,iter,hcols,hrows;
	_Bool* currentGen=NULL;
	_Bool* nextGen=NULL;
	_Bool* tmp=NULL;
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


	
	MPI_Init(NULL,NULL);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	// MPI_Barrier(MPI_COMM_WORLD);
	double t1,t2,elapsed;
	elapsed=0;


if (rank==0)
{
	printf("NUMPROCS %d\n",numProcs);
	printf("ITERATIONS %d\n",iterations);
	printf("PROBLEM SIZE %d %d\n",rows,cols);
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
			nextGen[(i*hcols)+j] = 0;
		}
	}

// FINISH array creation
}

for (int generations = 0; generations < iterations; ++generations)
{
	MPI_Comm_size(MPI_COMM_WORLD,&numProcs);
	if (rank == 0)
	{	
		t1= MPI_Wtime();
		numProcs--;//excluding Master process
		//MASTER

		// PrintArray(currentGen,hrows,hcols);
		int iter,length,end,pRank;
		int lohi[2];
		length = rows;
		iter = Max(1,(length/numProcs));
		pRank=1;
		for (int i = 1; i <= Min(numProcs-1,length); i++)
		{	//adding a hollow ring up and below

			end = Min(i*iter,length);
			lohi[0]=((i-1)*iter);
			lohi[1]=end+1;

			MPI_Send(&lohi,2,MPI_INT,pRank,0,MPI_COMM_WORLD);
			MPI_Send(&currentGen[lohi[0]*(hcols)],(lohi[1]-lohi[0]+1)*(hcols),MPI_C_BOOL,pRank,0,MPI_COMM_WORLD);
			pRank++;
		}		
		//for last one, irregular remaining matrix
		if (pRank<=rows)
		{	
			lohi[0] = ((pRank-1)*iter);
			lohi[1] = length+1;
			MPI_Send(&lohi,2,MPI_INT,pRank,0,MPI_COMM_WORLD);
			MPI_Send(&currentGen[lohi[0]*(hcols)],(lohi[1]-lohi[0]+1)*(hcols),MPI_C_BOOL,pRank,0,MPI_COMM_WORLD);
		}
		// WAIT AND RECEIVE BACK
		int n=0;
		int sauce=0;
		for (int i = 1; i <=Min(numProcs,length) ; ++i)
		{
			MPI_Probe(MPI_ANY_SOURCE,0,MPI_COMM_WORLD,&stat);
			MPI_Get_count(&stat,MPI_C_BOOL,&n);
			_Bool* buff = malloc(n);						
			MPI_Recv(buff,n,MPI_C_BOOL,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,&stat);
			sauce = stat.MPI_SOURCE;
			lohi[0]=((sauce-1)*iter)+1;
			for (int i = 0; i < n; ++i)
			{
				//copy to nextGen
				nextGen[(lohi[0]*hcols)+i] = buff[i];
			}
			free(buff);
		}
			t2 = MPI_Wtime();
			// PrintArray(nextGen,hrows,hcols);
			elapsed += t2-t1;
			printf("elapsed time: %f\n",t2-t1);
			tmp = nextGen;
			nextGen = currentGen;
			currentGen = tmp;
	}
	else{
		// WORKERS
		int lohi[2];
		int n=0;
		if (rank <= rows)
		{
			MPI_Recv(&lohi,2,MPI_INT,0,0,MPI_COMM_WORLD,&stat);
			int size = (lohi[1]-lohi[0])+1;
			_Bool* mytmp = malloc(size*(hcols));
			_Bool* newState= malloc((size-2)*hcols);
			MPI_Recv(mytmp,size*hcols,MPI_C_BOOL,0,0,MPI_COMM_WORLD,&stat);

			// MAIN ALGORITHM START
			// for (int i = 0; i < size-2; ++i)
			// {
			// 	for (int j = 0; j < hcols; ++j)
			// 	{
			// 		newState[(i*hcols)+j] = 0;
			// 	}
			// }


			NextGameState(mytmp,newState,size-2,hcols);
			// PrintArray(newState,size-2,hcols);

			// MAIN ALGORITHM END

			MPI_Get_count(&stat,MPI_C_BOOL,&n);	

			MPI_Send(newState,(size-2)*(hcols),MPI_C_BOOL,0,0,MPI_COMM_WORLD);
			free(mytmp);
			free(newState);
		}

	}

	// MPI_Wait(&req,&stat);
	MPI_Barrier(MPI_COMM_WORLD);
	}

if (rank==0)
{
	printf("Average case: %f\n", elapsed/iterations);
}

	MPI_Finalize();
	free(currentGen);
	free(nextGen);

	return 0;
}

void NextGameState(_Bool* oldboard,_Bool* newBoard,int rows,int cols){
	// rows correspond to newBoard, oldboard has rows = rows+2
	int neighbours;
	for (int i = 0; i < rows; ++i)
	{
		for (int j = 1; j < cols-1; ++j)
		{
			neighbours = GetNeighbours(oldboard,i+1,j,cols);
			newBoard[(i*cols)+j] = NextCellState(oldboard,i+1,j,cols,neighbours);
		}
		// printf("\n");
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
