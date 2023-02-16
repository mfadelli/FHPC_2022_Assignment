# Exercise 2

## Content
The results and the graphs can be found in the corresponding folder, pertaining either the weak or the strong scalability. Commentary and analysis are on the report.

## Test script
One can check everything works by using the script provided:

    $ module load architecture/AMD
    $ module load openMPU/4.1.4/gnu/12.2.1
    $ salloc -n 1 -N1 -p EPYC --time=0:10:0
    $ srun -n 1 make
    $ sbatch script.sh
