# bddreordering
This project mainly implementation by a C program (auto.c) developed by Yanming Liang, the program call the blif2bdd executable from CUDD package: https://github.com/Yu-Utah/ECE-CS-5740-6740-CAD/tree/main/BDDs/blifbdd-package/blif2bdd

This repo include 3 benchmarks in /bench; one source code in /src.

To run this program, there are two ways

Before running program, you may need to change to permission of blif2bdd executable using "chmod +x blif2bdd".


1) compile the auto.c in /src with gcc using: "gcc -o auto auto.c";
   
   then example command: "./auto adder8.pla adder8.blif" to see the result

2) using "chmod +x auto" directly adding permission of the executable already in the repo
   
   then example command: "./auto adder8.pla adder8.blif" to see the result
