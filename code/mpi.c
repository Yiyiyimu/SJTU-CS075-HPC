#include <stdio.h>
#include <mpi.h>
#include <math.h>

/*
 * The Basic Idea: 
 *     Tan(Pi/4) = 1.0. 
 *     (I.e. The Legs of a right triangle with 2, 45 degree angles are equal)
 *     This Means that arctan(1.0) = Pi/4
 *     SO - We need to compute arctan(1.0)
 *         Arctan(x) has a known derivative of x'/(1+x*x) = 1.0/(1+x*x)
 *         By integrating this, we can approximate Arctan(1.0)
 *         We'll perform a numerical integration to get an 
 *         approximation of Pi/4, then multiply by 4.0
 *         to get an approximation of Pi.
 */

/* Compute the Derivative of ArcTan*/
double dx_arctan(double x)
{    
    return (1.0 / (1.0 + x*x));
}


int main(int argc, char *argv[])
{
    int n;
    double PI25DT = 3.141592653589793238462643;
    double mypi, h, pi, i, sum, x, a, startwtime, endwtime;
    int myid, numprocs, resultlen;
    char name[MPI_MAX_PROCESSOR_NAME] ; 

    MPI_Init(&argc,&argv);

    /* Do this FIRST (to avoid error)  */    
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
    MPI_Get_processor_name(name, &resultlen); 

    /* Show all processor IDs */
    printf("This is Process-%d/%d running on %s \n",
           myid,numprocs,name);
    fflush(0);
    MPI_Barrier(MPI_COMM_WORLD);

    /* Get Runtime information */
    if(myid == 0) 
    {
        printf("This program uses %d processes\n", numprocs);
	
       /* n = atoi(argv[1]);    total number of evaluation points */

	 n = 100000000;

        printf("The number of intervals = %d \n", n); 

        startwtime = MPI_Wtime();
    }

    /* Share intervals with other processors */
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    sum = 0.0;
    h   = 1.0/n;
    /* Compute and Sum the "Heights" of each bar of the integration */
    for (i=myid+0.5; i<n; i+=numprocs)
    {
        sum += dx_arctan(i*h);
    }
    /* Multiply by the "Widths" of each bar and 
       4.0 (arctan(1)=Pi/4) */
    mypi = 4.0*h*sum;

    /* Consolidate and Sum Results */
    MPI_Reduce(&mypi, &pi, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (myid == 0)
    {
        endwtime = MPI_Wtime();
        printf("pi is approximately %.16f, Error is %.16f\n",
                pi, fabs(pi - PI25DT)); 
        printf("wall clock time = %f\n", endwtime-startwtime); 
    }
    MPI_Finalize();
    return 0;
}