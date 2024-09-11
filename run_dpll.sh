#!/bin/bash

# Path to the folder containing the .cnf files
folder="$HOME/Desktop/SAT_SOLVER/TESTCASES/UUF"

# Loop through all .cnf files in the folder
for file in "$folder"/*.cnf; do
  # Run the dpll program on each file
  ./dpll "$file"
done
