

## How to compile the code
The folder contains a Makefile to ease the compiling hence it is enough to load the modules and do a *make*

    module load architecture/AMD
    module load openMPU/4.1.4/gnu/12.2.1
    salloc -n 1 -N1 -p EPYC --time=0:10:0
    srun -n 1 make

## How to run the code
To get the script and the job going it is sufficient to type

    sbatch script1.sh
