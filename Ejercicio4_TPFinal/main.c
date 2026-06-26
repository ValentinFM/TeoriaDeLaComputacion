#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int id_global_graphviz = 0;

typedef enum {
    NODO_OPERANDO,
    NODO_UNION,
    NODO_CONCAT,
    NODO_KLEENE
} TipoNodo;

typedef struct NodoArbol {
    int id;
    TipoNodo tipo;
    char caracter;
    struct NodoArbol *rama_izq;
    struct NodoArbol *rama_der;
} NodoArbol;

NodoArbol* asignarMemoriaNodo() {
    NodoArbol* n = (NodoArbol*)malloc(sizeof(NodoArbol));
    if (!n) { exit(EXIT_FAILURE); }
    n->id = id_global_graphviz++;
    n->rama_izq = NULL;
    n->rama_der = NULL;
    return n;
}

NodoArbol* fabricarHoja(char simbolo) {
    NodoArbol* nodo = asignarMemoriaNodo();
    nodo->tipo = NODO_OPERANDO;
    nodo->caracter = simbolo;
    return nodo;
}

NodoArbol* fabricarOperadorBinario(char operador, NodoArbol* izq, NodoArbol* der) {
    NodoArbol* nodo = asignarMemoriaNodo();
    nodo->tipo = (operador == '|') ? NODO_UNION : NODO_CONCAT;
    nodo->caracter = operador;
    nodo->rama_izq = izq;
    nodo->rama_der = der;
    return nodo;
}

NodoArbol* fabricarClausura(NodoArbol* base) {
    NodoArbol* nodo = asignarMemoriaNodo();
    nodo->tipo = NODO_KLEENE;
    nodo->caracter = '*';
    nodo->rama_izq = base;
    return nodo;
}

void destruirArbol(NodoArbol* raiz) {
    if (raiz == NULL) return;
    destruirArbol(raiz->rama_izq);
    destruirArbol(raiz->rama_der);
    free(raiz);
}
// CODIGO REUTILIZADO EL ANTERIOR
//pto3
typedef struct Estado Estado;
typedef struct Transicion Transicion;

struct Transicion {
    char simbolo; // 'a', 'b' o 'E' para lambda
    Estado *destino;
    Transicion *siguiente;
};

struct Estado {
    int id;
    int esFinal;
    Transicion *transiciones;
    int visitado; // Auxiliar para recorridos de grafos (evita bucles infinitos)
};

// Estructura contenedora para manipular los AFN como bloques individuales
typedef struct {
    Estado *inicial;
    Estado *final;
} AFN;

int contador_estados_id = 0;

Estado* crearEstado(int esFinal) {
    Estado *e = (Estado *) malloc(sizeof(Estado));
    if (!e) { exit(EXIT_FAILURE); }
    e->id = contador_estados_id++;
    e->esFinal = esFinal;
    e->transiciones = NULL;
    e->visitado = 0;
    return e;
}

void agregarTransicion(Estado *origen, char simbolo, Estado *destino) {
    Transicion *t = (Transicion *) malloc(sizeof(Transicion));
    if (!t) { exit(EXIT_FAILURE); }
    t->simbolo = simbolo;
    t->destino = destino;
    t->siguiente = origen->transiciones;
    origen->transiciones = t;
}

void escribir_transiciones_dot(Estado *e, FILE *f) {
    if (e == NULL || e->visitado) return;
    e->visitado = 1;

    if (e->esFinal) {
        fprintf(f, "    N%d [shape=doublecircle];\n", e->id);
    } else {
        fprintf(f, "    N%d [shape=circle];\n", e->id);
    }

    for (Transicion *t = e->transiciones; t != NULL; t = t->siguiente) {
        fprintf(f, "    N%d -> N%d [label=\"%c\"];\n", e->id, t->destino->id, t->simbolo);
        escribir_transiciones_dot(t->destino, f);
    }
}

void limpiar_visitas(Estado *e) {
    if (e == NULL || !e->visitado) return;
    e->visitado = 0;
    for (Transicion *t = e->transiciones; t != NULL; t = t->siguiente) {
        limpiar_visitas(t->destino);
    }
}

void exportar_afn_dot(AFN afn, const char *nombre_archivo) {
    FILE *f = fopen(nombre_archivo, "w");
    if (!f) {
        perror("Error al abrir archivo DOT");
        return;
    }

    fprintf(f, "digraph AFN {\n");
    fprintf(f, "    rankdir=LR;\n");
    fprintf(f, "    node [style=filled, fillcolor=white];\n");

    fprintf(f, "    inicio [shape=point];\n");
    fprintf(f, "    inicio -> N%d;\n", afn.inicial->id);

    escribir_transiciones_dot(afn.inicial, f);

    fprintf(f, "}\n");
    fclose(f);

    limpiar_visitas(afn.inicial);
    printf("-> AFN exportado correctamente a '%s'\n", nombre_archivo);
}

AFN union_afn(AFN A, AFN B) {
    AFN resultado;
    resultado.inicial = crearEstado(0);
    resultado.final = crearEstado(1);
    A.final->esFinal = 0;
    B.final->esFinal = 0;
    agregarTransicion(resultado.inicial, 'E', A.inicial);
    agregarTransicion(resultado.inicial, 'E', B.inicial);
    agregarTransicion(A.final, 'E', resultado.final);
    agregarTransicion(B.final, 'E', resultado.final);
    return resultado;
}

AFN concatenacion_afn(AFN A, AFN B) {
    AFN resultado;
    resultado.inicial = A.inicial;
    A.final->esFinal = 0;
    agregarTransicion(A.final, 'E', B.inicial);
    resultado.final = B.final;
    resultado.final->esFinal = 1;
    return resultado;
}

AFN clausura_kleene_afn(AFN A) {
    AFN resultado;
    resultado.inicial = crearEstado(0);
    resultado.final = crearEstado(1);
    A.final->esFinal = 0;
    agregarTransicion(resultado.inicial, 'E', A.inicial);
    agregarTransicion(resultado.inicial, 'E', resultado.final);
    agregarTransicion(A.final, 'E', A.inicial);
    agregarTransicion(A.final, 'E', resultado.final);
    return resultado;
}
//pto 4
// Helper para convertir una letra pura (hoja del arbol) en un mini automata
AFN crear_afn_hoja(char simbolo) {
    AFN base;
    base.inicial = crearEstado(0); // 0 indica que NO es final
    base.final = crearEstado(1);   // 1 indica que SÍ es final

    // Conectamos el inicio con el fin usando el caracter (ej: 'a', 'b', 'c')
    agregarTransicion(base.inicial, simbolo, base.final);

    return base;
}

// Función Principal
AFN ast_a_afn(NodoArbol* raiz) {
    // Si el nodo es nulo, retornamos un AFN vacio por seguridad
    if (raiz == NULL) {
        AFN vacio = {NULL, NULL};
        return vacio;
    }

    // 1. Si es una hoja, creamos el automata basico
    if (raiz->tipo == NODO_OPERANDO) {
        return crear_afn_hoja(raiz->caracter);
    }

    // 2. Si es una Union (|), resolvemos sus hijos y los unimos
    if (raiz->tipo == NODO_UNION) {
        AFN afn_izq = ast_a_afn(raiz->rama_izq);
        AFN afn_der = ast_a_afn(raiz->rama_der);
        return union_afn(afn_izq, afn_der);
    }

    // 3. Si es una Concatenacion (.), resolvemos sus hijos y los concatenamos
    if (raiz->tipo == NODO_CONCAT) {
        AFN afn_izq = ast_a_afn(raiz->rama_izq);
        AFN afn_der = ast_a_afn(raiz->rama_der);
        return concatenacion_afn(afn_izq, afn_der);
    }

    // 4. Si es Kleene (*), resolvemos su unico hijo (el izquierdo) y le aplicamos la clausura
    if (raiz->tipo == NODO_KLEENE) {
        AFN afn_base = ast_a_afn(raiz->rama_izq);
        return clausura_kleene_afn(afn_base);
    }

    AFN error = {NULL, NULL};
    return error;
}


int main() {
    printf("=== PUNTO 4: Transformacion de AST a AFN (Thompson) ===\n\n");

    // Reiniciamos contadores
    id_global_graphviz = 0;
    contador_estados_id = 0;

    // PASO 1: Construimos el arbol para la expresión (a.b)|c*

    printf("[*] Construyendo Arbol AST en memoria para: (a.b)|c*\n");

    // Sub arbol Izquierdo: a.b
    NodoArbol* nodo_a = fabricarHoja('a');
    NodoArbol* nodo_b = fabricarHoja('b');
    NodoArbol* concat_ab = fabricarOperadorBinario('.', nodo_a, nodo_b);

    // Sub arbol Derecho: c*
    NodoArbol* nodo_c = fabricarHoja('c');
    NodoArbol* kleene_c = fabricarClausura(nodo_c);

    // Raiz: |
    NodoArbol* arbol_raiz = fabricarOperadorBinario('|', concat_ab, kleene_c);

    // PASO 2: Transformamos el AST en un AFN usando el Punto 4

    printf("[*] Ejecutando recursividad de Thompson...\n");
    AFN automata_resultante = ast_a_afn(arbol_raiz);

    // PASO 3: Exportamos el AFN a Graphviz

    exportar_afn_dot(automata_resultante, "resultado_punto4.dot");

    // Liberamos la memoria del arbol
    destruirArbol(arbol_raiz);

    printf("\n[EXITO] Finalizado. Revisa el archivo 'resultado_punto4.dot'.\n");
    return 0;
}
