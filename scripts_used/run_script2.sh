#python DataGeneratorScripts/randomclustergen/generaterawdata.py -c 1000 -p 10 -o DataGeneratorScripts/randomclustergen/dna1000x10x30 -v 30
#python DataGeneratorScripts/randomclustergen/generaterawdata.py -c 1000 -p 100 -o DataGeneratorScripts/randomclustergen/dna1000x100x30 -v 30
#python DataGeneratorScripts/randomclustergen/generaterawdata.py -c 1000 -p 1000 -o DataGeneratorScripts/randomclustergen/dna1000x1000x30 -v 30
#python DataGeneratorScripts/randomclustergen/generaterawdata.py -c 100 -p 100000 -o DataGeneratorScripts/randomclustergen/dna100x100000x30 -v 30

time /usr/lib64/openmpi/bin/mpirun -np 3 -machinefile machines_transfer mpi_kmeans_dna DataGeneratorScripts/randomclustergen/dna1000x10x30 30 1000
time /usr/lib64/openmpi/bin/mpirun -np 3 -machinefile machines_transfer mpi_kmeans_dna DataGeneratorScripts/randomclustergen/dna1000x100x30 30 1000
time /usr/lib64/openmpi/bin/mpirun -np 3 -machinefile machines_transfer mpi_kmeans_dna DataGeneratorScripts/randomclustergen/dna1000x1000x30 30 1000
#time /usr/lib64/openmpi/bin/mpirun -np 10 -machinefile machines_transfer mpi_kmeans_dna DataGeneratorScripts/randomclustergen/dna100x100000x30 30 100
