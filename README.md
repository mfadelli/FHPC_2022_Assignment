# FHPC_2022_Assignment
# **Repository for the 2022 - 2023 HPC assignment**
Università degli Studi di Trieste\
Data Science and Scientific Computing\
Academic Year 2022-2023

**Disclaimer:** This project was carried out by **Linda Broglio**, **Matteo Fadelli** and **Nicola Perin**. We worked together for the entirety of the time, occasionally working *in parallel* to speed up the process.

This repository contains the proposed solution for the 2022 - 2023 assignment of the course "Foundations of High Performance Computing". 
It consists of the report of the  assignment and two directories with the material produced. In each of these directories there is a readme that contains further information to navigate the content. 

The report can be found here. (da mettere il link)

## Exercise 1

The first exercise consists in a hybrid openMP+MPI implementation of Conway's Game of Life.
Regarding the mandatory instructions 

 - dynamic and static evolution modes were implemented;
 - openMP scalability and MPI strong and weak scalability for the static evolution were assessed using ORFEO.

Additionally, the writing on file was done using raw .pbm format with the magic number P4, and a profiling of the code was attempted.
More information on the structure of the code and the instructions to compile and run it can be found inside the readme of the exercise 1 directory.

All the results are presented in the report and discussed.

## Exercise 2

The second exercise consists in a number of benchmarking tasks of matrix-matrix multiplication that were performed on the ORFEO cluster using the MKL and OBLAS libraries.
The benchmarking were carried out for both AMD (EPYC) and Intel (THIN) nodes, and includes:
 - the measure of scalability obtained increasing the size of the matrix at fixed number of cores (64 for EPYC and 12 for THIN)  for single and double precision. Different threads allocation policies were also tested.
 - The measure of scalability obtained increasing the number of cores at a fixed matrix size for both single and double precision.
 
All the results are presented in the report and discussed.
