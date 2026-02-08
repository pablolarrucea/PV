import sys
import copy
import time
import random
import datetime
import math
import itertools
import argparse


def generar_puntos_a_archivo(nombre_archivo, num_puntos, max_coord):
    with open(nombre_archivo, 'w') as archivo:
        for _ in range(num_puntos):
            x = random.uniform(0, max_coord)
            y = random.uniform(0, max_coord)
            archivo.write(f"{x:.4f},{y:.4f}\n")


def main():
    # Parser de argumentos de línea de comandos
    parser = argparse.ArgumentParser(description='Generar puntos aleatorios en un archivo')
    parser.add_argument('-puntos', type=int, default=10,
                        help='Número total de puntos a generar (default: 10)')
    parser.add_argument('-max', type=float, default=100.0,
                        help='Máxima distancia en cada eje (default: 100)')
    parser.add_argument('-path', type=str, default='./',
                        help='Directorio donde guardar el archivo (default: ./)')

    args = parser.parse_args()

    # Asignar valores desde los argumentos
    numero_total_de_puntos = args.puntos
    maxima_distinct_eje = args.max
    path = args.path

    # Asegurar que el path termina con /
    if not path.endswith('/'):
        path += '/'

    # Generar nombre de archivo
    ahora = datetime.datetime.now()
    nombre_archivo = f"{path}puntos_{numero_total_de_puntos}_x_{numero_total_de_puntos}_{ahora.strftime('%Y%m%d_%H%M%S_%f')}.txt"

    print(f"Generando {numero_total_de_puntos} puntos...")
    print(f"Máximo por eje: {maxima_distinct_eje}")
    print(f"Archivo de salida: {nombre_archivo}")

    generar_puntos_a_archivo(nombre_archivo, numero_total_de_puntos, maxima_distinct_eje)

    print(f"Archivo generado: {nombre_archivo}")


if __name__ == '__main__':
    main()
