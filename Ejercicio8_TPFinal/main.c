#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// ESTRUCTURAS

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

// EJERCICIO 7
// VERIFICA SI UNA CADENA ES ACEPTADA POR EL AFD

bool validar_cadena(AutaAFD aut, const char *input)
{
    int estado_actual = aut.nodos[0].indice;
    int largo = strlen(input);

    for (int i = 0; i < largo; i++)
    {
        char simbolo = input[i];
        int siguiente = -1;

        for (int j = 0; j < aut.total_rutas; j++)
        {
            if (aut.rutas[j].origen == estado_actual &&
                aut.rutas[j].letra == simbolo)
            {
                siguiente = aut.rutas[j].destino;
                break;
            }
        }

        if (siguiente == -1)
            return false;

        estado_actual = siguiente;
    }

    for (int i = 0; i < aut.total_nodos; i++)
    {
        if (aut.nodos[i].indice == estado_actual)
        {
            return aut.nodos[i].es_aceptacion == 1;
        }
    }

    return false;
}

// EJERCICIO 8 - MINIGREP
// Busca si alguna subcadena del texto es aceptada por el AFD


bool minigrep(AutaAFD aut, const char *texto)
{
    int largo = strlen(texto);

    char subcadena[128];

    for (int inicio = 0; inicio < largo; inicio++)
    {
        for (int fin = inicio; fin < largo; fin++)
        {
            int k = 0;

            for (int i = inicio; i <= fin; i++)
            {
                subcadena[k++] = texto[i];
            }

            subcadena[k] = '\0';

            printf("Probando: %s", subcadena);

            if (validar_cadena(aut, subcadena))
            {
                printf("  ---> ACEPTADA\n");
                return true;
            }
            else
            {
                printf("  ---> Rechazada\n");
            }
        }
    }

    return false;
}

int main()
{
    printf("=====================================\n");
    printf(" EJERCICIO 8 - MINIGREP\n");
    printf(" Automata de prueba: a.b*.c\n");
    printf(" Escriba 'salir' para terminar\n");
    printf("=====================================\n\n");

    AutaAFD mi_maquina;

    mi_maquina.total_nodos = 3;
    mi_maquina.total_rutas = 3;

    // Estados
    mi_maquina.nodos[0].indice = 0;
    mi_maquina.nodos[0].es_aceptacion = 0;

    mi_maquina.nodos[1].indice = 1;
    mi_maquina.nodos[1].es_aceptacion = 0;

    mi_maquina.nodos[2].indice = 2;
    mi_maquina.nodos[2].es_aceptacion = 1;

    // Transiciones
    mi_maquina.rutas[0].origen = 0;
    mi_maquina.rutas[0].letra = 'a';
    mi_maquina.rutas[0].destino = 1;

    mi_maquina.rutas[1].origen = 1;
    mi_maquina.rutas[1].letra = 'b';
    mi_maquina.rutas[1].destino = 1;

    mi_maquina.rutas[2].origen = 1;
    mi_maquina.rutas[2].letra = 'c';
    mi_maquina.rutas[2].destino = 2;

    char linea[200];

    while (true)
    {
        printf("Ingrese una linea: ");
        scanf("%s", linea);

        if (strcmp(linea, "salir") == 0)
            break;

        if (minigrep(mi_maquina, linea))
        {
            printf(">> COINCIDENCIA ENCONTRADA\n\n");
        }
        else
        {
            printf(">> NO SE ENCONTRO COINCIDENCIA\n\n");
        }
    }

    return 0;
}
