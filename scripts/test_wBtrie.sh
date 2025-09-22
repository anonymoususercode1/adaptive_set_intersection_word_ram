if [ "$#" -lt 4 ]; then
    echo "Error: You must pass at least 5 parameters."
    echo "Use: $0 <path_to_data_file> <path_to_query_file> --min_size <min_size> --parallel <t|f>"
    exit 1
fi

data_sets_file=$1
query_file=$2
min_size=0
parallel=""
i=3

while [ $i -le $# ]; do
    case ${!i} in
        --min_size)
            i=$((i + 1))
            min_size=${!i}
            ;;
        --parallel)
            i=$((i + 1))
            parallel=${!i}
            ;;
        *)
            echo "Error: Unknown parameter ${!i}"
            exit 1
            ;;
    esac
    i=$((i + 1))
done


echo "sets_read,total_elements,avg_size_bits_per_element,number_queries,avg_time"
./../build/test/test_wBtries "$data_sets_file" "$query_file" --rank v5 --wsize 8 --min_size "$min_size" --parallel "$parallel"
./../build/test/test_wBtries "$data_sets_file" "$query_file" --rank v5 --wsize 16 --min_size "$min_size" --parallel "$parallel"
./../build/test/test_wBtries "$data_sets_file" "$query_file" --rank v5 --wsize 32 --min_size "$min_size" --parallel "$parallel"
./../build/test/test_wBtries "$data_sets_file" "$query_file" --rank v5 --wsize 64 --min_size "$min_size" --parallel "$parallel"
./../build/test/test_wBtries "$data_sets_file" "$query_file" --rank v --wsize 8 --min_size "$min_size" --parallel "$parallel"
./../build/test/test_wBtries "$data_sets_file" "$query_file" --rank v --wsize 16 --min_size "$min_size" --parallel "$parallel"
./../build/test/test_wBtries "$data_sets_file" "$query_file" --rank v --wsize 32 --min_size "$min_size" --parallel "$parallel"
./../build/test/test_wBtries "$data_sets_file" "$query_file" --rank v --wsize 64 --min_size "$min_size" --parallel "$parallel"