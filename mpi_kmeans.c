#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>

#define MASTER 0               /* taskid of first task */
#define FROM_MASTER 1          /* setting a message type */
#define FROM_WORKER 2          /* setting a message type */

//print points
void print_points(double* points, int num_points, int input_dim) {
        printf("num_points: %d \t input_dim: %d\n", num_points, input_dim);
        int i, j;

        for (i=0; i<num_points; i++) {
                printf("(");
                for (j=0; j<input_dim; j++) {
			int idx = (i*input_dim) + j;
                        fprintf(stdout, "%lf\t", points[idx]);
                }
                printf(")\n");
        }
}

//estimate new centroids
void find_centroids(double* centroids, int num_clusters, double* points, int* membership, int num_points, int input_dim) {
        int i, j;

        for (i=0; i<num_clusters; ++i) {
                for (j=0; j<input_dim; ++j) {
                        int idx = (i*input_dim) + j;
			centroids[idx] = 0;
                }
        }

        int* counts = (int*)calloc(num_clusters, sizeof(int));

        for (i=0; i<num_points; ++i) {
                for (j=0; j<input_dim; ++j) {
                        int c_idx = ((membership[i]-1)*input_dim) + j;
			int p_idx = (i*input_dim) + j;
			centroids[c_idx] += points[p_idx];
                }
                counts[membership[i]-1]++;
        }

        for (i=0; i<num_clusters; ++i) {
                for (j=0; j<input_dim; ++j) {
                        if (counts[i] != 0) {
                                int idx = (i*input_dim) + j;
				centroids[idx] /= counts[i];
                        }
                }
        }
}

//returns number of changes from membership_old to membership_new
int num_changed_members(int* membership_old, int* membership_new, int num_points) {
        int i;
        int num_changed = 0;
        for (i=0; i<num_points; ++i) {
                if (membership_old[i] != membership_new[i]) {
                        num_changed++;
                }
        }

        return num_changed;
}

void init_cluster_centroids(int* centroids, int num_clusters, int num_points) {
        srand(time(NULL));

        int i;
        int* points_shuffle = (int*)malloc(num_points*sizeof(int));
        for (i=0; i<num_points; i++) {
                points_shuffle[i] = i;
        }

        for (i=0; i<num_points; i++) {
                int temp = points_shuffle[i];
                int randomIndex = rand()%num_points;

                points_shuffle[i] = points_shuffle[randomIndex];
                points_shuffle[randomIndex] = temp;
        }

        for (i=0; i<num_clusters; ++i) {
                centroids[i] = points_shuffle[i];
        }
}

double distance(double* x, double* y, int input_dim) {
        /* Squared Euclidean, but can be changed to anything */
        int i;
        double dist = 0;

        for (i=0; i<input_dim; i++) {
                dist += (x[i] - y[i])*(x[i] - y[i]);
        }

        return dist;
}

int main(int argc, char **argv) {
	if (argc < 4) {
                printf("Usage: %s <input_path> <input_dimensions> <num_clusters>\n", argv[0]);
                exit(1);
        }

	/* Parse dimension of input */
        int input_dim = atoi(argv[2]);

        /* Parse number of clusters */
        int num_clusters = atoi(argv[3]);

        /* Cluster specific data structures */
        int num_points;                 //no. of points in dataset
        double *points;                //data structure for points
        int *membership_new;    //membership for clusters
        double *centroids = (double*)malloc((num_clusters*input_dim)*sizeof(double)); //centroids of clusters

	FILE *fp = fopen(argv[1], "r");
        num_points = 0;

        /* Going through the file to find the number of points */
        while (!feof(fp)) {
        	double t1, t2;
                fscanf(fp, "%lf,%lf\n", &t1, &t2);
                num_points++;
        }

        /* Initialize array for points */
        points = (double*)malloc((num_points*input_dim)*sizeof(double));

        /* Send file pointer to beginning of file */
        rewind(fp);

	/* Read the points into an array */
        int i = 0, j = 0;
        while (!feof(fp)) {
        	for (j=0; j<input_dim; ++j) {
                	int idx = (i*input_dim) + j;
			if (j == 0)
                        	fscanf(fp, "%lf", &points[idx]);
                        else
                        	fscanf(fp, ",%lf", &points[idx]);
                 }
                 fscanf(fp, "\n");
                 i++;
        }

	//print_points(points, num_points, input_dim);	
	
	/* Arrays to keep track of point memberships */
        int *membership_old = (int*)malloc(num_points*sizeof(int));
        membership_new = (int*)malloc(num_points*sizeof(int));

        /* initialize to dummy cluster values */
        for (i=0; i<num_points; ++i) {
	        membership_new[i] = 1;
        }

        /* Initialize cluster centroids */
        int *centroid_idx = (int*)malloc(num_clusters*sizeof(int));
        init_cluster_centroids(centroid_idx, num_clusters, num_points);
	
	for (i=0; i<num_clusters; ++i) {
        	for (j=0; j<input_dim; ++j) {
                	int c_idx = (i*input_dim) + j;
			int p_idx = (centroid_idx[i]*input_dim) + j;
			centroids[c_idx] = points[p_idx];
                }
       	}

        print_points(centroids, num_clusters, input_dim);

        /* The K-means loop */
        int num_iter = 0;
        do {
        	num_iter++;
                for (i=0; i<num_points; i++) {
                	membership_old[i] = membership_new[i];
                }

		/* Assign points to clusters */
                for (i=0; i<num_points; ++i) {
			int p_idx = i*input_dim;
                	double min_dist = distance((points+p_idx), (centroids), input_dim);
			int min_dist_idx = 1;
                        /* For each point */
                        for (j=0; j<num_clusters; ++j) {
                        	/* For each cluster, find distance */
                                int c_idx = j*input_dim;
				double dist = distance((points+p_idx), (centroids+c_idx), input_dim);

                                if (dist < min_dist) {
                                	min_dist_idx = j+1;
                                        min_dist = dist;
                                }
                        }

                        /* Give the point its new cluster */
                        membership_new[i] = min_dist_idx;
		}

                find_centroids(centroids, num_clusters, points, membership_new, num_points, input_dim);

	} while (num_changed_members(membership_old, membership_new, num_points) != 0);		

	printf("Finished in %d iterations\n", num_iter);
        printf("The final centroids are:\n");
        print_points(centroids, num_clusters, input_dim);

	return 0;
}
