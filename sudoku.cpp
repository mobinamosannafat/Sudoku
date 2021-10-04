/*
pthread_create (thread, attr, start_routine, arg)
thread : An opaque, unique identifier for the new thread returned by the subroutine
attr : An opaque attribute object that may be used to set thread attributes. You can specify a thread attributes object, or NULL for the default values.
start_routine : The C++ routine that the thread will execute once it is created
arg : A single argument that may be passed to start_routine. It must be passed by reference as a pointer cast of type void. NULL may be used if no argument is to be passed.


Stderr, also known as standard error, is the default file descriptor where a process can write error messages.
*/





#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>


#define Free 0           //used for empty cells in sudoku
#define num_threads 31   // 3 * 9 + 4

/*
	Initialize the array which worker threads can update to 1 if the
	corresponding region of the sudoku puzzle they were responsible
 	for is valid.
*/

int valid[num_threads] = {0}; //initilize to 0

// Struct that stores the data to be passed to threads
typedef struct
{
	int row;
	int column;
} parameters;

// Hyper Sudoku puzzle to be solved
int sudoku[9][9] = {
	{1, 5, 6, 2, 4, 3, 7, 8, 9},
	{2, 3, 4, 7, 8, 9, 1, 5, 6},
	{7, 8, 9, 5, 1, 6, 2, 4, 3},
	{9, 6, 2, 1, 5, 7, 8, 3, 4},
	{5, 4, 3, 8, 6, 2, 9, 1, 7},
	{8, 7, 1, 3, 9, 4, 6, 2, 5},
	{4, 2, 5, 6, 7, 8, 3, 9, 1},
	{6, 9, 8, 4, 3, 1, 5, 7, 2},
	{3, 1, 7, 9, 2, 5, 4, 6, 8}
};


// Sudoku puzzle to be solved
// 0 = free cells
	/*int sudoku[9][9] = {
	{ 5, 3, 4, 6, 7, 8, 9, 1, 2 },
    { 6, 7, 2, 1, 9, 5, 3, 4, 8 },
    { 1, 9, 8, 3, 4, 2, 5, 6, 7 },
    { 8, 5, 9, 7, 6, 1, 4, 2, 3 },
    { 4, 2, 6, 8, 5, 3, 7, 9, 1 },
    { 7, 1, 3, 9, 2, 4, 8, 5, 6 },
    { 9, 6, 1, 5, 3, 7, 2, 8, 4 },
    { 2, 8, 7, 4, 1, 9, 6, 3, 5 },
    { 3, 4, 5, 2, 8, 6, 1, 7, 9 }
 };
*/

//*******functions for solving*******

//check that if we enter this number , this specified row is valid or not
bool CheckRowValid(int sudoku[9][9], int row, int num)
{
	for (int col = 0; col < 9; col++)
    {
        if (sudoku[row][col] == num)
        {
            return false;
        }
    }
	return true;
}

//check that if we enter this number , this specified column is valid or not
bool CheckColumnValid(int sudoku[9][9], int col, int num)
{
	for (int row = 0; row < 9; row++)
    {
        if (sudoku[row][col] == num)
        {
            return false;
        }
    }
	return true;
}

//check that if we enter this number , this specified 3*3 box is valid or not
bool Check3x3Valid(int sudoku[9][9], int SRow, int SCol, int num)
{
	for (int row = 0; row < 3; row++)
    {
        for (int col = 0; col < 3; col++)
        {
            if (sudoku[row + SRow][col + SCol] == num)
            {
                return false;
            }
        }
    }
	return true;
}


bool CheckSafety(int sudoku[9][9], int row, int col, int num)
{
	// Check if 'num' is not already placed in current row, current column and current 3x3 box and chosen cell is free
        if(CheckRowValid(sudoku, row, num) &&  CheckColumnValid(sudoku, col, num)  &&
           Check3x3Valid(sudoku, row - row % 3,col - col % 3, num) && sudoku[row][col] == Free)
        {
            return true;
        }
        else return false;
}


//Search in to sudoku array find an entry that is still unassigned(set to 0)
bool FindFree(int sudoku[9][9], int &row, int &col)
{
	for (row = 0; row < 9; row++)
    {
        for (col = 0; col < 9; col++)
        {
            if (sudoku[row][col] == Free)
            {
                return true;
            }
        }
    }

	return false;
}

/* this function find free cells and a number to set there and check if its
valid in the row , column and 3*3 box */
bool LetsSolve(int sudoku[9][9])
{
	int row, col;

	// If there is no free (0) location, sudoku is solved
	if (!FindFree(sudoku, row, col))
    {
        return true; // success!
    }


    // Fill with digits 1 to 9
	for (int num = 1; num <= 9; num++)
	{
		// check
		if (CheckSafety(sudoku, row, col, num))
		{
			sudoku[row][col] = num;

			// return, if it's okay
			if (LetsSolve(sudoku))
            {
                return true;
            }

			// failure, unmake & try again
			sudoku[row][col] = Free;
		}
	}
	return false; // this cause backtracking
}




//*******functions for cheking*******

// Method that determines if numbers 1-9 only appear once in a column
void *isColumnValid(void* param)
 {
	// Confirm that parameters indicate a valid col subsection
	parameters *params = (parameters*) param;
	int row = params->row;
	int col = params->column;

	if (row != 0 || col > 8)
    {
		fprintf(stderr, "Invalid row or column for col subsection! row=%d, col=%d\n", row, col);
		pthread_exit(NULL);
    }

	// Check if numbers 1-9 only appear once in the column
	int validityArray[9] = {0};
	int i;

	for (i = 0; i < 9; i++)
    {
		int num = sudoku[i][col];

		if (num < 1 || num > 9 || validityArray[num - 1] == 1)
        {
			pthread_exit(NULL);
		}
        else
        {
			validityArray[num - 1] = 1;
		}
	}

	// If reached this point, col subsection is valid.
	valid[18 + col] = 1;
	pthread_exit(NULL);
}

// Method that determines if numbers 1-9 only appear once in a row
void *isRowValid(void* param)
{
	// Confirm that parameters indicate a valid row subsection
	parameters *params = (parameters*) param;
	int row = params->row;
	int col = params->column;

	if (col != 0 || row > 8)
    {
		fprintf(stderr, "Invalid row or column for row subsection! row=%d, col=%d\n", row, col);
		pthread_exit(NULL);
	}

	// Check if numbers 1-9 only appear once in the row
	int validityArray[9] = {0};
	int i;

	for (i = 0; i < 9; i++)
    {
		// If the corresponding index for the number is set to 1, and the number is encountered again,
		// the valid array will not be updated and the thread will exit.
		int num = sudoku[row][i];

		if (num < 1 || num > 9 || validityArray[num - 1] == 1)
        {
			pthread_exit(NULL);
		}
		else
        {
			validityArray[num - 1] = 1;
		}
	}
	// If reached this point, row subsection is valid.
	valid[9 + row] = 1;
	pthread_exit(NULL);
}

// Method that determines if numbers 1-9 only appear once in a 3x3 subsection
void *is3x3Valid(void* param)
{
	// Confirm that parameters indicate a valid 3x3 subsection
	parameters *params = (parameters*) param;
	int row = params->row;
	int col = params->column;

	if (row > 6 || row % 3 != 0 || col > 6 || col % 3 != 0)
    {
		fprintf(stderr, "Invalid row or column for subsection! row=%d, col=%d\n", row, col);
		pthread_exit(NULL);
	}

	int validityArray[9] = {0};
	int i, j;

	for (i = row; i < row + 3; i++)
    {
		for (j = col; j < col + 3; j++)
		{
			int num = sudoku[i][j];
			if (num < 1 || num > 9 || validityArray[num - 1] == 1)
			{
				pthread_exit(NULL);
			}
			 else
            {
				validityArray[num - 1] = 1;
			}
		}
	}

	// If reached this point, 3x3 subsection is valid.
	valid[row + col/3] = 1; // Maps the subsection to an index in the first 8 indices of the valid array
	pthread_exit(NULL);
}


void *isHyperValid(void* param)
{
	// Confirm that parameters indicate a valid 3x3 Hyper subsection
	parameters *params = (parameters*) param;
	int row = params->row;
	int col = params->column;

	if (!((row==1 && (col==1 || col==5)) || (row==5 && (col==1 || col==5))))
    {
		fprintf(stderr, "NOT Hyper!!!\nInvalid row or column for subsection! row=%d, col=%d\n", row, col);
		pthread_exit(NULL);
	}

	int validityArray[9] = {0};
	int i, j;

	for (i = row; i < row + 3; i++)
    {
		for (j = col; j < col + 3; j++)
		{
			int num = sudoku[i][j];
			if (num < 1 || num > 9 || validityArray[num - 1] == 1)
			{
				pthread_exit(NULL);
			}
			 else
            {
				validityArray[num - 1] = 1;
			}
		}
	}

	// If reached this point, 3x3 Hyper subsection is valid.
	//in valid array elments 27 to 30 belongs to this feature
	if(row==1 && col==1)
    {
        valid[27] = 1;
    }
    else if(row==1 && col==5)
    {
        valid[28] = 1;
    }
    else if(row==5 && col==1)
    {
        valid[29] = 1;
    }
    else if(row==5 && col==5)
    {
        valid[30] = 1;
    }

	pthread_exit(NULL);
}



int main()
{
    //first lets solve it using backtracking

    clock_t start1 = clock();

    if (LetsSolve(sudoku) == true)
    {
        for (int prow = 0; prow < 9; prow++)
        {
            for (int pcol = 0; pcol < 9; pcol++)
            {
               printf("%d ",sudoku[prow][pcol]);
            }

            printf("\n");
        }
    }
	else
		printf("No solution exists");


    clock_t finish1 = clock();

    //now we can check it

    clock_t start2 = clock();

	pthread_t threads[num_threads];

	int threadIndex = 0;
	int i,j;

	// Create 9 threads for 9 3x3 subsections,
	// 9 threads for 9 columns and 9 threads for 9 rows.
	// This will end up with a total of 27 threads.
	for (i = 0; i < 9; i++)
    {
		for (j = 0; j < 9; j++)
        {
			if (i%3 == 0 && j%3 == 0)
			{
				parameters *data = (parameters *) malloc(sizeof(parameters));
				data->row = i;     //SATR
				data->column = j;  //SOTUN
				pthread_create(&threads[threadIndex++], NULL, is3x3Valid, data); // 3x3 subsection threads
                                                                                //pthread_create is used to create a POSIX thread
			}

			if (i == 0)
            {
				parameters *columnData = (parameters *) malloc(sizeof(parameters));
				columnData->row = i;
				columnData->column = j;
				pthread_create(&threads[threadIndex++], NULL, isColumnValid, columnData);	// column threads
                                                                                           //pthread_create is used to create a POSIX thread
			}

			if (j == 0)
            {
				parameters *rowData = (parameters *) malloc(sizeof(parameters));
				rowData->row = i;
				rowData->column = j;
				pthread_create(&threads[threadIndex++], NULL, isRowValid, rowData); // row threads
                                                                                   //pthread_create is used to create a POSIX thread
			}

			if( (i==1 && (j==1 || j==5)) || (i==5 && (j==1 || j==5)) )
			{
			    parameters *data = (parameters *) malloc(sizeof(parameters));
				data->row = i;
				data->column = j;
				pthread_create(&threads[threadIndex++], NULL, isHyperValid, data); // 3x3 Hyper subsection threads
                                                                                //pthread_create is used to create a POSIX thread
			}

		}
	}

	/*
	The pthread_join() function shall suspend execution of
	the calling thread until the target thread terminates,
    unless the target thread has already terminated
    */
	for (i = 0; i < num_threads; i++)
    {
		pthread_join(threads[i], NULL);			// Wait for all threads to finish
	}


	clock_t finish2 = clock();


    printf("\n\n");

    double time1;
    double time2;

	time1 = double(finish1 - start1)/(double) CLOCKS_PER_SEC;
	time2 = double(finish2 - start2)/(double) CLOCKS_PER_SEC;

	printf("The amount of time for solving: %f seconds\n",time1 );
	printf("The amount of time for cheking: %f seconds",time2 );

	printf("\n\n");

	// If any of the entries in the valid array are 0, then the sudoku solution is invalid
	for (i = 0; i < num_threads; i++)
    {

		if (valid[i] == 0)
		{
		    if (i<=26)
			{
			    printf("Sudoku solution is invalid!\n");
                return EXIT_SUCCESS;
			}
			else if (i>26)
            {
                printf("Sudoku solution is valid but not hyper!\n");
                return EXIT_SUCCESS;
            }
		}
	}


	return EXIT_SUCCESS;
}

