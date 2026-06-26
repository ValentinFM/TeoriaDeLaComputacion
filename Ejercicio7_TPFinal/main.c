#include <stdio.h>
#include <string.h>
#include <stdbool.h>

//Estructuras

typedef struct {
    int ids_origen[50];
    int tam;
} RegistroContenido;

typedef struct {
    int indice;
    int es_aceptacion;
    RegistroContenido info;
} NodoAFD;

typedef struct {
    int origen;
    char letra;
    int destino;
} FlechaTransicion;

typedef struct {
    NodoAFD nodos[50];
    int total_nodos;
    FlechaTransicion rutas[200];
    int total_rutas;
} AutaAFD;

//recorrido
bool validar_cadena(AutaAFD aut, const char *input) {
    int cursor = aut.nodos[0].indice;
    int largo = strlen(input);

    for (int k = 0; k < largo; k++) {
        char simbolo = input[k];
        int siguiente = -1;

        // Buscamos la transicion validando el origen y el simbolo simultaneamente
        for (int m = 0; m < aut.total_rutas; m++) {
            if (aut.rutas[m].origen == cursor && aut.rutas[m].letra == simbolo) {
                siguiente = aut.rutas[m].destino;
                break;
            }
        }

        // Si 'siguiente' sigue siendo -1, no hay camino posible
        if (siguiente == -1) return false;

        cursor = siguiente;
    }

    // Verificación final contra los estados de aceptación
    for (int n = 0; n < aut.total_nodos; n++) {
        if (aut.nodos[n].indice == cursor) {
            return aut.nodos[n].es_aceptacion == 1;
        }
    }

    return false;
}

int main() {
    printf("=== EVALUADOR DE LENGUAJE (PUNTO 7) ===\n");
    printf("Estructura de prueba: (a.b*.c)\n\n");

    AutaAFD mi_maquina;
    mi_maquina.total_nodos = 3;
    mi_maquina.total_rutas = 3;

    // Configuracion de nodos
    mi_maquina.nodos[0].indice = 0; mi_maquina.nodos[0].es_aceptacion = 0;
    mi_maquina.nodos[1].indice = 1; mi_maquina.nodos[1].es_aceptacion = 0;
    mi_maquina.nodos[2].indice = 2; mi_maquina.nodos[2].es_aceptacion = 1;

    // Configuracion de rutas
    mi_maquina.rutas[0] = (FlechaTransicion){0, 'a', 1};
    mi_maquina.rutas[1] = (FlechaTransicion){1, 'b', 1};
    mi_maquina.rutas[2] = (FlechaTransicion){1, 'c', 2};

    char buffer[128];
    while (true) {
        printf("Cadena a testear: ");
        scanf("%s", buffer);

        if (strcmp(buffer, "salir") == 0) break;

        if (validar_cadena(mi_maquina, buffer)) {
            printf(">> ESTATUS: [ACEPTADA]\n\n");
        } else {
            printf(">> ESTATUS: [RECHAZADA]\n\n");
        }
    }

    return 0;
}
