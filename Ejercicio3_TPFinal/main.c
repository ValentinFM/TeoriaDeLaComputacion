#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    int visitado; // Auxiliar para evita bucles infinitos
};

// Estructura contenedora para manipular los AFN como bloques individuales
typedef struct {
    Estado *inicial;
    Estado *final;
} AFN;

int contador_estados_id = 0;// Id unico

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
// parte a
void escribir_transiciones_dot(Estado *e, FILE *f) {
    if (e == NULL || e->visitado) return;
    e->visitado = 1;

    // Configurar aspecto si es un estado final
    if (e->esFinal) {
        fprintf(f, "    N%d [shape=doublecircle];\n", e->id);
    } else {
        fprintf(f, "    N%d [shape=circle];\n", e->id);
    }

    // Recorrer e imprimir transiciones salientes
    for (Transicion *t = e->transiciones; t != NULL; t = t->siguiente) {
        fprintf(f, "    N%d -> N%d [label=\"%c\"];\n", e->id, t->destino->id, t->simbolo);
        escribir_transiciones_dot(t->destino, f);
    }
}

// Resetea la bandera de visitado de forma recursiva
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
    fprintf(f, "    rankdir=LR;\n"); // Gráfico de izquierda a derecha
    fprintf(f, "    node [style=filled, fillcolor=white];\n");

    // Un puntero invisible para señalar el estado inicial
    fprintf(f, "    inicio [shape=point];\n");
    fprintf(f, "    inicio -> N%d;\n", afn.inicial->id);

    // Escribir la estructura del grafo
    escribir_transiciones_dot(afn.inicial, f);

    fprintf(f, "}\n");
    fclose(f);

    // Dejar limpio el arbol
    limpiar_visitas(afn.inicial);
    printf("AFN exportado correctamente a '%s'\n", nombre_archivo);
}
//parte b
AFN union_afn(AFN A, AFN B) {
    AFN resultado;

    // Crear nuevos estados extremos
    resultado.inicial = crearEstado(0);
    resultado.final = crearEstado(1);

    // Desmarcar la condición de final de los antiguos autómatas
    A.final->esFinal = 0;
    B.final->esFinal = 0;

    // Transiciones 'E' (lambda) desde el nuevo inicio
    agregarTransicion(resultado.inicial, 'E', A.inicial);
    agregarTransicion(resultado.inicial, 'E', B.inicial);

    // Transiciones 'E' (lambda) hacia el nuevo final
    agregarTransicion(A.final, 'E', resultado.final);
    agregarTransicion(B.final, 'E', resultado.final);

    return resultado;
}
//parte c
AFN concatenacion_afn(AFN A, AFN B) {
    AFN resultado;

    // El inicio de todo es el inicio de A
    resultado.inicial = A.inicial;

    // El final de A ya no es un estado final absoluto
    A.final->esFinal = 0;

    // Conexión puente por epsilon entre los bloques
    agregarTransicion(A.final, 'E', B.inicial);

    // El final de todo es el final de B
    resultado.final = B.final;
    resultado.final->esFinal = 1;

    return resultado;
}
//parte d
AFN clausura_kleene_afn(AFN A) {
    AFN resultado;

    // Crear nuevos extremos de control
    resultado.inicial = crearEstado(0);
    resultado.final = crearEstado(1);

    A.final->esFinal = 0;

    // Camino 1: Entrar al automata por primera vez
    agregarTransicion(resultado.inicial, 'E', A.inicial);

    // Camino 2: Saltear el automata por completo
    agregarTransicion(resultado.inicial, 'E', resultado.final);

    // Camino 3: Bucle de retorno (permite volver a procesar A tras terminar)
    agregarTransicion(A.final, 'E', A.inicial);

    // Camino 4: Salida del automata hacia el nuevo final
    agregarTransicion(A.final, 'E', resultado.final);

    return resultado;
}

//main
int main() {
    printf("=== Generando Ejemplos de Algoritmos de Thompson ===\n\n");

    // ==========================================
    // EJEMPLO 1: CONCATENACION (a . b)
    // ==========================================
    printf("1. Procesando Concatenacion (a . b)...\n");
    contador_estados_id = 0; // Reiniciamos IDs para este ejemplo

    AFN afn_a1 = { crearEstado(0), crearEstado(1) };
    agregarTransicion(afn_a1.inicial, 'a', afn_a1.final);

    AFN afn_b1 = { crearEstado(0), crearEstado(1) };
    agregarTransicion(afn_b1.inicial, 'b', afn_b1.final);

    AFN resultado_concat = concatenacion_afn(afn_a1, afn_b1);
    exportar_afn_dot(resultado_concat, "afn_1_concatenacion.dot");
    printf("-----------------------------------------\n");


    // ==========================================
    // EJEMPLO 2: UNION (a | b)
    // ==========================================
    printf("2. Procesando Union (a | b)...\n");
    contador_estados_id = 0; // Reiniciamos IDs para este ejemplo

    AFN afn_a2 = { crearEstado(0), crearEstado(1) };
    agregarTransicion(afn_a2.inicial, 'a', afn_a2.final);

    AFN afn_b2 = { crearEstado(0), crearEstado(1) };
    agregarTransicion(afn_b2.inicial, 'b', afn_b2.final);

    AFN resultado_union = union_afn(afn_a2, afn_b2);
    exportar_afn_dot(resultado_union, "afn_2_union.dot");
    printf("-----------------------------------------\n");


    // ==========================================
    // EJEMPLO 3: CLAUSURA DE KLEENE (a*)
    // ==========================================
    printf("3. Procesando Clausura de Kleene (a*)...\n");
    contador_estados_id = 0; // Reiniciamos IDs para este ejemplo

    AFN afn_a3 = { crearEstado(0), crearEstado(1) };
    agregarTransicion(afn_a3.inicial, 'a', afn_a3.final);

    AFN resultado_clausura = clausura_kleene_afn(afn_a3);
    exportar_afn_dot(resultado_clausura, "afn_3_clausura.dot");
    printf("-----------------------------------------\n");

    printf("\n[EXITO] Se generaron los 3 archivos .dot en tu carpeta.\n");
    return 0;
}
