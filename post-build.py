Import("env")
import os

def generate_coverage(source, target, env):
    # Verificar si las pruebas se ejecutaron correctamente
    if not os.path.exists('.pio/build/native'):
        print("No se encontró el directorio de compilación nativo. Las pruebas podrían haber fallado.")
        return
    
    # Verificar si existen archivos de cobertura
    gc_files = []
    for root, dirs, files in os.walk('.pio/build/native'):
        for file in files:
            if file.endswith('.gcda'):
                gc_files.append(os.path.join(root, file))
    
    if not gc_files:
        print("No se encontraron archivos de cobertura (.gcda). Las pruebas podrían no haberse ejecutado correctamente.")
        return
    
    print(f"Encontrados {len(gc_files)} archivos de cobertura. Generando informe...")
    
    # Crear directorio de informe
    coverage_dir = 'coverage_report'
    if os.path.exists(coverage_dir):
        for root, dirs, files in os.walk(coverage_dir, topdown=False):
            for name in files:
                os.remove(os.path.join(root, name))
            for name in dirs:
                os.rmdir(os.path.join(root, name))
    os.makedirs(coverage_dir, exist_ok=True)
    
    # Capturar datos de cobertura
    os.system("lcov --capture --directory .pio/build/native --output-file coverage.info")
    
    # Filtrar directorios del sistema y bibliotecas
    os.system("lcov --remove coverage.info '/usr/*' '*.h' --output-file coverage.info")
    
    # Generar informe HTML
    os.system(f"genhtml coverage.info --output-directory {coverage_dir}")
    
    print(f"Informe de cobertura generado en: {coverage_dir}/index.html")

env.AddPostAction("test", generate_coverage)