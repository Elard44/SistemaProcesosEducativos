// =====================================================================
//  SIGEP-Colegio: Sistema de Gestión de Procesos Escolares
//  I.E. Emblemática Gran Unidad Escolar Inca Garcilaso de la Vega
//  Curso: Estructura de Datos - Universidad Continental
// =====================================================================

// ===================== 1.1 LIBRERÍAS =====================
#include <iostream>   // Entrada y salida por consola (cin, cout)
#include <fstream>    // Manejo de archivos para guardar y cargar datos
#include <string>     // Uso del tipo string para textos
#include <ctime>      // Obtener la fecha y hora del sistema
#include <cstdlib>    // Funciones atoi() y atol() para convertir texto a número
#include <clocale>    // setlocale() para mostrar tildes y ñ
#include <limits>     // numeric_limits para limpiar el buffer de entrada
using namespace std;  // Evita escribir std:: antes de cout, cin, string

// ===================== 1.2 ESTRUCTURAS (NODOS) =====================

// Nodo de la LISTA ENLAZADA: guarda los datos de un estudiante
struct Estudiante {
    int codigo;              // Código único del estudiante
    string nombre;           // Nombres y apellidos
    string grado;            // Grado (ej: 1ro, 2do...)
    string seccion;          // Sección (ej: A, B...)
    Estudiante* siguiente;   // Puntero al siguiente nodo de la lista

    // Constructor: inicializa los datos del nodo
    Estudiante(int c, string n, string g, string s) {
        codigo = c; nombre = n; grado = g; seccion = s;
        siguiente = NULL;    // El nuevo nodo aún no apunta a otro
    }
};

// Nodo de la PILA: guarda una acción del historial con su fecha y hora
struct Accion {
    string accion;           // Descripción de la acción realizada
    string fecha;            // Fecha en que se realizó (dd/mm/aaaa)
    string hora;             // Hora en que se realizó (hh:mm:ss)
    Accion* siguiente;       // Puntero al nodo que está debajo en la pila

    Accion(string a, string f, string h) {
        accion = a; fecha = f; hora = h;
        siguiente = NULL;
    }
};

// Nodo de la COLA DE PRIORIDAD: guarda un trámite de secretaría
struct Tramite {
    int codigo;              // Código del estudiante que hace el trámite
    string tipo;             // Tipo de trámite
    int prioridad;           // 1 = Urgente, 2 = Matrícula, 3 = Constancia, 4 = Consulta
    string estado;           // Pendiente, En atención o Atendido
    Tramite* siguiente;      // Puntero al siguiente nodo de la cola

    Tramite(int c, string t, int p) {
        codigo = c; tipo = t; prioridad = p;
        estado = "Pendiente";  // Todo trámite nuevo empieza como Pendiente
        siguiente = NULL;
    }
};

// ===================== 1.3 PUNTEROS PRINCIPALES Y CONSTANTES =====================
// Los punteros Lista_inicio, cima y Cola_inicio se declaran en main()
// y se envían a cada módulo por referencia (Estudiante*&, Accion*&, Tramite*&).
const int capacidad = 50;                // Capacidad máxima del historial (pila)
int tamanio = 0;                         // Cantidad de acciones registradas en la pila
const long periodo = 7L * 24 * 60 * 60;  // Limpieza cada 7 días (en segundos)
time_t ultimaLimpieza = 0;               // Momento de la última limpieza del historial

// Estructura estática: arreglo con los tipos de trámite (la posición define la prioridad)
string tiposTramite[4] = {"Urgente", "Matricula", "Constancia", "Consulta"};

// Prototipo: se usa en la lista antes de ser definida en la pila
void insertarAccion(Accion*& cima, string tipoAccion);

// ===================== 1.8 VALIDACIONES =====================

// Lee un número entero; si el usuario escribe letras, vuelve a pedirlo
int leerEntero(string mensaje) {
    int valor;
    cout << mensaje;
    while (!(cin >> valor)) {          // Mientras lo ingresado no sea un número
        cin.clear();                   // Limpia el estado de error
        cin.ignore(numeric_limits<streamsize>::max(), '\n');  // Descarta lo escrito
        cout << "  Dato inválido. Ingrese solo números: ";
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');  // Limpia el Enter del buffer
    return valor;
}

// Lee un texto que no puede quedar vacío ni contener el separador '|'
string leerTexto(string mensaje) {
    string texto;
    do {
        cout << mensaje;
        getline(cin, texto);           // Lee la línea completa (permite espacios)
        if (texto == "")
            cout << "  El campo no puede estar vacío." << endl;
        else if (texto.find('|') != string::npos) {
            cout << "  No se permite el carácter '|'." << endl;
            texto = "";
        }
    } while (texto == "");
    return texto;
}

// Obtiene la fecha actual en formato dd/mm/aaaa
string ObtenerFecha() {
    time_t ahora = time(0);
    char texto[11];
    strftime(texto, sizeof(texto), "%d/%m/%Y", localtime(&ahora));
    return string(texto);
}

// Obtiene la hora actual en formato hh:mm:ss
string ObtenerHora() {
    time_t ahora = time(0);
    char texto[9];
    strftime(texto, sizeof(texto), "%H:%M:%S", localtime(&ahora));
    return string(texto);
}

// Convierte un número entero a texto (para armar mensajes del historial)
string aTexto(int numero) {
    char texto[20];
    sprintf(texto, "%d", numero);
    return string(texto);
}

// ===================== 1.4 MÓDULOS DE LA LISTA (Estudiantes) =====================

// Verifica si la lista está vacía
bool listaVacia(Estudiante* Lista_inicio) {
    return Lista_inicio == NULL;               // true si no hay estudiantes
}

// INSERTAR: registra un estudiante al final de la lista, sin códigos repetidos
void insertarEstudiante(Estudiante*& Lista_inicio, Accion*& cima) {
    int codigo = leerEntero("Código del estudiante: ");
    string nombre = leerTexto("Nombres y apellidos: ");
    string grado = leerTexto("Grado: ");
    string seccion = leerTexto("Sección: ");

    bool existe = false;                       // existe <- Falso
    Estudiante* temporal = Lista_inicio;       // temporal <- Lista_inicio
    while (temporal != NULL && existe == false) {   // Recorre nodo por nodo
        if (temporal->codigo == codigo)        // ¿El código ya está registrado?
            existe = true;
        temporal = temporal->siguiente;        // Avanza al siguiente nodo
    }
    if (existe == true) {
        cout << "El código ya está registrado" << endl;
    } else {
        // Crea el nuevo nodo (su puntero siguiente queda en NULL)
        Estudiante* nuevoNodo = new Estudiante(codigo, nombre, grado, seccion);
        if (listaVacia(Lista_inicio)) {        // Si la lista está vacía
            Lista_inicio = nuevoNodo;          // El nuevo nodo es el primero
        } else {
            temporal = Lista_inicio;
            while (temporal->siguiente != NULL)    // Busca el último nodo
                temporal = temporal->siguiente;
            temporal->siguiente = nuevoNodo;   // Enlaza el último nodo con el nuevo
        }
        cout << "Estudiante registrado correctamente" << endl;
        insertarAccion(cima, "Registró estudiante " + aTexto(codigo));  // Guarda en el historial
    }
}

// ELIMINAR: da de baja a un estudiante, enlazando el nodo anterior con el siguiente
void eliminarEstudiante(Estudiante*& Lista_inicio, Accion*& cima) {
    int codigo = leerEntero("Código del estudiante a eliminar: ");
    Estudiante* actual = Lista_inicio;         // actual <- Lista_inicio
    Estudiante* anterior = NULL;               // anterior <- NULO
    while (actual != NULL && actual->codigo != codigo) {
        anterior = actual;                     // anterior queda un nodo detrás
        actual = actual->siguiente;            // actual avanza al siguiente nodo
    }
    if (actual == NULL) {
        cout << "Estudiante no encontrado" << endl;
    } else {
        if (anterior == NULL)                  // Es el primer nodo
            Lista_inicio = actual->siguiente;
        else                                   // Está en medio o al final
            anterior->siguiente = actual->siguiente;
        delete actual;                         // Libera la memoria del nodo
        cout << "Estudiante eliminado correctamente" << endl;
        insertarAccion(cima, "Eliminó estudiante " + aTexto(codigo));
    }
}

// ACTUALIZAR: modifica el grado y la sección de un estudiante
void actualizarEstudiante(Estudiante*& Lista_inicio, Accion*& cima) {
    int codigo = leerEntero("Código del estudiante a actualizar: ");
    bool encontrado = false;
    Estudiante* temporal = Lista_inicio;
    while (temporal != NULL && encontrado == false) {
        if (temporal->codigo == codigo)
            encontrado = true;                 // temporal queda en el nodo buscado
        else
            temporal = temporal->siguiente;
    }
    if (encontrado == false) {
        cout << "Estudiante no encontrado" << endl;
    } else {
        string nuevoGrado = leerTexto("Nuevo grado: ");
        string nuevaSeccion = leerTexto("Nueva sección: ");
        temporal->grado = nuevoGrado;          // Modifica los datos del nodo
        temporal->seccion = nuevaSeccion;
        cout << "Estudiante actualizado correctamente" << endl;
        insertarAccion(cima, "Actualizó estudiante " + aTexto(codigo));
    }
}

// BUSCAR: ubica a un estudiante por su código (búsqueda lineal)
void buscarEstudiante(Estudiante* Lista_inicio) {
    int codigo = leerEntero("Código del estudiante a buscar: ");
    bool encontrado = false;
    Estudiante* temporal = Lista_inicio;
    while (temporal != NULL && encontrado == false) {
        if (temporal->codigo == codigo) {
            encontrado = true;
            cout << "Código: " << temporal->codigo << " Nombre: " << temporal->nombre << endl;
            cout << "Grado: " << temporal->grado << " Sección: " << temporal->seccion << endl;
        } else {
            temporal = temporal->siguiente;
        }
    }
    if (encontrado == false)
        cout << "Estudiante no encontrado" << endl;
}

// BUSCAR POR NOMBRE: muestra los estudiantes cuyo nombre contiene el texto ingresado
void buscarPorNombre(Estudiante* Lista_inicio) {
    string nombre = leerTexto("Nombre (o parte del nombre): ");
    bool encontrado = false;
    Estudiante* temporal = Lista_inicio;
    while (temporal != NULL) {
        if (temporal->nombre.find(nombre) != string::npos) {
            cout << "Código: " << temporal->codigo << " | " << temporal->nombre
                 << " | " << temporal->grado << " " << temporal->seccion << endl;
            encontrado = true;
        }
        temporal = temporal->siguiente;
    }
    if (encontrado == false)
        cout << "Estudiante no encontrado" << endl;
}

// MOSTRAR: recorre la lista e imprime todos los estudiantes
void mostrarEstudiantes(Estudiante* Lista_inicio) {
    if (listaVacia(Lista_inicio)) {
        cout << "No hay estudiantes registrados." << endl;
        return;
    }
    Estudiante* temporal = Lista_inicio;
    cout << "\n--- LISTA DE ESTUDIANTES ---" << endl;
    while (temporal != NULL) {
        cout << temporal->codigo << " | " << temporal->nombre << " | "
             << temporal->grado << " | " << temporal->seccion << endl;
        temporal = temporal->siguiente;
    }
}

