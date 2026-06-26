/*
 * Trabajo Práctico Obligatorio - Teoría de la Computación
 * Punto 5: Algoritmo de Construcción de Subconjuntos (AFN a AFD)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//ESTRUCTURAS DEL AFN DEL PUNTO 3

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
    int visitado;
};

typedef struct {
    Estado *inicial;
    Estado *final;
} AFN;

int contador_estados_id = 0;

Estado* crearEstado(int esFinal) {
    Estado *e = (Estado *) malloc(sizeof(Estado));
    if (!e) exit(EXIT_FAILURE);
    e->id = contador_estados_id++;
    e->esFinal = esFinal;
    e->transiciones = NULL;
    e->visitado = 0;
    return e;
}

void agregarTransicion(Estado *origen, char simbolo, Estado *destino) {
    Transicion *t = (Transicion *) malloc(sizeof(Transicion));
    if (!t) exit(EXIT_FAILURE);
    t->simbolo = simbolo;
    t->destino = destino;
    t->siguiente = origen->transiciones;
    origen->transiciones = t;
}

void limpiar_visitas(Estado *e) {
    if (e == NULL || !e->visitado) return;
    e->visitado = 0;
    for (Transicion *t = e->transiciones; t != NULL; t = t->siguiente) {
        limpiar_visitas(t->destino);
    }
}

// pto5: ALGORITMO DE SUBCONJUNTOS (AFN -> AFD)

#define MAX_ESTADOS 512

// 1. Estructura para manejar conjuntos de estados del AFN (Array de banderas)
typedef struct {
    int estados[MAX_ESTADOS];
} Conjunto;

// 2. Estructuras exclusivas para el nuevo AFD determinista
typedef struct TransicionAFD TransicionAFD;

typedef struct EstadoAFD {
    int id;
    Conjunto conjunto_afn;
    int esFinal;
    TransicionAFD *transiciones;
    int marcado;
} EstadoAFD;

struct TransicionAFD {
    char simbolo;
    EstadoAFD *destino;
    TransicionAFD *siguiente;
};

typedef struct {
    EstadoAFD *inicial;
    EstadoAFD *lista_estados[MAX_ESTADOS];
    int cantidad_estados;
} AFD;


//Funciones para manejo de Conjuntos

void inicializar_conjunto(Conjunto *c) {
    for (int i = 0; i < MAX_ESTADOS; i++) c->estados[i] = 0;
}

int conjuntos_iguales(Conjunto *c1, Conjunto *c2) {
    for (int i = 0; i < MAX_ESTADOS; i++) {
        if (c1->estados[i] != c2->estados[i]) return 0;
    }
    return 1;
}

void copiar_conjunto(Conjunto *dest, Conjunto *src) {
    for (int i = 0; i < MAX_ESTADOS; i++) dest->estados[i] = src->estados[i];
}


//Mapa Global para busqueda rapida de estados AFN
Estado* mapa_afn[MAX_ESTADOS];

void mapear_estados_afn(Estado *e) {
    if (e == NULL || mapa_afn[e->id] != NULL) return;
    mapa_afn[e->id] = e; // Guardamos la referencia
    for (Transicion *t = e->transiciones; t != NULL; t = t->siguiente) {
        mapear_estados_afn(t->destino);
    }
}

// Logica Central de Subconjuntos

// Calcula la cerradura Epsilon para un solo estado recursivamente
void calcular_epsilon_clausura(Estado *e, Conjunto *resultado) {
    if (e == NULL || resultado->estados[e->id] == 1) return;

    resultado->estados[e->id] = 1; // Todo estado llega a si mismo por Epsilon

    for (Transicion *t = e->transiciones; t != NULL; t = t->siguiente) {
        if (t->simbolo == 'E') {
            calcular_epsilon_clausura(t->destino, resultado);
        }
    }
}

// Calcula la cerradura Epsilon para todo un grupo (conjunto)
void e_closure_conjunto(Conjunto *T, Conjunto *resultado) {
    inicializar_conjunto(resultado);
    for (int i = 0; i < MAX_ESTADOS; i++) {
        if (T->estados[i] == 1 && mapa_afn[i] != NULL) {
            calcular_epsilon_clausura(mapa_afn[i], resultado);
        }
    }
}

// Operacion Move: A que estados se llega directo usando una letra en particular
void calcular_move(Conjunto *T, char simbolo, Conjunto *resultado) {
    inicializar_conjunto(resultado);
    for (int i = 0; i < MAX_ESTADOS; i++) {
        if (T->estados[i] == 1 && mapa_afn[i] != NULL) {
            Estado *e = mapa_afn[i];
            for (Transicion *t = e->transiciones; t != NULL; t = t->siguiente) {
                if (t->simbolo == simbolo) {
                    resultado->estados[t->destino->id] = 1;
                }
            }
        }
    }
}

// Construccion del AFD

int id_estado_afd = 0;

EstadoAFD* crear_estado_afd(Conjunto *c) {
    EstadoAFD *nuevo = (EstadoAFD*)malloc(sizeof(EstadoAFD));
    nuevo->id = id_estado_afd++;
    copiar_conjunto(&nuevo->conjunto_afn, c);
    nuevo->esFinal = 0;
    nuevo->transiciones = NULL;
    nuevo->marcado = 0;
    return nuevo;
}

void agregar_transicion_afd(EstadoAFD *origen, char simbolo, EstadoAFD *destino) {
    TransicionAFD *t = (TransicionAFD*)malloc(sizeof(TransicionAFD));
    t->simbolo = simbolo;
    t->destino = destino;
    t->siguiente = origen->transiciones;
    origen->transiciones = t;
}

EstadoAFD* buscar_conjunto_en_afd(AFD *afd, Conjunto *c) {
    for (int i = 0; i < afd->cantidad_estados; i++) {
        if (conjuntos_iguales(&afd->lista_estados[i]->conjunto_afn, c)) {
            return afd->lista_estados[i];
        }
    }
    return NULL;
}

// FUNCION PRINCIPAL pto 5
AFD convertir_afn_a_afd(AFN afn) {
    AFD afd;
    afd.cantidad_estados = 0;
    id_estado_afd = 0;

    // 1. Registrar todos los estados del AFN en el mapa
    for(int i = 0; i < MAX_ESTADOS; i++) mapa_afn[i] = NULL;
    limpiar_visitas(afn.inicial);
    mapear_estados_afn(afn.inicial);

    // 2. Estado Inicial = e clausura del inicial del AFN
    Conjunto c_inicial;
    inicializar_conjunto(&c_inicial);
    calcular_epsilon_clausura(afn.inicial, &c_inicial);

    EstadoAFD *estado_inicial_afd = crear_estado_afd(&c_inicial);
    afd.inicial = estado_inicial_afd;
    afd.lista_estados[afd.cantidad_estados++] = estado_inicial_afd;

    // Definimos el alfabeto a buscar (simplificado a minusculas y digitos para este TP)
    char alfabeto[] = "abcdefghijklmnopqrstuvwxyz0123456789";
    int len_alfabeto = strlen(alfabeto);

    // 3. Iteracion para descubrir los subconjuntos
    int todos_marcados = 0;
    while (!todos_marcados) {
        todos_marcados = 1;
        EstadoAFD *T = NULL;

        // Busca un estado sin marcar
        for (int i = 0; i < afd.cantidad_estados; i++) {
            if (!afd.lista_estados[i]->marcado) {
                T = afd.lista_estados[i];
                T->marcado = 1;
                todos_marcados = 0;
                break;
            }
        }

        if (T == NULL) break; // Terminamos

        // Para cada simbolo del alfabeto, calcular eclausura(Move(T, a))
        for (int a = 0; a < len_alfabeto; a++) {
            char simbolo = alfabeto[a];
            Conjunto res_move;
            calcular_move(&T->conjunto_afn, simbolo, &res_move);

            // Verificamos si Move encontro algo (no es vacío)
            int vacio = 1;
            for(int i = 0; i < MAX_ESTADOS; i++) {
                if(res_move.estados[i] == 1) { vacio = 0; break; }
            }

            if (!vacio) {
                Conjunto res_closure;
                e_closure_conjunto(&res_move, &res_closure);

                //nos preguntamos si ¿este conjunto resultante ya lo tenemos en el AFD?
                EstadoAFD *U = buscar_conjunto_en_afd(&afd, &res_closure);

                if (U == NULL) {
                    U = crear_estado_afd(&res_closure);
                    afd.lista_estados[afd.cantidad_estados++] = U;
                }

                // Añadimos la transicion determinista
                agregar_transicion_afd(T, simbolo, U);
            }
        }
    }

    // 4. Marcar los estados finales del AFD
    for (int i = 0; i < afd.cantidad_estados; i++) {
        EstadoAFD *estado_afd = afd.lista_estados[i];
        for (int j = 0; j < MAX_ESTADOS; j++) {
            // Si el estado de AFD contiene un estado AFN original que era final, entonces tambien es final
            if (estado_afd->conjunto_afn.estados[j] == 1 && mapa_afn[j] != NULL) {
                if (mapa_afn[j]->esFinal == 1) {
                    estado_afd->esFinal = 1;
                    break;
                }
            }
        }
    }

    return afd;
}

//Exportacion a DOT para el nuevo AFD

void escribir_transiciones_afd_dot(EstadoAFD *e, FILE *f) {
    if (e == NULL || e->marcado == 2) return;
    e->marcado = 2; // Marcar como visitado para imprimir

    if (e->esFinal) {
        fprintf(f, "    D%d [shape=doublecircle];\n", e->id);
    } else {
        fprintf(f, "    D%d [shape=circle];\n", e->id);
    }

    for (TransicionAFD *t = e->transiciones; t != NULL; t = t->siguiente) {
        fprintf(f, "    D%d -> D%d [label=\"%c\"];\n", e->id, t->destino->id, t->simbolo);
        escribir_transiciones_afd_dot(t->destino, f);
    }
}

void exportar_afd_dot(AFD afd, const char *nombre_archivo) {
    FILE *f = fopen(nombre_archivo, "w");
    if (!f) return;

    // Reseteamos las marcas antes de imprimir
    for(int i = 0; i < afd.cantidad_estados; i++) afd.lista_estados[i]->marcado = 0;

    fprintf(f, "digraph AFD {\n");
    fprintf(f, "    rankdir=LR;\n");
    fprintf(f, "    node [style=filled, fillcolor=lightblue];\n"); // Color celeste para diferenciar del AFN

    fprintf(f, "    inicio [shape=point];\n");
    fprintf(f, "    inicio -> D%d;\n", afd.inicial->id);

    escribir_transiciones_afd_dot(afd.inicial, f);

    fprintf(f, "}\n");
    fclose(f);
    printf("[EXITO] AFD (Subconjuntos) exportado a '%s'\n", nombre_archivo);
}

int main() {
    printf("=== PRUEBA PUNTO 5: Algoritmo de Subconjuntos ===\n\n");

    // probamos
    AFN mi_afn;
    Estado* s0 = crearEstado(0);
    Estado* s1 = crearEstado(0);
    Estado* s2 = crearEstado(0);
    Estado* s3 = crearEstado(1); // Final

    agregarTransicion(s0, 'E', s1);
    agregarTransicion(s0, 'E', s2);
    agregarTransicion(s1, 'a', s3);
    agregarTransicion(s2, 'b', s3);

    mi_afn.inicial = s0;
    mi_afn.final = s3;

    printf("1. AFN de origen construido exitosamente.\n");

    AFD mi_afd = convertir_afn_a_afd(mi_afn);
    printf("2. Algoritmo AFN -> AFD ejecutado (Estados del AFD creados: %d).\n", mi_afd.cantidad_estados);

    // Exportamos el resultado
    exportar_afd_dot(mi_afd, "prueba_punto5_afd.dot");

    return 0;
}
