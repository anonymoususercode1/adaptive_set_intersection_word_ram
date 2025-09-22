#!/bin/bash
# --------------------
# Script to run all the tests with different parameters
# runall
# --------------------
OUTPUT_DIR=$(pwd) 

if [ "$#" -lt 1 ]; then
    echo "Error: You must pass the path to the datasets folder."
    echo "Uso: $0 <path_to_datasets_folder>"
    exit 1
fi

DATASETS_FOLDER=$1

cd scripts/
GR='\033[0;32m'
NC='\033[0m'
echo -e "${GR}Running all tests...${NC}"
echo -e "${GR}1- Running the Gov2 dataset...${NC}"
# time -p (
    bash test_Btrie.sh "${DATASETS_FOLDER}/gov2.docs" ../queries/Gov2/1mq.txt --min_size 4096 --parallel t > "../outputs/Btrie_Gov2.csv"
    bash test_wBtrie.sh "${DATASETS_FOLDER}/gov2.docs" ../queries/Gov2/1mq.txt --min_size 4096 --parallel t > "../outputs/wBtrie_Gov2.csv"
    # bash test_x2WBtrie.sh "${DATASETS_FOLDER}/gov2.docs" ../queries/Gov2/1mq.txt --min_size 4096 --parallel t > "../outputs/x2WBtrie_Gov2.csv"
    bash test_x2WRBtrie.sh "${DATASETS_FOLDER}/gov2.docs" ../queries/Gov2/1mq.txt --min_size 4096 --parallel t > "../outputs/x2WRBtrie_Gov2.csv"
    # bash test_x2WTRBtrie.sh "${DATASETS_FOLDER}/gov2.docs" ../queries/Gov2/1mq.txt --min_size 4096 --parallel t > "../outputs/x2WTRBtrie_Gov2.csv"
    bash test_x3WRBtrie.sh "${DATASETS_FOLDER}/gov2.docs" ../queries/Gov2/1mq.txt --min_size 4096 --parallel t > "../outputs/x3WRBtrie_Gov2.csv"
    bash test_x3WTRBtrie.sh "${DATASETS_FOLDER}/gov2.docs" ../queries/Gov2/1mq.txt --min_size 4096 --parallel t > "../outputs/x3WTRBtrie_Gov2.csv"
    bash test_x2WRBtrie.sh "${DATASETS_FOLDER}/gov2.docs" ../queries/Gov2/1mq.txt --min_size 4096 --parallel f > "../outputs/NPx2WRBtrie_Gov2.csv"
# )
echo -e "${GR}2- Running the CC-news dataset...${NC}"
# time -p (
    bash test_Btrie.sh "${DATASETS_FOLDER}/cc-news_4096.docs" ../queries/CC-news/queries.txt --min_size 4096 --parallel t > "../outputs/Btrie_CC-News.csv"
    bash test_wBtrie.sh "${DATASETS_FOLDER}/cc-news_4096.docs" ../queries/CC-news/queries.txt --min_size 4096 --parallel t > "../outputs/wBtrie_CC-News.csv"
#     # bash test_x2WBtrie.sh "${DATASETS_FOLDER}/cc-news_4096.docs" ../queries/CC-news/queries.txt --min_size 4096 --parallel t > "../outputs/x2WBtrie_CC-News.csv"
    bash test_x2WRBtrie.sh "${DATASETS_FOLDER}/cc-news_4096.docs" ../queries/CC-news/queries.txt --min_size 4096 --parallel t > "../outputs/x2WRBtrie_CC-News.csv"
#     # bash test_x2WTRBtrie.sh "${DATASETS_FOLDER}/cc-news_4096.docs" ../queries/CC-news/queries.txt --min_size 4096 --parallel t > "../outputs/x2WTRBtrie_CC-News.csv"
    bash test_x3WRBtrie.sh "${DATASETS_FOLDER}/cc-news_4096.docs" ../queries/CC-news/queries.txt --min_size 4096 --parallel t > "../outputs/x3WRBtrie_CC-News.csv"
    bash test_x3WTRBtrie.sh "${DATASETS_FOLDER}/cc-news_4096.docs" ../queries/CC-news/queries.txt --min_size 4096 --parallel t > "../outputs/x3WTRBtrie_CC-News.csv"
    bash test_x2WRBtrie.sh "${DATASETS_FOLDER}/cc-news_4096.docs" ../queries/CC-news/queries.txt --min_size 4096 --parallel f > "../outputs/NPx2WRBtrie_CC-News.csv"
# )
echo -e "${GR}3- Running the Clueweb09 dataset...${NC}"
# time -p (
    bash test_Btrie.sh "${DATASETS_FOLDER}/clueweb09.docs" ../queries/Clueweb09/1mq.txt --min_size 4096 --parallel t > "../outputs/Btrie_ClueWeb09.csv"
    bash test_wBtrie.sh "${DATASETS_FOLDER}/clueweb09.docs" ../queries/Clueweb09/1mq.txt --min_size 4096 --parallel t > "../outputs/wBtrie_ClueWeb09.csv"
#     # bash test_x2WBtrie.sh "${DATASETS_FOLDER}/clueweb09.docs" ../queries/Clueweb09/1mq.txt --min_size 4096 --parallel t > "../outputs/x2WBtrie_ClueWeb09.csv"
    bash test_x2WRBtrie.sh "${DATASETS_FOLDER}/clueweb09.docs" ../queries/Clueweb09/1mq.txt --min_size 4096 --parallel t > "../outputs/x2WRBtrie_ClueWeb09.csv"
#     # bash test_x2WTRBtrie.sh "${DATASETS_FOLDER}/clueweb09.docs" ../queries/Clueweb09/1mq.txt --min_size 4096 --parallel t > "../outputs/x2WTRBtrie_ClueWeb09.csv"
    bash test_x3WRBtrie.sh "${DATASETS_FOLDER}/clueweb09.docs" ../queries/Clueweb09/1mq.txt --min_size 4096 --parallel t > "../outputs/x3WRBtrie_ClueWeb09.csv"
    bash test_x3WTRBtrie.sh "${DATASETS_FOLDER}/clueweb09.docs" ../queries/Clueweb09/1mq.txt --min_size 4096 --parallel t > "../outputs/x3WTRBtrie_ClueWeb09.csv"
    bash test_x2WRBtrie.sh "${DATASETS_FOLDER}/clueweb09.docs" ../queries/Clueweb09/1mq.txt --min_size 4096 --parallel f > "../outputs/NPx2WRBtrie_ClueWeb09.csv"
# )
echo -e "${GR}4-Building the results table...${NC}"
./../build/test/results_table


echo -e "${GR}5-Creating plots in $OUTPUT_DIR.${NC}"

cd ../outputs || { echo "No se puede acceder a ../outputs"; exit 1; }

# gnuplot plot_script_Gov2.gp
# gnuplot plot_script_ClueWeb09.gp
# gnuplot plot_script_CC-News.gp
for gp in plot_script_*.gp; do
    base=$(echo "$gp" | sed 's/plot_script_\(.*\)\.gp/\1/')
    gnuplot <(sed "s|set output .*|set output '${OUTPUT_DIR}/plot_${base}.png'|" "$gp")
done


echo -e "All plots generated successfully!"

