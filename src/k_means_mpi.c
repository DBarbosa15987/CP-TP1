#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <mpi.h>

typedef struct Cluster{

    float x;
    float y;
    float nr_pontos;

}Cluster;

void inicializa(float *x, float *y, Cluster *clusters, int N, int K){
  
    srand(10);

    for(int i = 0; i < N; i++) {

        x[i] = (float) rand() / RAND_MAX;
        y[i] = (float) rand() / RAND_MAX;

    }

    for(int i = 0; i < K; i++) {
        clusters[i].x = x[i];
        clusters[i].y = y[i];
        clusters[i].nr_pontos = 0;
    }

}


int k_meansAux(float *x, float *y, Cluster *clusters, int N, int K, int rank,int iterations,int size){

    Cluster *centroid_novo = malloc(sizeof(Cluster)*K);
    float *x_centroid = malloc(sizeof(float)*K);
    float *y_centroid = malloc(sizeof(float)*K);
    float *nr_pontos = malloc(sizeof(float)*K);
    float *x_centroid_reduced = malloc(sizeof(float)*K);
    float *y_centroid_reduced = malloc(sizeof(float)*K);
    float *nr_pontos_reduced = malloc(sizeof(float)*K);
    float distance, min;
    int indMin = 0, muda = 0;   
    
    /* 
    Every single process, including root, executes this section:
    Each one works on a fraction of the input and then the root process merges it all
    The size of these fractions depends on the number os processes used
    */
    MPI_Status status;
    int elems_per_proc = N/size;
    int index = rank * elems_per_proc;
    int lim = (rank + 1) * elems_per_proc;

    for(int i = 0; i < K; i++){
        x_centroid[i] = 0;
        y_centroid[i] = 0;
        nr_pontos[i] = 0;
        x_centroid_reduced[i] = 0;
        y_centroid_reduced[i] = 0;
        nr_pontos_reduced[i] = 0;
    }
  

    for(int i = index; i < lim; i++){
        
        indMin = 0;
        min = ((x[i] - clusters[0].x) * (x[i] - clusters[0].x)) + ((y[i] - clusters[0].y) * (y[i] - clusters[0].y));
        
        for(int j=1;j<K;j++){

            distance = ((x[i] - clusters[j].x) * (x[i] - clusters[j].x)) + ((y[i] - clusters[j].y) * (y[i] - clusters[j].y));

            if(distance < min){
                min = distance;
                indMin = j;
            }

        }

        nr_pontos[indMin] += 1;
        x_centroid[indMin] += x[i];
        y_centroid[indMin] += y[i];
        
    }

    MPI_Allreduce(nr_pontos,nr_pontos_reduced,K,MPI_FLOAT,MPI_SUM,MPI_COMM_WORLD);
    MPI_Allreduce(x_centroid,x_centroid_reduced,K,MPI_FLOAT,MPI_SUM,MPI_COMM_WORLD);
    MPI_Allreduce(y_centroid,y_centroid_reduced,K,MPI_FLOAT,MPI_SUM,MPI_COMM_WORLD);

    

    /* Calculation of the new centroid */
    for(int i = 0; i < K; i++){

        centroid_novo[i].x = x_centroid_reduced[i]/nr_pontos_reduced[i];
        centroid_novo[i].y = y_centroid_reduced[i]/nr_pontos_reduced[i]; 
        centroid_novo[i].nr_pontos = nr_pontos_reduced[i];

    }

    /* Check if the centroids have moved to determine if another iteration is necessary */
    for(int i = 0; i<K; i++) 
        muda = muda || (centroid_novo[i].x!=clusters[i].x || centroid_novo[i].y!=clusters[i].y);

    /* Update the old centroid to the newly calculated one */
    for(int i=0;i<K;i++){
        clusters[i].x = centroid_novo[i].x;
        clusters[i].y = centroid_novo[i].y;
        clusters[i].nr_pontos = centroid_novo[i].nr_pontos;
        
    }

    return muda;

}

int k_means(float *x, float *y, Cluster *clusters, int N, int K, int rank, int size){

    int iterations = 0;

    while(iterations<=20){
        
        k_meansAux(x,y,clusters,N,K,rank,iterations,size);
        iterations++;

    }
    MPI_Finalize();
    return iterations-1;

}

int main(int argc, char* argv[]){

    float *x, *y;
    Cluster *clusters;
    int iter;
    int N = atoi(argv[1]);
    int K = atoi(argv[2]);
    x = malloc(sizeof(float)*N);
    y = malloc(sizeof(float)*N);
    clusters = malloc(sizeof(Cluster)*K);
    
    
    inicializa(x,y,clusters, N, K);

    
    //Mpi
    int rank,size;
    MPI_Init(&argc, &argv);//
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    iter = k_means(x,y,clusters,N,K,rank,size);


    if(rank==0){
        printf("N = %d, K = %d\n",N,K);
        
        for(int i=0;i<K;i++){
            printf("Center: (%.3f,%.3f), Size: %d\n",clusters[i].x,clusters[i].y,(int) clusters[i].nr_pontos);
        }
        printf("Iterations: %d\n",iter);
    }
    return 0;

}

