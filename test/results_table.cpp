#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <numeric>

struct ResultRow {
    double space = -1.0;
    double time = -1.0;
};

struct AlgoritmoResult {
    std::vector<ResultRow> v5_data;
    std::vector<ResultRow> v_data;
    std::vector<ResultRow> il_data;
};

AlgoritmoResult read_csv_file(const std::string& filepath, const std::string& algo) {
    AlgoritmoResult result;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open file " << filepath << std::endl;
        return result;
    }

    std::string line;
    std::getline(file, line);

    std::vector<ResultRow> all_rows;
    while (std::getline(file, line)) {
        if(line.empty()) continue;
        std::stringstream ss(line);
        std::string cell;
        ResultRow row;
        
        for (int i = 0; std::getline(ss, cell, ','); ++i) {
            if (i == 2) {
                try { row.space = std::stod(cell); } catch (...) { row.space = -1; }
            }
            if (i == 4) {
                try { row.time = std::stod(cell); } catch (...) { row.time = -1; }
            }
        }
        
        if (row.space >= 0 || row.time >= 0) all_rows.push_back(row);
    }
    
    if (algo == "Btrie" && all_rows.size() >= 3) {
        result.v5_data.push_back(all_rows[0]);
        result.il_data.push_back(all_rows[1]);
        result.v_data.push_back(all_rows[2]);
    } else if (algo == "wBtrie" && all_rows.size() >= 8) {
        result.v5_data = std::vector<ResultRow>(all_rows.begin(), all_rows.begin() + 4);
        result.v_data = std::vector<ResultRow>(all_rows.begin() + 4, all_rows.begin() + 8);
    } else if (all_rows.size() >= 6) {
        result.v5_data = std::vector<ResultRow>(all_rows.begin(), all_rows.begin() + 3);
        result.v_data = std::vector<ResultRow>(all_rows.begin() + 3, all_rows.begin() + 6);
    } 

    return result;
}


void generateGnuplotFiles(
    const std::map<std::string, std::map<std::string, AlgoritmoResult>>& results,
    const std::vector<std::string>& datasets,
    const std::vector<std::string>& algorithms,
    const std::string& output_dir
) {
    std::cout << "Generating Gnuplot files..." << std::endl;

    for (const auto& dataset : datasets) {
        std::string dat_file_path = output_dir + "plot_data_" + dataset + ".dat";
        std::ofstream dat_file(dat_file_path);
        if (!dat_file.is_open()) {
            std::cerr << "Error: Could not create data file " << dat_file_path << std::endl;
            continue;
        }

        std::vector<std::string> series_titles;
        for (const auto& algo : algorithms) {
            series_titles.push_back(algo + " (v5)");
            series_titles.push_back(algo + " (v)");
            if (algo == "Btrie") series_titles.push_back(algo + " (il)");
        }

        std::set<double> all_spaces;
        for (const auto& algo : algorithms) {
            auto it_algo = results.find(algo);
            if (it_algo == results.end()) continue;
            auto it_dataset = it_algo->second.find(dataset);
            if (it_dataset == it_algo->second.end()) continue;
            const auto& ar = it_dataset->second;
            for (const auto& r : ar.v5_data) if (r.space >= 0) all_spaces.insert(r.space);
            for (const auto& r : ar.v_data) if (r.space >= 0) all_spaces.insert(r.space);
            for (const auto& r : ar.il_data) if (r.space >= 0) all_spaces.insert(r.space);
        }

        dat_file << "# space";
        for (const auto& title : series_titles) dat_file << "\t" << title;
        dat_file << "\n";

        for (double sp : all_spaces) {
            dat_file << sp;
            for (const auto& algo : algorithms) {
                auto it_algo = results.find(algo);
                if (it_algo == results.end() || it_algo->second.find(dataset) == it_algo->second.end()) {

                    dat_file << "\tNaN\tNaN";
                    if (algo == "Btrie") dat_file << "\tNaN";
                    continue;
                }

                const auto& ar = it_algo->second.at(dataset);

                // v5
                auto it = std::find_if(ar.v5_data.begin(), ar.v5_data.end(),
                                       [&](const ResultRow& r){ return r.space == sp; });
                dat_file << "\t" << (it != ar.v5_data.end() ? it->time : NAN);

                // v
                it = std::find_if(ar.v_data.begin(), ar.v_data.end(),
                                  [&](const ResultRow& r){ return r.space == sp; });
                dat_file << "\t" << (it != ar.v_data.end() ? it->time : NAN);

                // il (only Btrie)
                if (algo == "Btrie") {
                    it = std::find_if(ar.il_data.begin(), ar.il_data.end(),
                                      [&](const ResultRow& r){ return r.space == sp; });
                    dat_file << "\t" << (it != ar.il_data.end() ? it->time : NAN);
                }
            }
            dat_file << "\n";
        }
        dat_file.close();

        std::string gp_file_path = output_dir + "plot_script_" + dataset + ".gp";
        std::ofstream gp_file(gp_file_path);
        if (!gp_file.is_open()) {
            std::cerr << "Error: Could not create Gnuplot script " << gp_file_path << std::endl;
            continue;
        }

        gp_file << "set terminal pngcairo size 1000,700 enhanced font \"Arial,12\"\n";
        gp_file << "set output \"plot_" << dataset << ".png\"\n";
        gp_file << "set title \"" << dataset << "\"\n";
        gp_file << "set xlabel \"Space (bpi)\"\n";
        gp_file << "set ylabel \"Intersection time (milliseconds/query)\"\n";
        gp_file << "set grid\n";
        gp_file << "set key outside right\n";
        gp_file << "set autoscale\n";
        gp_file << "set datafile missing NaN\n\n"; 

        gp_file << "plot ";
        int col = 2;
        for (size_t i = 0; i < series_titles.size(); ++i, ++col) {
            if (i > 0) gp_file << ", \\\n     ";
            gp_file << "'plot_data_" << dataset << ".dat' using 1:" << col
                    << " with linespoints pointtype 7 title \"" << series_titles[i] << "\"";
        }
        gp_file << "\n";

        gp_file.close();
        // std::cout << "Wrote: " << dat_file_path << " and " << gp_file_path
        //           << " (" << series_titles.size() << " series)" << std::endl;
    }

    std::cout << "Gnuplot files generated successfully." << std::endl;
}

void generateGnuplotFilesV(
    const std::map<std::string, std::map<std::string, AlgoritmoResult>>& results,
    const std::vector<std::string>& datasets,
    const std::vector<std::string>& algorithms,
    const std::string& output_dir
) {
    std::cout << "Generating Gnuplot files..." << std::endl;

    for (const auto& dataset : datasets) {
        std::string dat_file_path = output_dir + "plot_data_" + dataset + ".dat";
        std::ofstream dat_file(dat_file_path);
        if (!dat_file.is_open()) {
            std::cerr << "Error: Could not create data file " << dat_file_path << std::endl;
            continue;
        }

        std::vector<std::string> series_titles;
        for (const auto& algo : algorithms) {
            if (algo == "Btrie") {
                series_titles.push_back("Btrie"); 
            } else {
                series_titles.push_back(algo + " (v)");
            }
        }

        std::set<double> all_spaces;
        for (const auto& algo : algorithms) {
            auto it_algo = results.find(algo);
            if (it_algo == results.end()) continue;
            auto it_dataset = it_algo->second.find(dataset);
            if (it_dataset == it_algo->second.end()) continue;
            const auto& ar = it_dataset->second;
            for (const auto& r : ar.v5_data) if (r.space >= 0) all_spaces.insert(r.space);
            for (const auto& r : ar.v_data) if (r.space >= 0) all_spaces.insert(r.space);
            for (const auto& r : ar.il_data) if (r.space >= 0) all_spaces.insert(r.space);
        }

        dat_file << "# space";
        for (const auto& title : series_titles) dat_file << "\t" << title;
        dat_file << "\n";

        for (double sp : all_spaces) {
            dat_file << sp;
            for (const auto& algo : algorithms) {
                auto it_algo = results.find(algo);
                if (it_algo == results.end() || it_algo->second.find(dataset) == it_algo->second.end()) {
                    dat_file << "\tNaN";
                    continue;
                }

                const auto& ar = it_algo->second.at(dataset);

                if (algo == "Btrie") {
                    std::vector<double> btrie_times;

                    // v5
                    auto it = std::find_if(ar.v5_data.begin(), ar.v5_data.end(),
                                           [&](const ResultRow& r){ return r.space == sp; });
                    if (it != ar.v5_data.end()) btrie_times.push_back(it->time);

                    // il
                    it = std::find_if(ar.il_data.begin(), ar.il_data.end(),
                                      [&](const ResultRow& r){ return r.space == sp; });
                    if (it != ar.il_data.end()) btrie_times.push_back(it->time);

                    // v
                    it = std::find_if(ar.v_data.begin(), ar.v_data.end(),
                                      [&](const ResultRow& r){ return r.space == sp; });
                    if (it != ar.v_data.end()) btrie_times.push_back(it->time);

                    if (btrie_times.empty()) {
                        dat_file << "\tNaN";
                    } else {

                        dat_file << "\t" << btrie_times[0];
                    }
                } else {
                    auto it = std::find_if(ar.v_data.begin(), ar.v_data.end(),
                                           [&](const ResultRow& r){ return r.space == sp; });
                    dat_file << "\t" << (it != ar.v_data.end() ? it->time : NAN);
                }
            }
            dat_file << "\n";
        }
        dat_file.close();

        std::string gp_file_path = output_dir + "plot_script_" + dataset + ".gp";
        std::ofstream gp_file(gp_file_path);
        if (!gp_file.is_open()) {
            std::cerr << "Error: Could not create Gnuplot script " << gp_file_path << std::endl;
            continue;
        }

        gp_file << "set terminal pngcairo size 1000,700 enhanced font \"Arial,12\"\n";
        gp_file << "set output \"plot_" << dataset << ".png\"\n";
        gp_file << "set title \"" << dataset << "\"\n";
        gp_file << "set xlabel \"Space (bpi)\"\n";
        gp_file << "set ylabel \"Intersection time (milliseconds/query)\"\n";
        gp_file << "set grid\n";
        gp_file << "set key outside right\n";
        gp_file << "set autoscale\n";
        gp_file << "set datafile missing NaN\n\n";

        gp_file << "plot ";
        int col = 2;
        for (size_t i = 0; i < series_titles.size(); ++i, ++col) {
            if (i > 0) gp_file << ", \\\n     ";
            gp_file << "'plot_data_" << dataset << ".dat' using 1:" << col
                    << " with linespoints pointtype 7 title \"" << series_titles[i] << "\"";
        }
        gp_file << "\n";

        gp_file.close();
    }

    std::cout << "Gnuplot files generated successfully." << std::endl;
}


int main() {
    std::vector<std::string> datasets = {"Gov2", "ClueWeb09", "CC-News"};
    std::vector<std::string> algorithms = {"Btrie", "wBtrie", "x2WRBtrie", "x3WRBtrie", "x3WTRBtrie", "NPx2WRBtrie"};
    std::string input_dir = "../outputs/";
    std::string output_file = "../results_table.md"; 
    
    std::map<std::string, std::map<std::string, AlgoritmoResult>> results;

    std::cout << "Reading result files..." << std::endl;
    for (const auto& algo : algorithms) {
        for (const auto& dataset : datasets) {
            std::string filename = algo + "_" + dataset + ".csv";
            std::string filepath = input_dir + filename;
            AlgoritmoResult data = read_csv_file(filepath, algo);
            
            if (!data.v5_data.empty() || !data.v_data.empty() || !data.il_data.empty()) {
                results[algo][dataset] = data;
                // std::cout << "Loaded: " << filename 
                //           << " (v5=" << data.v5_data.size() 
                //           << ", v=" << data.v_data.size() 
                //           << ", il=" << data.il_data.size() << ")" << std::endl;
            } else {
                std::cout << "No valid data in: " << filename << std::endl;
            }
        }
    }

    std::cout << "Generating the table..." << std::endl;
    std::stringstream table_ss;

    size_t max_algo_length = 0;
    for (const auto& algo : algorithms) {
        
            max_algo_length = std::max(max_algo_length, algo.length() + 5); 
    }

    table_ss << "| " << std::setw(max_algo_length) << std::left << "Data Structure" << " |";
    for (const auto& ds : datasets) table_ss << std::setw(23) << std::left << ds << "|";
    table_ss << "\n";

    table_ss << "| " << std::setw(max_algo_length) << "" << " |";
    for (size_t i = 0; i < datasets.size(); ++i) table_ss << "Space      Time |";
    table_ss << "\n";

    table_ss << "|:" << std::setw(max_algo_length-1) << std::left << "---|" ;
    for (size_t i = 0; i < datasets.size(); ++i) table_ss << ":---:|";
    table_ss << "\n";

    
    for (const auto& algo : algorithms) {
        if (results.find(algo) == results.end()) continue;

        if (algo != "Btrie") {
            size_t rep = algo == "wBtrie" ? 4 : 3;
            for (size_t i = 0; i < rep; ++i) {
                table_ss << "| " << std::setw(max_algo_length) << std::left << (algo + " (v5)") << " |";
                for (const auto& dataset : datasets) {
                    if (results[algo].count(dataset) && results[algo][dataset].v5_data.size() > i) {
                        table_ss << std::fixed << std::setprecision(2)
                                 << std::setw(8) << results[algo][dataset].v5_data[i].space << " "
                                 << std::setw(8) << results[algo][dataset].v5_data[i].time << " |";
                    } else table_ss << std::setw(9) << "N/A" << std::setw(8) << "N/A" << " |";
                }
                table_ss << std::endl;

                table_ss << "| " << std::setw(max_algo_length) << std::left << (algo + " (v)") << " |";
                for (const auto& dataset : datasets) {
                    if (results[algo].count(dataset) && results[algo][dataset].v_data.size() > i) {
                        table_ss << std::fixed << std::setprecision(2)
                                 << std::setw(8) << results[algo][dataset].v_data[i].space << " "
                                 << std::setw(8) << results[algo][dataset].v_data[i].time << " |";
                    } else table_ss << std::setw(9) << "N/A" << std::setw(8) << "N/A" << " |";
                }
                table_ss << std::endl;
            }
        } else {
            // Btrie
            table_ss << "| " << std::setw(max_algo_length) << std::left << (algo + " (v5)") << " |";
            for (const auto& dataset : datasets) {
                if (results[algo].count(dataset) && results[algo][dataset].v5_data.size() > 0) {
                    table_ss << std::fixed << std::setprecision(2)
                             << std::setw(8) << results[algo][dataset].v5_data[0].space << " "
                             << std::setw(8) << results[algo][dataset].v5_data[0].time << " |";
                } else table_ss << std::setw(9) << "N/A" << std::setw(8) << "N/A" << " |";
            }
            table_ss << std::endl;

            table_ss << "| " << std::setw(max_algo_length) << std::left << (algo + " (il)") << " |";
            for (const auto& dataset : datasets) {
                if (results[algo].count(dataset) && results[algo][dataset].il_data.size() > 0) {
                    table_ss << std::fixed << std::setprecision(2)
                             << std::setw(8) << results[algo][dataset].il_data[0].space << " "
                             << std::setw(8) << results[algo][dataset].il_data[0].time << " |";
                } else table_ss << std::setw(9) << "N/A" << std::setw(8) << "N/A" << " |";
            }
            table_ss << std::endl;

            table_ss << "| " << std::setw(max_algo_length) << std::left << (algo + " (v)") << " |";
            for (const auto& dataset : datasets) {
                if (results[algo].count(dataset) && results[algo][dataset].v_data.size() > 0) {
                    table_ss << std::fixed << std::setprecision(2)
                             << std::setw(8) << results[algo][dataset].v_data[0].space << " "
                             << std::setw(8) << results[algo][dataset].v_data[0].time << " |";
                } else table_ss << std::setw(9) << "N/A" << std::setw(8) << "N/A" << " |";
            }
            table_ss << std::endl;
        }

        table_ss << "|---|";
        for (size_t i = 0; i < datasets.size(); ++i) table_ss << "---|";
        table_ss << std::endl;
    }

    std::ofstream out_file(output_file);
    if (out_file.is_open()) {
        out_file << table_ss.str();
        out_file.close();
        std::cout << "Table generated successfully in '" << output_file << "'!" << std::endl;
    } else {
        std::cerr << "Error: Could not create output file '" << output_file << "'" << std::endl;
    }
    
    generateGnuplotFilesV(results, datasets, algorithms, input_dir);
    
    return 0;
}

