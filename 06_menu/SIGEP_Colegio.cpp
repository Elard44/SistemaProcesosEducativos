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

// ===================== 1.5 MÓDULOS DE LA PILA (Historial) =====================

// Verifica si la pila está vacía
bool pilaVacia(Accion* cima) {
    return cima == NULL;                       // true si no hay acciones
}

// ELIMINAR: vacía el historial si está lleno o si pasó el periodo de limpieza
void eliminarAccion(Accion*& cima) {
    time_t ahora = time(0);                                // ahora <- ObtenerFechaHora()
    double tiempoTranscurrido = difftime(ahora, ultimaLimpieza);
    if (tamanio == capacidad || tiempoTranscurrido >= periodo) {
        while (!pilaVacia(cima)) {                         // Retira todos los nodos
            Accion* temporal = cima;                       // temporal <- cima
            cima = cima->siguiente;                        // cima <- cima_siguiente
            delete temporal;                               // Liberar(temporal)
        }
        tamanio = 0;
        ultimaLimpieza = ahora;
        cout << "Historial eliminado correctamente" << endl;
    } else {
        cout << "Aún no es necesario eliminar el historial" << endl;
    }
}

// INSERTAR (push): registra una acción en la cima de la pila con fecha y hora
void insertarAccion(Accion*& cima, string tipoAccion) {
    if (tamanio == capacidad) {
        cout << "La pila está llena. Se vaciará el historial" << endl;
        eliminarAccion(cima);
    }
    Accion* nuevoNodo = new Accion(tipoAccion, ObtenerFecha(), ObtenerHora());
    nuevoNodo->siguiente = cima;       // El nuevo nodo apunta al que estaba en la cima
    cima = nuevoNodo;                  // El nuevo nodo pasa a ser la cima
    tamanio = tamanio + 1;
    cout << "Acción registrada correctamente" << endl;
}

// ACTUALIZAR: cambia la acción de un registro buscado por fecha y hora (usa pila auxiliar)
void actualizarHistorial(Accion*& cima) {
    string fechaBuscada = leerTexto("Fecha del registro (dd/mm/aaaa): ");
    string horaBuscada = leerTexto("Hora del registro (hh:mm:ss): ");
    string nuevaAccion = leerTexto("Nueva descripción de la acción: ");
    bool encontrado = false;
    Accion* pilaAuxiliar = NULL;
    Accion* temporal;
    while (cima != NULL && encontrado == false) {
        if (cima->fecha == fechaBuscada && cima->hora == horaBuscada) {
            cima->accion = nuevaAccion;            // Actualiza el nodo de la cima
            encontrado = true;
        } else {
            temporal = cima;                       // Retira el nodo de la cima
            cima = cima->siguiente;
            temporal->siguiente = pilaAuxiliar;    // Lo apila en la pila auxiliar
            pilaAuxiliar = temporal;
        }
    }
    while (pilaAuxiliar != NULL) {                 // Devuelve los nodos a la pila original
        temporal = pilaAuxiliar;
        pilaAuxiliar = pilaAuxiliar->siguiente;
        temporal->siguiente = cima;
        cima = temporal;
    }
    if (encontrado == true)
        cout << "Historial actualizado correctamente" << endl;
    else
        cout << "No se encontró el registro a actualizar" << endl;
}

// BUSCAR: busca en el historial por fecha o por tipo de acción (usa pila auxiliar)
void buscarHistorial(Accion*& cima) {
    cout << "1. Buscar por fecha" << endl;
    cout << "2. Buscar por acción" << endl;
    int opcion = leerEntero("Elija una opción: ");
    if (opcion == 1 || opcion == 2) {
        string fechaBuscada = "", tipoAccion = "";
        if (opcion == 1)
            fechaBuscada = leerTexto("Fecha (dd/mm/aaaa): ");
        else
            tipoAccion = leerTexto("Acción (texto a buscar): ");
        bool encontrado = false;
        Accion* pilaAuxiliar = NULL;
        Accion* temporal;
        while (cima != NULL) {
            bool coincide = false;
            if (opcion == 1 && cima->fecha == fechaBuscada)
                coincide = true;
            if (opcion == 2 && cima->accion.find(tipoAccion) != string::npos)
                coincide = true;
            if (coincide == true) {
                if (opcion == 1)
                    cout << "Acción: " << cima->accion << " - Hora: " << cima->hora << endl;
                else
                    cout << "Fecha: " << cima->fecha << " - Hora: " << cima->hora << endl;
                encontrado = true;
            }
            temporal = cima;                       // Pasa el nodo a la pila auxiliar
            cima = cima->siguiente;
            temporal->siguiente = pilaAuxiliar;
            pilaAuxiliar = temporal;
        }
        while (pilaAuxiliar != NULL) {             // Restaura la pila original
            temporal = pilaAuxiliar;
            pilaAuxiliar = pilaAuxiliar->siguiente;
            temporal->siguiente = cima;
            cima = temporal;
        }
        if (encontrado == false) {
            if (opcion == 1) cout << "No hay modificaciones en esa fecha" << endl;
            else cout << "No se encontró ese tipo de acción" << endl;
        }
    } else {
        cout << "Opción inválida" << endl;
    }
}

// MOSTRAR: imprime el historial desde la cima hasta la base
void mostrarHistorial(Accion* cima) {
    if (pilaVacia(cima)) {
        cout << "El historial está vacío." << endl;
        return;
    }
    Accion* temporal = cima;
    cout << "\n--- HISTORIAL (" << tamanio << "/" << capacidad << ") ---" << endl;
    while (temporal != NULL) {
        cout << temporal->fecha << " " << temporal->hora << " | " << temporal->accion << endl;
        temporal = temporal->siguiente;
    }
}

// ===================== 1.6 MÓDULOS DE LA COLA (Trámites) =====================

// Verifica si la cola está vacía
bool colaVacia(Tramite* Cola_inicio) {
    return Cola_inicio == NULL;                // true si no hay trámites
}

// INSERTAR (encolar): ubica el trámite según su prioridad
void encolarTramite(Tramite*& Cola_inicio, int codigo, string tipo, int prioridad) {
    // Crea el nodo con estado Pendiente y siguiente en NULL
    Tramite* nuevoNodo = new Tramite(codigo, tipo, prioridad);
    if (colaVacia(Cola_inicio) || prioridad < Cola_inicio->prioridad) {
        nuevoNodo->siguiente = Cola_inicio;    // Se coloca al inicio de la cola
        Cola_inicio = nuevoNodo;
    } else {
        Tramite* temporal = Cola_inicio;
        while (temporal->siguiente != NULL && temporal->siguiente->prioridad <= prioridad)
            temporal = temporal->siguiente;    // Avanza detrás de los más urgentes e iguales
        nuevoNodo->siguiente = temporal->siguiente;
        temporal->siguiente = nuevoNodo;
    }
    cout << "Trámite registrado correctamente" << endl;
}

// AGREGAR PROCESO: lee y valida los datos del trámite antes de encolarlo
void agregarProceso(Tramite*& Cola_inicio) {
    int codigo = leerEntero("Código del estudiante: ");
    cout << "Tipos de trámite:" << endl;
    for (int i = 0; i < 4; i++)                // Recorre el arreglo estático
        cout << "  " << (i + 1) << ". " << tiposTramite[i] << endl;
    int prioridad = leerEntero("Elija el tipo (1-4): ");
    if (prioridad < 1 || prioridad > 4) {
        cout << "Prioridad no válida" << endl;
    } else {
        string tipo = tiposTramite[prioridad - 1];  // El tipo según la posición
        encolarTramite(Cola_inicio, codigo, tipo, prioridad);
    }
}

// ELIMINAR (atender): atiende el trámite del inicio de la cola
void atenderTramite(Tramite*& Cola_inicio) {
    if (colaVacia(Cola_inicio)) {
        cout << "No hay trámites pendientes" << endl;
    } else {
        cout << "Atendiendo: " << Cola_inicio->codigo << " - " << Cola_inicio->tipo << endl;
        Tramite* temporal = Cola_inicio;       // Guarda el nodo atendido
        Cola_inicio = Cola_inicio->siguiente;  // Cola_inicio avanza al siguiente nodo
        delete temporal;                       // Libera la memoria del nodo atendido
        cout << "Trámite atendido correctamente" << endl;
    }
}

// ACTUALIZAR: cambia la prioridad de un trámite sacándolo de la cola y volviéndolo a encolar
void actualizarTramite(Tramite*& Cola_inicio) {
    int codigo = leerEntero("Código del estudiante: ");
    for (int i = 0; i < 4; i++)                // Muestra los tipos del arreglo estático
        cout << "  " << (i + 1) << ". " << tiposTramite[i] << endl;
    int nuevaPrioridad = leerEntero("Nueva prioridad (1-4): ");
    while (nuevaPrioridad < 1 || nuevaPrioridad > 4)   // Valida el rango
        nuevaPrioridad = leerEntero("Prioridad no válida. Ingrese de 1 a 4: ");
    Tramite* actual = Cola_inicio;             // actual <- Cola_inicio
    Tramite* anterior = NULL;                  // anterior <- NULO
    while (actual != NULL && actual->codigo != codigo) {
        anterior = actual;
        actual = actual->siguiente;
    }
    if (actual == NULL) {
        cout << "Trámite no encontrado" << endl;
    } else {
        if (anterior == NULL)                  // Desenlaza el nodo de la cola
            Cola_inicio = actual->siguiente;
        else
            anterior->siguiente = actual->siguiente;
        // Lo vuelve a encolar para ubicarlo según su nueva prioridad
        encolarTramite(Cola_inicio, codigo, tiposTramite[nuevaPrioridad - 1], nuevaPrioridad);
        delete actual;                         // Liberar(actual)
        cout << "Prioridad actualizada correctamente" << endl;
    }
}

// BUSCAR: muestra los datos del trámite de un estudiante
void buscarTramite(Tramite* Cola_inicio) {
    int codigo = leerEntero("Código del estudiante: ");
    bool encontrado = false;
    Tramite* temporal = Cola_inicio;
    while (temporal != NULL && encontrado == false) {
        if (temporal->codigo == codigo) {
            encontrado = true;
            cout << "Código: " << temporal->codigo << " Tipo: " << temporal->tipo << endl;
            cout << "Prioridad: " << temporal->prioridad
                 << " Estado: " << temporal->estado << endl;
        } else {
            temporal = temporal->siguiente;
        }
    }
    if (encontrado == false)
        cout << "Trámite no encontrado" << endl;
}

// CAMBIAR ESTADO: Pendiente -> En atención -> Atendido
void cambiarEstadoProceso(Tramite* Cola_inicio) {
    int codigo = leerEntero("Código del estudiante: ");
    bool encontrado = false;
    Tramite* temporal = Cola_inicio;
    while (temporal != NULL && encontrado == false) {
        if (temporal->codigo == codigo) {
            encontrado = true;
            if (temporal->estado == "Pendiente")
                temporal->estado = "En atención";
            else
                temporal->estado = "Atendido";
            cout << "Nuevo estado: " << temporal->estado << endl;
        } else {
            temporal = temporal->siguiente;
        }
    }
    if (encontrado == false)
        cout << "Trámite no encontrado" << endl;
}

// MOSTRAR: imprime la cola en orden de atención
void mostrarCola(Tramite* Cola_inicio) {
    if (colaVacia(Cola_inicio)) {
        cout << "No hay trámites en espera." << endl;
        return;
    }
    Tramite* temporal = Cola_inicio;
    int turno = 1;
    cout << "\n--- COLA DE ATENCIÓN ---" << endl;
    while (temporal != NULL) {
        cout << "Turno " << turno << ": " << temporal->codigo << " | " << temporal->tipo
             << " | Prioridad " << temporal->prioridad << " | " << temporal->estado << endl;
        temporal = temporal->siguiente;
        turno++;
    }
}

// ===================== 1.7 PERSISTENCIA (GUARDAR Y CARGAR) =====================

// Escribe la pila desde la base hasta la cima (recursivo) para conservar su orden
void guardarPila(Accion* nodo, ofstream& archivo) {
    if (nodo == NULL) return;
    guardarPila(nodo->siguiente, archivo);         // Primero los nodos de abajo
    archivo << nodo->accion << "|" << nodo->fecha << "|" << nodo->hora << endl;
}

void guardarDatos(Estudiante* Lista_inicio, Accion* cima, Tramite* Cola_inicio) {
    ofstream fe("estudiantes.txt");                // Archivo de estudiantes
    for (Estudiante* t = Lista_inicio; t != NULL; t = t->siguiente)
        fe << t->codigo << "|" << t->nombre << "|" << t->grado << "|" << t->seccion << endl;
    fe.close();

    ofstream fh("historial.txt");                  // Archivo del historial
    fh << ultimaLimpieza << endl;
    guardarPila(cima, fh);
    fh.close();

    ofstream ft("tramites.txt");                   // Archivo de trámites
    for (Tramite* t = Cola_inicio; t != NULL; t = t->siguiente)
        ft << t->codigo << "|" << t->tipo << "|" << t->prioridad << "|" << t->estado << endl;
    ft.close();
    cout << "Datos guardados correctamente." << endl;
}

// Separa una línea de texto en campos usando el carácter '|'
int separar(string linea, string campos[], int maximo) {
    int n = 0;
    size_t pos;
    while (n < maximo - 1 && (pos = linea.find('|')) != string::npos) {
        campos[n++] = linea.substr(0, pos);
        linea = linea.substr(pos + 1);
    }
    campos[n++] = linea;
    return n;
}

void cargarDatos(Estudiante*& Lista_inicio, Accion*& cima, Tramite*& Cola_inicio) {
    string linea, c[4];
    ifstream fe("estudiantes.txt");
    Estudiante* ultimo = NULL;
    while (getline(fe, linea)) {
        if (separar(linea, c, 4) < 4) continue;
        Estudiante* nuevoNodo = new Estudiante(atoi(c[0].c_str()), c[1], c[2], c[3]);
        if (Lista_inicio == NULL) Lista_inicio = nuevoNodo;   // Se enlaza al final
        else ultimo->siguiente = nuevoNodo;
        ultimo = nuevoNodo;
    }
    fe.close();

    ifstream fh("historial.txt");
    if (getline(fh, linea)) ultimaLimpieza = (time_t)atol(linea.c_str());
    while (getline(fh, linea)) {
        if (separar(linea, c, 3) < 3) continue;
        Accion* nuevoNodo = new Accion(c[0], c[1], c[2]);   // Se apila en el mismo orden
        nuevoNodo->siguiente = cima;
        cima = nuevoNodo;
        tamanio++;
    }
    fh.close();
    if (ultimaLimpieza == 0) ultimaLimpieza = time(0);

    ifstream ft("tramites.txt");
    Tramite* final = NULL;
    while (getline(ft, linea)) {
        if (separar(linea, c, 4) < 4) continue;
        Tramite* nuevoNodo = new Tramite(atoi(c[0].c_str()), c[1], atoi(c[2].c_str()));
        nuevoNodo->estado = c[3];
        if (Cola_inicio == NULL) Cola_inicio = nuevoNodo;    // Ya vienen ordenados
        else final->siguiente = nuevoNodo;
        final = nuevoNodo;
    }
    ft.close();
}

// ===================== 1.9 MENÚ PRINCIPAL (main) =====================

void menuEstudiantes(Estudiante*& Lista_inicio, Accion*& cima) {
    int op;
    do {
        cout << "\n===== GESTIÓN DE ESTUDIANTES =====" << endl;
        cout << "1. Registrar estudiante\n2. Buscar por código\n"
             << "3. Buscar por nombre\n4. Actualizar grado y sección\n"
             << "5. Eliminar estudiante\n6. Mostrar estudiantes\n0. Volver" << endl;
        op = leerEntero("Opción: ");
        switch (op) {
            case 1: insertarEstudiante(Lista_inicio, cima); break;
            case 2: buscarEstudiante(Lista_inicio); break;
            case 3: buscarPorNombre(Lista_inicio); break;
            case 4: actualizarEstudiante(Lista_inicio, cima); break;
            case 5: eliminarEstudiante(Lista_inicio, cima); break;
            case 6: mostrarEstudiantes(Lista_inicio); break;
            case 0: break;
            default: cout << "Opción no válida." << endl;
        }
    } while (op != 0);
}

void menuTramites(Tramite*& Cola_inicio) {
    int op;
    do {
        cout << "\n===== ATENCIÓN EN SECRETARÍA =====" << endl;
        cout << "1. Registrar trámite\n2. Atender siguiente trámite\n"
             << "3. Buscar trámite\n4. Actualizar prioridad del trámite\n"
             << "5. Cambiar estado del trámite\n6. Mostrar cola\n0. Volver" << endl;
        op = leerEntero("Opción: ");
        switch (op) {
            case 1: agregarProceso(Cola_inicio); break;
            case 2: atenderTramite(Cola_inicio); break;
            case 3: buscarTramite(Cola_inicio); break;
            case 4: actualizarTramite(Cola_inicio); break;
            case 5: cambiarEstadoProceso(Cola_inicio); break;
            case 6: mostrarCola(Cola_inicio); break;
            case 0: break;
            default: cout << "Opción no válida." << endl;
        }
    } while (op != 0);
}

void menuHistorial(Accion*& cima) {
    int op;
    do {
        cout << "\n===== HISTORIAL DE ACCIONES =====" << endl;
        cout << "1. Mostrar historial\n2. Buscar en el historial\n"
             << "3. Actualizar un registro\n"
             << "4. Eliminar historial\n0. Volver" << endl;
        op = leerEntero("Opción: ");
        switch (op) {
            case 1: mostrarHistorial(cima); break;
            case 2: buscarHistorial(cima); break;
            case 3: actualizarHistorial(cima); break;
            case 4: eliminarAccion(cima); break;
            case 0: break;
            default: cout << "Opción no válida." << endl;
        }
    } while (op != 0);
}

int main() {
    setlocale(LC_CTYPE, "Spanish");    // Permite mostrar tildes y ñ
    Estudiante* Lista_inicio = NULL;   // Apunta al primer nodo de la lista de estudiantes
    Accion* cima = NULL;               // Apunta a la cima de la pila (última acción)
    Tramite* Cola_inicio = NULL;       // Apunta al primer nodo de la cola de trámites
    cargarDatos(Lista_inicio, cima, Cola_inicio);   // Recupera la información guardada
    int opcion;
    do {
        cout << "\n========== SIGEP-COLEGIO ==========" << endl;
        cout << "I.E. Inca Garcilaso de la Vega" << endl;
        cout << "1. Gestión de estudiantes" << endl;
        cout << "2. Atención en secretaría" << endl;
        cout << "3. Historial de acciones" << endl;
        cout << "4. Guardar datos" << endl;
        cout << "0. Salir" << endl;
        opcion = leerEntero("Seleccione una opción: ");
        switch (opcion) {
            case 1: menuEstudiantes(Lista_inicio, cima); break;
            case 2: menuTramites(Cola_inicio); break;
            case 3: menuHistorial(cima); break;
            case 4: guardarDatos(Lista_inicio, cima, Cola_inicio); break;
            case 0:
                guardarDatos(Lista_inicio, cima, Cola_inicio);
                cout << "Hasta pronto." << endl;
                break;
            default: cout << "Opción no válida." << endl;
        }
    } while (opcion != 0);
    return 0;
}
