set terminal pngcairo size 1000,700 enhanced font "Arial,12"
set output "plot_Gov2.png"
set title "Gov2"
set xlabel "Space (bpi)"
set ylabel "Intersection time (milliseconds/query)"
set grid
set key outside right
set autoscale
set datafile missing NaN

plot 'plot_data_Gov2.dat' using 1:2 with linespoints pointtype 7 title "Btrie", \
     'plot_data_Gov2.dat' using 1:3 with linespoints pointtype 7 title "wBtrie (v)", \
     'plot_data_Gov2.dat' using 1:4 with linespoints pointtype 7 title "x2WRBtrie (v)", \
     'plot_data_Gov2.dat' using 1:5 with linespoints pointtype 7 title "x3WRBtrie (v)", \
     'plot_data_Gov2.dat' using 1:6 with linespoints pointtype 7 title "x3WTRBtrie (v)", \
     'plot_data_Gov2.dat' using 1:7 with linespoints pointtype 7 title "NPx2WRBtrie (v)"
