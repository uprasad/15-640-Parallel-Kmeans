15-640-Parallel-Kmeans
======================

Sequential and parallel implementations of K-means clustering on DNA sequences using OpenMPI

kgabbita	Krishna Aditya Gabbita
udbhavp		Udbhav Prasad

1. untar the archive lab4_kgabbita-udbhavp.tar.gz

2. use "make" for compiling the C files

3. run "./serial_kmeans_2d <input_path> <input_dimensions> <num_clusters>" for Serial KMeans clustering for 2D points

4. run "./serial_kmeans_dna <input_path> <input_dimensions or dna_length> <num_clusters>" for Serial KMeans clustering for DNA points

5. run "/usr/lib64/openmpi/bin/mpirun -np <num_parallel_machines + 1> -machinefile <machine_addresses_file> ./mpi_kmeans <input_path> <input_dimensions> <num_clusters>"
   for Parallel KMeans clustering for 2D points

6. run "/usr/lib64/openmpi/bin/mpirun -np <num_parallel_machines + 1> -machinefile <machine_addresses_file> ./mpi_kmeans_dna <input_path> <dna_length> <num_clusters>"
   for Parallel KMeans clustering for DNA points

Note: 	<input_dimensions> is the same as <dna_length> used for DNA points clustering
	Use 2 as <input_dimensions> for 2D points clustering
	<input_path> is the path for input file of 2Dpoints/DNA points
	
	Generate data using ./DataGeneratorScripts/randomclustergen/generaterawdata.py for 2D points
	Generate data using ./DataGeneratorScripts/randomclustergen/generateDNAdata.py for DNA points
