#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>

struct ResultRow {
    double space;
    double time;
};

struct AlgoritmoResult {
    std::vector<ResultRow> v5_data;
    std::vector<ResultRow> v_data;
};

AlgoritmoResult read_csv_file(const std::string& filepath) {
    AlgoritmoResult result;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open file" << filepath << std::endl;
        return result;
    }

    std::string line;
    std::getline(file, line); 

    std::vector<ResultRow> all_rows;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;
        ResultRow row;
        
        for (int i = 0; std::getline(ss, cell, ','); ++i) {
            if (i == 2) {
                row.space = std::stod(cell);
            }
            if (i == 4) {
                row.time = std::stod(cell);
            }
        }
        all_rows.push_back(row);
    }
    
    if (all_rows.size() >= 6) {
        result.v5_data = std::vector<ResultRow>(all_rows.begin(), all_rows.begin() + 3);
        result.v_data = std::vector<ResultRow>(all_rows.begin() + 3, all_rows.begin() + 6);
    }

    return result;
}

int main() {
    // Definir los datasets y algoritmos
    std::vector<std::string> datasets = {"Gov2", "ClueWeb09", "CC-News"};
    std::vector<std::string> algorithms = {"wBtrie", "x2WBtrie", "x2WRBtrie", "x2WTRBtrie", "x3WRBtrie", "x3WTRBtrie"};
    std::string input_dir = "outputs/";
    std::string output_file = "tabla_resultados.md";
    
    std::map<std::string, std::map<std::string, AlgoritmoResult>> results;

    std::cout << "Reading result files..." << std::endl;
    for (const auto& algo : algorithms) {
        for (const auto& dataset : datasets) {
            std::string filename = algo + "_" + dataset + ".csv";
            std::string filepath = input_dir + filename;
            AlgoritmoResult data = read_csv_file(filepath);
            
            if (!data.v5_data.empty() || !data.v_data.empty()) {
                results[algo][dataset] = data;
            }
        }
    }

    // Generar la tabla en un string
    std::cout << "Generating the table..." << std::endl;
    std::stringstream table_ss;

    // Encabezado
    table_ss << "| Data Structure |"
             << std::setw(23) << std::left << "Gov2" << "|"
             << std::setw(23) << std::left << "ClueWeb09" << "|"
             << std::setw(23) << std::left << "CC-News" << "|" << std::endl;
    
    table_ss << "|                | "
             << "Space      Time | "
             << "Space      Time | "
             << "Space      Time |" << std::endl;
    
    table_ss << "|:---|:---:|:---:|:---:|" << std::endl;

    for (const auto& algo : algorithms) {
        if (results.find(algo) == results.end()) {
            continue;
        }
        
        for (size_t i = 0; i < 3; ++i) {
            table_ss << "| " << algo << " (v5) |";
            for (const auto& dataset : datasets) {
                if (results[algo].count(dataset) && results[algo][dataset].v5_data.size() > i) {
                    table_ss << std::fixed << std::setprecision(2) << std::setw(8) << results[algo][dataset].v5_data[i].space << " "
                             << std::setw(8) << results[algo][dataset].v5_data[i].time << " |";
                } else {
                    table_ss << std::setw(9) << "N/A" << std::setw(8) << "N/A" << " |";
                }
            }
            table_ss << std::endl;
        }

        // Variante v
        for (size_t i = 0; i < 3; ++i) {
            table_ss << "| " << algo << " (v) |";
            for (const auto& dataset : datasets) {
                if (results[algo].count(dataset) && results[algo][dataset].v_data.size() > i) {
                    table_ss << std::fixed << std::setprecision(2) << std::setw(8) << results[algo][dataset].v_data[i].space << " "
                             << std::setw(8) << results[algo][dataset].v_data[i].time << " |";
                } else {
                    table_ss << std::setw(9) << "N/A" << std::setw(8) << "N/A" << " |";
                }
            }
            table_ss << std::endl;
        }
        table_ss << "|---|---|---|---|" << std::endl;
    }

    std::ofstream out_file(output_file);
    if (out_file.is_open()) {
        out_file << table_ss.str();
        out_file.close();
        std::cout << "\nTable generated successfully in'" << output_file << "'!" << std::endl;
    } else {
        std::cerr << "Error: Could not create output file'" << output_file << "'" << std::endl;
    }

    return 0;
}