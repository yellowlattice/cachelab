/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*one set/block is 2^5 = 32bytes, can hold 8 int*/
/*FILL THE CACHE consecutively, THEN REPEAT(CONFLICT MISS)*/

/*32*32:
1. use 8*8 blocking(8 matrix rows fill the cache so no conflict miss within a blocking)
2. write prev to avoid diagonal elements causing conflict miss(A[m][n] B[m][n] both mapped to set_index = m + 8)*/

void trans_32_32(int A[32][32], int B[32][32])
{
	int i, j, tmp, x, y, prev; //6 variables
	for(x = 0; x < 4; x++)
	{
		for(y = 0; y < 4; y++)
		{
			for(i = 8*x; i < 8 + 8*x; i++) {
        		for(j = 8*y; j < 8 + 8*y; j++){
            		tmp = A[i][j];
            		if(i == j)
            		{
            			if(i > 0) B[j-1][i-1] = prev; //write previous one
            			prev = tmp; 
            		}
            		else B[j][i] = tmp;
        			}
    		}    
		}
	}
	B[32 - 1][32 - 1] = A[32 - 1][32 - 1];
}

/*64*64 is the hard one: 
1. 4*4 blocking at least around 1700. For B, each set we load 8 but use only 4, loss of spatial locality(requires 8);
2. 8*8 blocking, the problem is, 4 matrix rows fill the cache, causing conflict miss within a blocking, specificly, row 5/1 6/2 
7/3 8/4 mapped to the same sets.
One solution is to divide within the 8*8 blocking into 4 4*4 to analyze, in which moves between up and down cause conflict miss:
Originally, we visit 
A: upleft-upright-downleft-downright (1 conflict miss); 
B: upleft-downleft-upright-downright (3 conflict miss).
The upleft and downright part do not change. The trick is we can first wrongly transpose A upright to B upright, then use local 
vars to get a B upright row, and transpose the correct A downleft colomn to the B upright row, and then give B downleft row the 
local var values, so we in B we also go from upright to downleft:
A: upleft-upright-downleft-downright (1 conflict miss)
B: upleft-upright-downleft-downright (1 conflict miss)
*/

void trans_64_64(int A[64][64], int B[64][64])
{
	int i, j, k, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8; //11 variables
	for(j = 0; j < 64; j += 8){
            for(i = 0; i < 64; i += 8){
            	/*same A upleft to B upleft, but A upright to B upright*/
                for(k = i; k < i + 4; k++)
                {
                    tmp1=A[k][j];tmp2=A[k][j+1];tmp3=A[k][j+2];tmp4=A[k][j+3];
                    tmp5=A[k][j+4];tmp6=A[k][j+5];tmp7=A[k][j+6];tmp8=A[k][j+7];
                    B[j][k]=tmp1;B[j][k+4]=tmp5;B[j+1][k]=tmp2;B[j+1][k+4]=tmp6;
                    B[j+2][k]=tmp3;B[j+2][k+4]=tmp7;B[j+3][k]=tmp4;B[j+3][k+4]=tmp8;                               
                }
                for(k = j; k < j + 4; k++)
                {
                	/*get B upright*/
                    tmp1=B[k][i+4];tmp2=B[k][i+5];tmp3=B[k][i+6];tmp4=B[k][i+7];
                    /*get A downleft*/
                    tmp5=A[i+4][k];tmp6=A[i+5][k];tmp7=A[i+6][k];tmp8=A[i+7][k];
                    /*A downleft to B upright*/
					B[k][i+4]=tmp5;B[k][i+5]=tmp6;B[k][i+6]=tmp7;B[k][i+7]=tmp8;
					/*B upright to B downleft*/
                    B[k+4][i]=tmp1;B[k+4][i+1]=tmp2;B[k+4][i+2]=tmp3;B[k+4][i+3]=tmp4;
                }
                /*same A downright to B downright*/
                for(k = j + 4; k < j + 8; k++)
                {
                    tmp1=A[i+4][k];tmp2=A[i+5][k];tmp3=A[i+6][k];tmp4=A[i+7][k];
                    B[k][i+4]=tmp1;B[k][i+5]=tmp2;B[k][i+6]=tmp3;B[k][i+7]=tmp4;
                }
            }
        }
}

/*the restriction is loose*/
void trans_61_67(int M, int N, int A[N][M], int B[M][N])
{
	int i, j, s, tmp; //4 variables
	for(s = 0; s < M; s+=8)
	{
		for(i = 0; i < N; i++)
		{
			for(j = s; j < s+8 && j < M; j++)
			{
				tmp = A[i][j];
				B[j][i] = tmp;
			}
		}
	}

}
/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
	if(M == 32) trans_32_32(A, B);
	else if(M == 64) trans_64_64(A, B);
	else trans_61_67(M, N, A, B);
}



/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 
char trans_p4_desc[] = "parament4";
void trans_p4(int M, int N, int A[N][M], int B[M][N])
{
	int i, j, tmp, x, y, prev; //6 variables
	for(x = 0; x < 16; x++)
	{
		for(y = 0; y < 16; y++)
		{
			for(i = 4*x; i < 4 + 4*x; i++) {
        		for(j = 4*y; j < 4 + 4*y; j++){
            		tmp = A[i][j];
            		if(i == j)
            		{
            			if(i > 0) B[j-1][i-1] = prev; //write previous one
            			prev = tmp; 
            		}
            		else B[j][i] = tmp;
        			}
    		}    
		}
	}
	B[64 - 1][64 - 1] = A[64 - 1][64 - 1];
}


/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

	for (i = 0; i < N; i++) {
		for (j = 0; j < M; j++) {
			tmp = A[i][j];
			B[j][i] = tmp;
		}
	}
}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 
    registerTransFunction(trans_p4, trans_p4_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

