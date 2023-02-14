# Exercise 1 


## How to compile the code
The folder contains a Makefile to ease the compiling hence it is enough to load the modules and do a *make*

    module load architecture/AMD
    module load openMPU/4.1.4/gnu/12.2.1
    salloc -n 1 -N1 -p EPYC --time=0:10:0
    srun -n 1 make

## How to run the code
To get the script and the job going it is sufficient to type

    sbatch script1.sh

## Setting the arguments
To know what are the arguments and get a brief guide, type
   
    ./Conway.x -h
The arguments are the following

| Argument  | Usage  | 
| :------------ |:---------------| 
| -i file <br> -i random | Initializes grid loading a pattern from an ASCII .pbm file <br> Initializes the grid randomly |
| -r      | Run the program  |
| -k <number> | Set the size of the grid |
| -e 0 <br> -e 1 | Select the ordered evolution <br> Select the static evolution |
| -f <filename>| Set the name of the file to be loaded |
| -n <number>|Set the number of iterations|
| -s <number>|Choose every how many iterations the grid gets saved|
| -h|Print the manual|

