#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <iomanip>

// Estructura para almacenar los datos de cada fila
struct ResultRow {
    double space;
    double time;
};

// Estructura para almacenar todos los resultados por variante de rank (v5 y v)
struct AlgoritmoResult {
    std::vector<ResultRow> v5_data;
    std::vector<ResultRow> v_data;
};

// Función para leer un archivo CSV y extraer los datos
AlgoritmoResult read_csv_file(const std::string& filepath) {
    AlgoritmoResult result;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Advertencia: No se pudo abrir el archivo " << filepath << std::endl;
        return result;
    }

    std::string line;
    std::getline(file, line); // Leer y descartar la línea de encabezado

    std::vector<ResultRow> all_rows;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string cell;
        ResultRow row;
        
        // Asumiendo que las columnas de interés son la 3 y la 5
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
    
    // Asumiendo que las primeras 3 filas son para v5 y las siguientes 3 para v
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
    std::string input_dir = "outputs/"; // La carpeta donde están los archivos CSV
    std::string output_file = "tabla_resultados.md";
    
    // Estructura para almacenar todos los datos leídos
    std::map<std::string, std::map<std::string, AlgoritmoResult>> results;

    std::cout << "Leyendo archivos de resultados..." << std::endl;
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
    std::cout << "Generando la tabla..." << std::endl;
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

    // Llenar la tabla con los datos
    for (const auto& algo : algorithms) {
        if (results.find(algo) == results.end()) {
            continue;
        }
        
        // Variante v5
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

    // Escribir la tabla al archivo de salida
    std::ofstream out_file(output_file);
    if (out_file.is_open()) {
        out_file << table_ss.str();
        out_file.close();
        std::cout << "\n¡Tabla generada con éxito en '" << output_file << "'!" << std::endl;
    } else {
        std::cerr << "Error: No se pudo crear el archivo de salida '" << output_file << "'" << std::endl;
    }

    return 0;
}