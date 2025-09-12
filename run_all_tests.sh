#!/bin/bash
# --------------------
# Script to run all the tests with different parameters
# runall
# --------------------

# Se cronometra todo el bloque de comandos
time -p (
    cd scripts/
    GR='\033[0;32m'
    NC='\033[0m'
    echo -e "${GR}Running all tests...${NC}"

    # bash test_wBtrie.sh "/mnt/c/JOSE M/data_sets/gov2sample20000.bin" ../queries/Gov2/1mq.txt --min_size 4096 --parallel t > "../outputs/wBtrie_Gov2.csv"
    # bash test_x2WBtrie.sh "/mnt/c/JOSE M/data_sets/gov2sample20000.bin" ../queries/Gov2/1mq.txt --min_size 4096 --parallel t > "../outputs/x2WBtrie_Gov2.csv"
    # bash test_x2WRBtrie.sh "/mnt/c/JOSE M/data_sets/gov2sample20000.bin" ../queries/Gov2/1mq.txt --min_size 4096 --parallel t > "../outputs/x2WRBtrie_Gov2.csv"
    # bash test_x2WTRBtrie.sh "/mnt/c/JOSE M/data_sets/gov2sample20000.bin" ../queries/Gov2/1mq.txt --min_size 4096 --parallel t > "../outputs/x2WTRBtrie_Gov2.csv"
    # bash test_x3WRBtrie.sh "/mnt/c/JOSE M/data_sets/gov2sample20000.bin" ../queries/Gov2/1mq.txt --min_size 4096 --parallel t > "../outputs/x3WRBtrie_Gov2.csv"
    # bash test_x3WTRBtrie.sh "/mnt/c/JOSE M/data_sets/gov2sample20000.bin" ../queries/Gov2/1mq.txt --min_size 4096 --parallel t > "../outputs/x3WTRBtrie_Gov2.csv"
)
./build/test/table_results

