#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <float.h>

void print_points(double** points, int num_points, int input_dim) {
	int i, j;

	for (i=0; i<num_points; i++) {
		printf("( ");
		for (j=0; j<input_dim; j++) {
			fprintf(stdout, "%lf ", points[i][j]);
		}
		printf(" )\n");
	}
}

void find_centroids(double** centroids, int num_clusters, double** points, int* membership, int num_points, int input_dim) {
	int i, j;

	for (i=0; i<num_clusters; ++i) {
		for (j=0; j<input_dim; ++j) {
			centroids[i][j] = 0;
		}
	}

	int*** counts = (int***)malloc(num_clusters*sizeof(int **));
	for(i=0; i<num_clusters; i++) {
		counts[i] = (int**)malloc(input_dim*sizeof(int *));
		for(j=0; j<input_dim; j++) {
			counts[i][j] = (int*)calloc(4, sizeof(int));
		}
	}

	//counts for each dimension for each point
	for (i=0; i<num_points; ++i) {
		for(j=0; j<input_dim; j++) {
			int idx = (int) points[i][j];
			counts[membership[i]-1][j][idx]++;
		}
	}

	//find max of each dimension of each cluster
	for (i=0; i<num_clusters; ++i) {
		for (j=0; j<input_dim; ++j) {
			int max_val = counts[i][j][0];
			int max_idx = 0;
			
			int k = 0;
			for(k=1; k<4; k++) {
				if(counts[i][j][k] > max_val) {
					max_val = counts[i][j][k];
					max_idx = k;
				}
			}
			
			centroids[i][j] = max_idx;
		}
	}
}

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
		if(x[i] != y[i])
			dist += 1;
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

	FILE *fp = fopen(argv[1], "r");
	int num_points = 0;

	/* Going through the file to find the number of points */
	while (!feof(fp)) {
		//double t1, t2;
		//fscanf(fp, "%lf,%lf\n", &t1, &t2);
		int i, t1;
		for(i=0; i<input_dim; i++) {
			if(i==0)
				fscanf(fp, "%lf", &t1);
			else
				fscanf(fp, ",%lf", &t1);
		}
		fscanf(fp, "\n");
		num_points++;
	}

	/* Initialize array for points */
	double **points = (double**)malloc(num_points*sizeof(double*));
	int i;
	for (i=0; i<num_points; ++i) {
		points[i] = (double*)malloc(input_dim*sizeof(double));
	}

	/* Send file pointer to beginning of file */
	rewind(fp);
	
	/* Read the points into an array */
	i = 0;
	int j = 0;
	while (!feof(fp)) {
		for (j=0; j<input_dim; ++j) {
			if (j == 0)
				fscanf(fp, "%lf", &points[i][j]);
			else
				fscanf(fp, ",%lf", &points[i][j]);
		}
		fscanf(fp, "\n");
		i++;
	}

	//print_points(points, num_points, input_dim);
	
	/* Arrays to keep track of point memberships */
	int *membership_old = (int*)malloc(num_points*sizeof(int));
	int *membership_new = (int*)malloc(num_points*sizeof(int));

	for (i=0; i<num_points; ++i) {
		membership_new[i] = 1;
	}

	/* Initialize cluster centroids */
	int *centroid_idx = (int*)malloc(num_clusters*sizeof(int));
	init_cluster_centroids(centroid_idx, num_clusters, num_points);

	double **centroids = (double**)malloc(num_clusters*sizeof(double*));
	for (i=0; i<num_clusters; ++i) {
		centroids[i] = (double*)malloc(input_dim*sizeof(double));
		for (j=0; j<input_dim; ++j) {
			centroids[i][j] = points[centroid_idx[i]][j];
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
			double min_dist = distance(points[i], centroids[0], input_dim);
			int min_dist_idx = 1;
			/* For each point */
			for (j=0; j<num_clusters; ++j) {
				/* For each cluster, find distance */
				double dist = distance(points[i], centroids[j], input_dim);
				
				if (dist < min_dist) {
					min_dist_idx = j+1;
					min_dist = dist;
				}
			}

			/* Give the point its new cluster */
			membership_new[i] = min_dist_idx;
		}

		find_centroids(centroids, num_clusters, points, membership_new, num_points, input_dim);

		if(num_iter%10 == 0) {
			printf("%d\n",num_iter);
		}

	} while (num_changed_members(membership_old, membership_new, num_points) != 0);

	printf("Finished in %d iterations\n", num_iter);
	printf("The final centroids are:\n");
	print_points(centroids, num_clusters, input_dim);

	for(i=0; i<num_points; i++) {
		printf("%d ",membership_new[i]);
	}
	printf("\n");
	
	return 0;
}
