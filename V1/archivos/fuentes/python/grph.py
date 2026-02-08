import argparse
import os
import sys
import matplotlib.pyplot as plt

class Punto:
    def __init__(self, x1=0, y1=0):
        self.PosX = x1
        self.PosY = y1
        self.Lista_Segmentos_Candidatos = []
        self.Indice = 0
        self.seq = 0
        self.peso = 1
        self.composicion_puntos = []
        self.puntos_cercanos = []
        self.distancia_media_puntos_cercanos = -1
        self.distancia_media_global = -1

def leer_puntos_desde_archivo(nombre_archivo):
    puntos = []
    with open(nombre_archivo, 'r') as archivo:
        for linea in archivo:
            linea = linea.strip()
            if linea:  # Ignora líneas vacías
                try:
                    x_str, y_str = linea.split(',')
                    x, y = float(x_str), float(y_str)
                    punto = Punto(x,y)
                    punto.Indice = puntos.__len__()
                    puntos.append(punto)
                except ValueError:
                    print(f"Error al procesar la línea: '{linea}'")
    return puntos

def DibujarLista(lista_de_puntos, tipo):
    plt.cla()

    for j, p in enumerate(lista_de_puntos):
        if j > 0:
            if tipo == "G":
                plt.plot([punto_anterior.PosX, p.PosX], [punto_anterior.PosY, p.PosY], color='b')
            plt.scatter(p.PosX, p.PosY, color='r', s=20)
        punto_anterior = p
    if tipo == "G":
        plt.plot([lista_de_puntos[0].PosX, lista_de_puntos[-1].PosX], [lista_de_puntos[0].PosY, lista_de_puntos[-1].PosY], color='b')
    plt.scatter(p.PosX, p.PosY, color='r', s=20)
    plt.show()
    return


def main():
    parser = argparse.ArgumentParser(
        description="Procesamiento con parámetros por línea de comandos",
        allow_abbrev=False
    )
    print("ARGV:", sys.argv)
    # Definimos las opciones (en minúsculas)
    parser.add_argument("-input", required=True, help="Archivo de entrada (con ruta)")
    parser.add_argument("-tipo", default=1, help="Tipo Grafico o puntos (G o P)")

    # Normalizamos argumentos a minúsculas (solo las opciones)
    args = parser.parse_args([a.lower() if a.startswith("-") else a for a in sys.argv[1:]])

    input_file = args.input
    Tipo = args.tipo

    base, _ = os.path.splitext(input_file)

    # Valores iniciales de variables
    lista_de_puntos = []
    lista_de_puntos = leer_puntos_desde_archivo(input_file)
    numero_total_de_puntos = lista_de_puntos.__len__()
    DibujarLista( lista_de_puntos, Tipo)



# Press the green button in the gutter to run the script.
if __name__ == '__main__':
    main()
