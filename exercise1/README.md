# Exercise 1 


## How to compile the code
The folder contains a Makefile to ease the compiling hence it is enough to load the modules and do a *make*

    $ module load architecture/AMD
    $ module load openMPI/4.1.4/gnu/12.2.1
    $ salloc -n 1 -N1 -p EPYC --time=0:10:0
    $ srun -n 1 make

## How to run the code
Example:

    $ salloc -N 2 -p THIN --ntasks 2 --cpus-per-task=64 --time=1:50:0
    $ export OMP_NUM_THREADS=64
and then *$ srun Conway.x* with the appropriate arguments.<br>
**Setting the arguments:** <br>
To know what the arguments are and get a brief guide, type
   
    $ srun Conway.x -h
The arguments are the following

| Argument  | Usage  | 
| :------------ |:---------------| 
| -i file <br> -i random | Initialize grid loading a pattern from an ASCII .pbm file <br> Initialize the grid randomly |
| -r      | Run the program  |
| -k <number> | Set the size of the grid |
| -e 0 <br> -e 1 | Select the ordered evolution <br> Select the static evolution |
| -f <filename>| Set the name of the file to be loaded |
| -n <number>|Set the number of iterations|
| -s <number>|Choose every how many iterations the grid gets saved|
| -h|Print the manual|

