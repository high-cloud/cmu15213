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
    if (M == 32)
    {
        int a1,a2,a3,a4,a5,a6,a7,a8;
        for (int ii = 0; ii < 32; ii += 8)
            for (int jj = 0; jj < 32; jj += 8)
                for (int i = ii; i < ii + 8; ++i)
                {
                    a1=A[i][jj];
                    a2=A[i][jj+1];
                    a3=A[i][jj+2];
                    a4=A[i][jj+3];
                    a5=A[i][jj+4];
                    a6=A[i][jj+5];
                    a7=A[i][jj+6];
                    a8=A[i][jj+7];

                    B[jj][i]=a1;
                    B[jj+1][i]=a2;
                    B[jj+2][i]=a3;
                    B[jj+3][i]=a4;
                    B[jj+4][i]=a5;
                    B[jj+5][i]=a6;
                    B[jj+6][i]=a7;
                    B[jj+7][i]=a8;
                }
    }

    if(M==64)
    {
        int a1,a2,a3,a4,a5,a6,a7,a8;
        for (int ii = 0; ii < 64; ii += 8)
            for (int jj = 0; jj < 64; jj += 8)
            {
                //first 4 row
                for (int i = ii; i < ii+4 ; ++i)
                {
                    a1=A[i][jj];
                    a2=A[i][jj+1];
                    a3=A[i][jj+2];
                    a4=A[i][jj+3];
                    a5=A[i][jj+4];
                    a6=A[i][jj+5];
                    a7=A[i][jj+6];
                    a8=A[i][jj+7];

                    B[jj][i]=a1;
                    B[jj+1][i]=a2;
                    B[jj+2][i]=a3;
                    B[jj+3][i]=a4;
                    B[jj][i+4]=a5;
                    B[jj+1][i+4]=a6;
                    B[jj+2][i+4]=a7;
                    B[jj+3][i+4]=a8;

                }


                for(int i=0;i<4;++i)
                {
                    a1=B[jj+i][ii+4];
                    a2=B[jj+i][ii+5];
                    a3=B[jj+i][ii+6];
                    a4=B[jj+i][ii+7];

                    a5=A[ii+4][jj+i];
                    a6=A[ii+5][jj+i];
                    a7=A[ii+6][jj+i];
                    a8=A[ii+7][jj+i];

                    B[jj+i][ii+4]=a5;
                    B[jj+i][ii+5]=a6;
                    B[jj+i][ii+6]=a7;
                    B[jj+i][ii+7]=a8;

                    B[jj+4+i][ii]=a1;
                    B[jj+i+4][ii+1]=a2;
                    B[jj+i+4][ii+2]=a3;
                    B[jj+i+4][ii+3]=a4;
                    
                }

                for(int i=ii+4;i<ii+8;++i)
                    for(int j=jj+4;j<jj+8;++j)
                    {
                        B[j][i]=A[i][j];
                    }
            }
    }
    if(M==61)
    {
        int bsize=20;
        for (int ii = 0; ii < M; ii += bsize)
            for (int jj = 0; jj < N; jj += bsize)
                for (int i = ii; i < ii + bsize && i<M; ++i)
                {
                    for (int j=jj;j<jj+bsize && j<N;++j)
                    {
                        B[i][j]=A[j][i];
                    }
                }
    }
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; j++)
        {
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

    // /* Register any additional transpose functions */
    // registerTransFunction(trans, trans_desc);
}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++)
    {
        for (j = 0; j < M; ++j)
        {
            if (A[i][j] != B[j][i])
            {
                return 0;
            }
        }
    }
    return 1;
}
