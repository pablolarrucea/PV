#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <random>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <limits>
#include <chrono>
#include <omp.h>
#include <unordered_set>
#include <cstring>

using namespace std;

class Punto {
public:
    double PosX;
    double PosY;
    int Indice;

    Punto(double x1 = 0, double y1 = 0)
        : PosX(x1), PosY(y1), Indice(0) {}
};

// Estructura para los argumentos
struct Argumentos {
    int hilos = 1;
    string input_file;
    string output_file;
    string log_file;
};

inline double distancia_entre_dos_puntos(const Punto& p1, const Punto& p2) {
    double dx = p1.PosX - p2.PosX;
    double dy = p1.PosY - p2.PosY;
    return sqrt(dx * dx + dy * dy);
}

// Función para convertir a minúsculas
string toLower(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// Parser de argumentos de línea de comandos
Argumentos parsear_argumentos(int argc, char* argv[]) {
    Argumentos args;

    cout << "ARGV: ";
    for (int i = 0; i < argc; i++) {
        cout << argv[i] << " ";
    }
    cout << endl;

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        string arg_lower = toLower(arg);

        if (arg_lower == "-hilos" && i + 1 < argc) {
            args.hilos = stoi(argv[++i]);
        }
        else if (arg_lower == "-input" && i + 1 < argc) {
            args.input_file = argv[++i];
        }
        else if (arg_lower == "-output" && i + 1 < argc) {
            args.output_file = argv[++i];
        }
        else if (arg_lower == "-log" && i + 1 < argc) {
            args.log_file = argv[++i];
        }
    }

    // Validaciones
    if (args.input_file.empty()) {
        cerr << "Error: Se requiere el parámetro -input" << endl;
        cerr << "Uso: " << argv[0] << " -input <archivo> [-hilos <n>] [-output <archivo>] [-log <archivo>]" << endl;
        exit(1);
    }

    if (args.hilos < 1) {
        cerr << "Error: El número de hilos debe ser >= 1" << endl;
        exit(1);
    }

    // Generar nombres por defecto si no se especifican
    if (args.output_file.empty() || args.log_file.empty()) {
        size_t lastdot = args.input_file.find_last_of(".");
        string base = (lastdot == string::npos) ? args.input_file : args.input_file.substr(0, lastdot);

        if (args.output_file.empty()) {
            args.output_file = base + ".log";
        }
        if (args.log_file.empty()) {
            args.log_file = base + ".trc";
        }
    }

    return args;
}

vector<Punto> leer_puntos_desde_archivo(const string& nombre_archivo) {
    vector<Punto> puntos;
    ifstream archivo(nombre_archivo);
    string linea;

    if (!archivo.is_open()) {
        cerr << "Error al abrir el archivo: " << nombre_archivo << endl;
        return puntos;
    }

    while (getline(archivo, linea)) {
        // Eliminar espacios en blanco y caracteres de retorno de carro
        linea.erase(remove_if(linea.begin(), linea.end(),
                    [](unsigned char c) { return isspace(c); }), linea.end());

        if (linea.empty()) continue;

        size_t pos = linea.find(',');
        if (pos != string::npos) {
            try {
                double x = stod(linea.substr(0, pos));
                double y = stod(linea.substr(pos + 1));
                Punto punto(x, y);
                punto.Indice = puntos.size();
                puntos.push_back(punto);
            } catch (const exception& e) {
                cerr << "Error al procesar la línea: " << linea << endl;
            }
        }
    }

    archivo.close();

    // Invertir para que coincida con el orden de Python
    //reverse(puntos.begin(), puntos.end());

    // Reajustar los índices después de invertir
    for (size_t i = 0; i < puntos.size(); i++) {
        puntos[i].Indice = i;
    }

    return puntos;
}

void generar_puntos_a_archivo(const string& nombre_archivo, int num_puntos, double max_coord) {
    ofstream archivo(nombre_archivo);
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(0.0, max_coord);

    for (int i = 0; i < num_puntos; i++) {
        double x = dis(gen);
        double y = dis(gen);
        archivo << fixed << setprecision(4) << x << "," << y << "\n";
    }

    archivo.close();
}

vector<vector<double>> GeneraMatrizDeDistancias(const vector<Punto>& lista_de_puntos) {
    int tam = lista_de_puntos.size();
    vector<vector<double>> matriz(tam, vector<double>(tam, 0.0));

    // Equivalente a Python: MatrizDeDististancias[i, j] = distancia(elemento_x, elemento_y)
    // donde j es el índice de elemento_x y i es el índice de elemento_y
    for (int j = 0; j < tam; j++) {
        for (int i = 0; i < tam; i++) {
            if (i != j) {
                matriz[i][j] = distancia_entre_dos_puntos(lista_de_puntos[j], lista_de_puntos[i]);
            }
        }
    }
    return matriz;
}

pair<vector<vector<double>>, vector<vector<int>>> GenerarMatrizDeColumnasOrdenadas(
    const vector<vector<double>>& MatrizDeDistanciasOriginal) {

    int tam = MatrizDeDistanciasOriginal.size();
    vector<vector<double>> MatrizOrdenada(tam, vector<double>(tam));
    vector<vector<int>> MatrizIndices(tam, vector<int>(tam));

    // Procesar cada columna
    for (int col = 0; col < tam; col++) {
        vector<pair<double, int>> columna;
        columna.reserve(tam);

        // Extraer la columna con sus índices originales
        for (int row = 0; row < tam; row++) {
            columna.push_back({MatrizDeDistanciasOriginal[row][col], row});
        }

        // Ordenar por distancia
        sort(columna.begin(), columna.end());

        // Guardar valores ordenados y sus índices originales
        for (int row = 0; row < tam; row++) {
            MatrizOrdenada[row][col] = columna[row].first;
            MatrizIndices[row][col] = columna[row].second;
        }
    }

    return {MatrizOrdenada, MatrizIndices};
}

inline vector<int> OrdenarLista(const vector<int>& ListaAproximada,
                         const vector<vector<int>>& MatrizDeIndicesOriginalesColumnasOrdenadas) {
    int NumeroDeFilasColumnas = ListaAproximada.size();
    vector<int> ListaOrdenada;
    ListaOrdenada.reserve(NumeroDeFilasColumnas);
    int SgteValor = 0;

    for (int i = 0; i < NumeroDeFilasColumnas; i++) {
        int Valor = MatrizDeIndicesOriginalesColumnasOrdenadas[ListaAproximada[SgteValor]][SgteValor];
        ListaOrdenada.push_back(Valor);
        SgteValor = Valor;
    }

    return ListaOrdenada;
}

inline vector<int> DesordenarLista(const vector<int>& ListaOrdenada) {
    int NumeroDeFilasColumnas = ListaOrdenada.size();
    vector<int> ListaDesordenada(NumeroDeFilasColumnas);
    int Valor = 0;

    for (int i = 0; i < NumeroDeFilasColumnas; i++) {
        ListaDesordenada[Valor] = ListaOrdenada[i];
        Valor = ListaOrdenada[i];
    }

    return ListaDesordenada;
}

vector<int> DeListaDesordenadaAListaAproximada(
    const vector<vector<int>>& MatrizDeIndicesOriginalesColumnasOrdenadas,
    const vector<int>& ListaDesordenada) {

    int NumeroDeFilasColumnas = ListaDesordenada.size();
    vector<int> ListaAproximada(NumeroDeFilasColumnas);

    for (int Columna = 0; Columna < NumeroDeFilasColumnas; Columna++) {
        int Valor = ListaDesordenada[Columna];
        for (int i = 1; i < NumeroDeFilasColumnas; i++) {
            if (MatrizDeIndicesOriginalesColumnasOrdenadas[i][Columna] == Valor) {
                ListaAproximada[Columna] = i;
                break;
            }
        }
    }

    return ListaAproximada;
}

inline double CalcularDistanciaDeListaFilasIncluidas(
    const vector<vector<double>>& MatrizDeDistanciasOriginal,
    const vector<int>& ListaFilasIncluidas) {

    double distancia = 0;
    int tam = MatrizDeDistanciasOriginal.size();

    for (int j = 0; j < tam; j++) {
        distancia += MatrizDeDistanciasOriginal[ListaFilasIncluidas[j]][j];
    }

    return distancia;
}

pair<vector<int>, double> GenerarListaAproximada(
    const vector<vector<double>>& MatrizDeDistanciasOriginal,
    const vector<vector<double>>& MatriZDeColumnasOrdenadasOrig,
    const vector<vector<int>>& MatrizDeIndicesOriginalesColumnasOrdenadasOrig,
    int Columna) {

    int NumeroDeFilasColumnas = MatriZDeColumnasOrdenadasOrig.size();
    vector<int> ListaAproximada;
    vector<int> ListaDeFilasIncluidas;

    ListaAproximada.reserve(NumeroDeFilasColumnas);
    ListaDeFilasIncluidas.reserve(NumeroDeFilasColumnas);

    vector<vector<double>> MatriZDeColumnasOrdenadas = MatriZDeColumnasOrdenadasOrig;
    vector<vector<int>> MatrizDeIndicesOriginalesColumnasOrdenadas = MatrizDeIndicesOriginalesColumnasOrdenadasOrig;

    // Rotar matrices si Columna != 0
    if (Columna != 0) {
        for (int i = 0; i < NumeroDeFilasColumnas; i++) {
            rotate(MatriZDeColumnasOrdenadas[i].begin(),
                   MatriZDeColumnasOrdenadas[i].begin() + Columna,
                   MatriZDeColumnasOrdenadas[i].end());
            rotate(MatrizDeIndicesOriginalesColumnasOrdenadas[i].begin(),
                   MatrizDeIndicesOriginalesColumnasOrdenadas[i].begin() + Columna,
                   MatrizDeIndicesOriginalesColumnasOrdenadas[i].end());
        }

        for (int i = 0; i < NumeroDeFilasColumnas; i++) {
            for (int j = 0; j < NumeroDeFilasColumnas; j++) {
                MatrizDeIndicesOriginalesColumnasOrdenadas[i][j] =
                    (MatrizDeIndicesOriginalesColumnasOrdenadas[i][j] - Columna + NumeroDeFilasColumnas)
                    % NumeroDeFilasColumnas;
            }
        }
    }

    // Matrices de control de bucles
    vector<int> MatrizSaltosInvertida(NumeroDeFilasColumnas, -1);
    vector<int> MatrizSaltos(NumeroDeFilasColumnas, -1);

    // Construcción inicial de la lista aproximada
    for (int j = 0; j < NumeroDeFilasColumnas; j++) {
        bool EncontradoCandidato = false;
        int Indice = j;

        for (int i = 0; i < NumeroDeFilasColumnas; i++) {
            if (MatriZDeColumnasOrdenadas[i][Indice] != 0) {
                int candidato = MatrizDeIndicesOriginalesColumnasOrdenadas[i][Indice];

                // Verificar si el candidato ya está incluido
                if (find(ListaDeFilasIncluidas.begin(), ListaDeFilasIncluidas.end(), candidato)
                    == ListaDeFilasIncluidas.end()) {

                    ListaDeFilasIncluidas.push_back(candidato);
                    ListaAproximada.push_back(i);

                    // Comprobación de existencia de bucles
                    bool ExisteBucle = false;

                    if (j < NumeroDeFilasColumnas) {
                        MatrizSaltosInvertida[ListaDeFilasIncluidas[j]] = j;

                        if (MatrizSaltos[j] == -1) {
                            MatrizSaltos[j] = ListaDeFilasIncluidas[j];
                        } else {
                            j = -1;  // Error según Python
                        }

                        int Referencia = j;
                        int IndiceReferencia = MatrizSaltosInvertida[MatrizSaltos[j]];

                        for (int k = 0; k <= j; k++) {
                            if (MatrizSaltosInvertida[IndiceReferencia] != -1) {
                                if ((k > 0) && (MatrizSaltosInvertida[IndiceReferencia] == Referencia)) {
                                    if (k < (NumeroDeFilasColumnas - 1)) {
                                        MatrizSaltosInvertida[IndiceReferencia] = -1;
                                        MatrizSaltos[Referencia] = -1;
                                        ListaDeFilasIncluidas.pop_back();
                                        ListaAproximada.pop_back();
                                        ExisteBucle = true;
                                    }
                                    break;
                                } else {
                                    IndiceReferencia = MatrizSaltosInvertida[IndiceReferencia];
                                }
                            } else {
                                break;
                            }
                        }

                        if (!ExisteBucle) {
                            EncontradoCandidato = true;
                            break;
                        }
                    } else {
                        EncontradoCandidato = true;
                        break;
                    }
                }
            }
            if (EncontradoCandidato) break;
        }
    }

    // Optimización con inversiones de segmentos
    vector<int> ListaOrdenada = OrdenarLista(ListaAproximada, MatrizDeIndicesOriginalesColumnasOrdenadas);
    bool EncontradoAlgoMejor = true;
    double MejorDistancia = 0;
    double LongitudLista1 = -1;
    vector<int> ListaDesordenada1;

    while (EncontradoAlgoMejor) {
        EncontradoAlgoMejor = false;

        for (int k = NumeroDeFilasColumnas - 1; k >= 2; k--) {
            for (int i = k - 2; i >= 0; i--) {
                if (!((k == NumeroDeFilasColumnas - 1) && (i == 0))) {
                    vector<int> Lista1 = ListaOrdenada;
                    vector<int> Lista3 = ListaOrdenada;

                    int D2 = k;
                    int O2 = k - 1;
                    int D1 = i;
                    int O1 = i - 1;

                    reverse(Lista3.begin() + D1, Lista3.begin() + O2 + 1);

                    if (LongitudLista1 == -1) {
                        ListaDesordenada1 = DesordenarLista(Lista1);
                        LongitudLista1 = CalcularDistanciaDeListaFilasIncluidas(
                            MatrizDeDistanciasOriginal, ListaDesordenada1);
                        MejorDistancia = LongitudLista1;
                        EncontradoAlgoMejor = true;
                    } else {
                        double SegmentoExistente2 = MatrizDeDistanciasOriginal[Lista1[k]][Lista1[k - 1]];
                        double SegmentoExistente1 = MatrizDeDistanciasOriginal[Lista1[i]][Lista1[i - 1]];
                        double SegmentoNuevo2 = MatrizDeDistanciasOriginal[Lista1[k - 1]][Lista1[i - 1]];
                        double SegmentoNuevo1 = MatrizDeDistanciasOriginal[Lista1[k]][Lista1[i]];

                        double LongitudLista3 = LongitudLista1 - SegmentoExistente2 - SegmentoExistente1
                                              + SegmentoNuevo2 + SegmentoNuevo1;

                        if (LongitudLista1 > LongitudLista3) {
                            LongitudLista1 = LongitudLista3;
                            vector<int> ListaDesordenada3 = DesordenarLista(Lista3);
                            ListaDesordenada1 = ListaDesordenada3;
                            ListaAproximada = DeListaDesordenadaAListaAproximada(
                                MatrizDeIndicesOriginalesColumnasOrdenadas, ListaDesordenada3);
                            ListaOrdenada = OrdenarLista(ListaAproximada, MatrizDeIndicesOriginalesColumnasOrdenadas);
                            MejorDistancia = LongitudLista3;
                            EncontradoAlgoMejor = true;
                        }
                    }
                }

                if (EncontradoAlgoMejor) break;
            }
            if (EncontradoAlgoMejor) break;
        }
    }

    // Retornar ListaOrdenada como en Python
    return {ListaOrdenada, MejorDistancia};
}

string obtener_hora_actual() {
    auto now = chrono::system_clock::now();
    auto now_time_t = chrono::system_clock::to_time_t(now);
    auto now_ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;

    tm* ltm = localtime(&now_time_t);
    stringstream ss;
    ss << setfill('0') << setw(2) << ltm->tm_hour << ":"
       << setw(2) << ltm->tm_min << ":"
       << setw(2) << ltm->tm_sec << "."
       << setw(3) << now_ms.count();
    return ss.str();
}

int main(int argc, char* argv[]) {
    // Parsear argumentos
    Argumentos args = parsear_argumentos(argc, argv);

    cout << "Configuración:" << endl;
    cout << "  Hilos: " << args.hilos << endl;
    cout << "  Input: " << args.input_file << endl;
    cout << "  Output: " << args.output_file << endl;
    cout << "  Log: " << args.log_file << endl;
    cout << endl;

    // Cargar puntos desde archivo
    vector<Punto> lista_de_puntos = leer_puntos_desde_archivo(args.input_file);

    if (lista_de_puntos.empty()) {
        cerr << "No se pudieron cargar los puntos desde: " << args.input_file << endl;
        return 1;
    }

    int numero_total_de_puntos = lista_de_puntos.size();
    cout << "Puntos cargados: " << numero_total_de_puntos << endl;

    // Generar matriz de distancias
    auto MatrizDeDistanciasOriginal = GeneraMatrizDeDistancias(lista_de_puntos);

    // Generar matriz de columnas ordenadas
    auto [MatrizDeColumnasOrdenadas, MatrizDeIndicesOriginalesColumnasOrdenadas] =
        GenerarMatrizDeColumnasOrdenadas(MatrizDeDistanciasOriginal);

    // Configurar número de hilos
    omp_set_num_threads(args.hilos);

    // Abrir archivo de log
    ofstream archivoLog(args.log_file);
    if (!archivoLog.is_open()) {
        cerr << "Error al crear archivo de log: " << args.log_file << endl;
        return 1;
    }

    // Generar lista de aproximaciones
    vector<int> MejorLista;
    double DistanciaMinima = -1;

    string HoraInicio = "Hora Inicio: " + obtener_hora_actual() + "\n";
    archivoLog << "----------------------\n";
    archivoLog << HoraInicio;
    archivoLog << "----------------------\n";

    cout << "Procesando columnas con " << args.hilos << " hilo(s)..." << endl;

    // Vectores para almacenar resultados de cada hilo
    vector<vector<int>> ResultadosListas(numero_total_de_puntos);
    vector<double> ResultadosDistancias(numero_total_de_puntos);

    // Paralelizar el bucle principal si se especifica más de 1 hilo
    if (args.hilos > 1) {
        #pragma omp parallel for schedule(dynamic)
        for (int Columna = 0; Columna < numero_total_de_puntos; Columna++) {
            auto [ListaAproximada, MejorDistancia] = GenerarListaAproximada(
                MatrizDeDistanciasOriginal,
                MatrizDeColumnasOrdenadas,
                MatrizDeIndicesOriginalesColumnasOrdenadas,
                Columna);

            ResultadosListas[Columna] = move(ListaAproximada);
            ResultadosDistancias[Columna] = MejorDistancia;
        }

        // Encontrar la mejor solución y escribir log
        for (int Columna = 0; Columna < numero_total_de_puntos; Columna++) {
            if (DistanciaMinima == -1 || DistanciaMinima > ResultadosDistancias[Columna]) {
                DistanciaMinima = ResultadosDistancias[Columna];
                MejorLista = ResultadosListas[Columna];
            }

            string LineaTraza = "Columna: " + to_string(Columna) + "  -> " +
                               to_string(ResultadosDistancias[Columna]) + "\n";
            archivoLog << LineaTraza;
        }
    } else {
        // Ejecución secuencial (1 hilo)
        for (int Columna = 0; Columna < numero_total_de_puntos; Columna++) {
            auto [ListaAproximada, MejorDistancia] = GenerarListaAproximada(
                MatrizDeDistanciasOriginal,
                MatrizDeColumnasOrdenadas,
                MatrizDeIndicesOriginalesColumnasOrdenadas,
                Columna);

            if (DistanciaMinima == -1 || DistanciaMinima > MejorDistancia) {
                DistanciaMinima = MejorDistancia;
                MejorLista = ListaAproximada;
            }

            string LineaTraza = "Columna: " + to_string(Columna) + "  -> " +
                               to_string(MejorDistancia) + "\n";
            archivoLog << LineaTraza;
        }
    }

    string HoraFin = "Hora Fin: " + obtener_hora_actual() + "\n";
    archivoLog << "----------------------\n";
    archivoLog << HoraFin;
    archivoLog << "----------------------\n";
    archivoLog << "Menor distancia: " << to_string(DistanciaMinima) << "\n";
    archivoLog << "----------------------\n";

    archivoLog.close();

    // Guardar solución a archivo
    ofstream archivoSalida(args.output_file);
    if (!archivoSalida.is_open()) {
        cerr << "Error al crear archivo de salida: " << args.output_file << endl;
        return 1;
    }

    for (int i = 0; i < numero_total_de_puntos; i++) {
        double x = lista_de_puntos[MejorLista[i]].PosX;
        double y = lista_de_puntos[MejorLista[i]].PosY;
        archivoSalida << fixed << setprecision(4) << x << "," << y << "\n";
    }

    archivoSalida.close();

    cout << "\n========================================" << endl;
    cout << "PROCESO COMPLETADO" << endl;
    cout << "========================================" << endl;
    cout << "Mejor distancia encontrada: " << fixed << setprecision(2) << DistanciaMinima << endl;
    cout << "Resultados guardados en: " << args.output_file << endl;
    cout << "Log guardado en: " << args.log_file << endl;
    cout << "========================================" << endl;

    return 0;
}
