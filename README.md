# bddreordering
This project is mainly implementated by a C program (auto.c) developed by Yanming Liang, the program call the blif2bdd executable from CUDD package: https://github.com/Yu-Utah/ECE-CS-5740-6740-CAD/tree/main/BDDs/blifbdd-package/blif2bdd

This repo include 3 benchmarks in /bench; one source code in /src.

This program takes 2 input fils: .pla and .blif

To run this program, there are two ways

Before running program, you may need to change to permission of blif2bdd executable using "chmod +x blif2bdd".


1) compile the auto.c in /src with gcc using: "gcc -o auto auto.c";
   
   then example command: "./auto adder8.pla adder8.blif" to see the result

2) using "chmod +x auto" directly adding permission of the executable already in the repo
   
   then example command: "./auto adder8.pla adder8.blif" to see the result

Note: To see different trial result, you may need to run the program with same input again in the case of shuffling function effects the size.

This project is tested in Ubuntu 24.04.1 LTS
