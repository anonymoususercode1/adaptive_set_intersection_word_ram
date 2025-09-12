import csv
import os
from collections import defaultdict
import textwrap

def generate_results_table(input_dir, output_file):
    """
    Lee archivos de resultados de experimentos y genera una tabla consolidada.
    """
    # 1. Definir los datasets y algoritmos
    datasets = ['Gov2', 'ClueWeb09', 'CC-News']
    algorithms = ['wBtrie', 'x2WBtrie', 'x2WRBtrie', 'x2WTRBtrie', 'x3WRBtrie', 'x3WTRBtrie']
    
    # Estructura para almacenar los datos
    results = defaultdict(lambda: defaultdict(dict))

    # 2. Leer los datos de cada fichero
    print("Leyendo archivos de resultados...")
    for dataset in datasets:
        for algo in algorithms:
            file_name = f"{algo}_{dataset}.csv"
            file_path = os.path.join(input_dir, file_name)

            if not os.path.exists(file_path):
                # print(f"Advertencia: No se encontró el archivo {file_path}. Se omite.")
                continue

            try:
                with open(file_path, 'r') as f:
                    reader = csv.DictReader(f)
                    
                    data_rows = list(reader)
                    
                    # Asumiendo 6 filas por archivo: 3 para v5 y 3 para v
                    if len(data_rows) >= 6:
                        # Extraer los datos de las columnas de interés
                        v5_data = [(float(row['avg_size_bits_per_element']), float(row['avg_time'])) for row in data_rows[:3]]
                        v_data = [(float(row['avg_size_bits_per_element']), float(row['avg_time'])) for row in data_rows[3:6]]
                        
                        results[algo][dataset]['v5'] = v5_data
                        results[algo][dataset]['v'] = v_data
            except Exception as e:
                print(f"Error al procesar el archivo {file_path}: {e}")

    # 3. Generar la tabla en un string
    print("Generando la tabla...")
    output_lines = []

    # Construir el encabezado de la tabla
    header = f"| Data Structure | {' &nbsp; '*10} | {' &nbsp; '*10} | {' &nbsp; '*10} |\n"
    header += f"|                | {'Gov2':^21} | {'ClueWeb09':^21} | {'CC-News':^21} |\n"
    header += f"|                | {'Space':>9} {'Time':>10} | {'Space':>9} {'Time':>10} | {'Space':>9} {'Time':>10} |"
    output_lines.append(header)
    output_lines.append("|:---|:---:|:---:|:---:|")

    # Llenar la tabla con los datos
    for algo in algorithms:
        if algo not in results:
            continue
        
        # Datos para la variante v5
        v5_data = [results[algo][ds].get('v5', [('N/A', 'N/A')] * 3) for ds in datasets]
        for i in range(3):
            line = f"| {algo} (v5) |"
            for j, dataset in enumerate(datasets):
                space, time = v5_data[j][i]
                line += f" {space:8.2f} {time:8.2f} |" if isinstance(space, float) else f" {'N/A':>8} {'N/A':>8} |"
            output_lines.append(line)

        # Datos para la variante v
        v_data = [results[algo][ds].get('v', [('N/A', 'N/A')] * 3) for ds in datasets]
        for i in range(3):
            line = f"| {algo} (v) |"
            for j, dataset in enumerate(datasets):
                space, time = v_data[j][i]
                line += f" {space:8.2f} {time:8.2f} |" if isinstance(space, float) else f" {'N/A':>8} {'N/A':>8} |"
            output_lines.append(line)
        output_lines.append("|---|---|---|---|")

    # 4. Escribir la tabla en el archivo de salida
    with open(output_file, 'w') as f:
        f.write("\n".join(output_lines))

    print(f"\n¡Tabla generada con éxito en '{output_file}'!")

# --- Uso del script ---
if __name__ == "__main__":
    # Asegúrate de que esta ruta apunte a la carpeta donde están tus 15 archivos CSV
    # Por ejemplo, si los archivos están en una carpeta llamada 'outputs' en el mismo directorio que el script
    input_directory = './outputs'
    output_file = 'tabla_resultados.md'
    
    generate_results_table(input_directory, output_file)