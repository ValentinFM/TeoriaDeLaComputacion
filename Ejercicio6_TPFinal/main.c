#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 1. ESTRUCTURAS DEL AST Y GENERACIÓN DE ARCHIVO .DOT

int id_global_graphviz = 0;

typedef enum { NODO_OPERANDO, NODO_UNION, NODO_CONCAT, NODO_KLEENE } TipoNodo;

typedef struct NodoArbol {
    int id;
    TipoNodo tipo;
    char caracter;
    struct NodoArbol *rama_izq;
    struct NodoArbol *rama_der;
} NodoArbol;

NodoArbol* asignarMemoriaNodo() {
    NodoArbol* n = (NodoArbol*)malloc(sizeof(NodoArbol));
    if (!n) exit(EXIT_FAILURE);
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

// Funcion recursiva que escribe directamente en el archivo
void recorrer_nodos_dot(NodoArbol* raiz, FILE* archivo) {
    if (raiz == NULL) return;

    if (raiz->rama_izq != NULL) {
        fprintf(archivo, "  N%d[label=\"%c\"];\n", raiz->rama_izq->id, raiz->rama_izq->caracter);
        fprintf(archivo, "  N%d -> N%d\n", raiz->id, raiz->rama_izq->id);
        recorrer_nodos_dot(raiz->rama_izq, archivo);
    }

    if (raiz->rama_der != NULL) {
        fprintf(archivo, "  N%d[label=\"%c\"];\n", raiz->rama_der->id, raiz->rama_der->caracter);
        fprintf(archivo, "  N%d -> N%d\n", raiz->id, raiz->rama_der->id);
        recorrer_nodos_dot(raiz->rama_der, archivo);
    }
}

// Crea y guarda el archivo fisico .dot
void exportar_arbol_dot(NodoArbol* raiz, const char* nombre_archivo) {
    FILE* archivo = fopen(nombre_archivo, "w");
    if (!archivo) {
        printf("[ERROR] No se pudo crear el archivo %s\n", nombre_archivo);
        return;
    }

    fprintf(archivo, "digraph G {\n");
    if (raiz != NULL) {
        fprintf(archivo, "  N%d[label=\"%c\"];\n", raiz->id, raiz->caracter);
        recorrer_nodos_dot(raiz, archivo);
    }
    fprintf(archivo, "}\n");

    fclose(archivo);
    printf("[EXITO] Arbol AST guardado correctamente en: '%s'\n", nombre_archivo);
}

// 2. PARSER LL(1) MODIFICADO (Devuelve Nodos en vez de Int)

const char *cursor;
char string[64];

NodoArbol* E();
NodoArbol* Edash(NodoArbol* izq);
NodoArbol* T();
NodoArbol* Tdash(NodoArbol* izq);
NodoArbol* F();
NodoArbol* Fdash(NodoArbol* izq);
NodoArbol* P();

int es_alfabeto(char c) {
    return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || (c == 'E');
}

int main() {
    puts("=== PUNTO 6: PARSER LL(1) A ARCHIVO .DOT ===");
    puts("Enter the Regular Expression:");
    scanf("%s", string);
    cursor = string;

    puts("");
    puts("Input          Action");
    puts("--------------------------------");

    NodoArbol* arbolAST = E();

    if (arbolAST != NULL && *cursor == '\0') {
        puts("--------------------------------");
        puts("Regular Expression is successfully parsed.");

        // Aca se genera el archivo automaticamente
        exportar_arbol_dot(arbolAST, "arbol_punto6.dot");

    } else {
        puts("--------------------------------");
        puts("Error in parsing Regular Expression.");
    }

    return 0;
}

// Grammar rule: E -> T E'
NodoArbol* E() {
    if (es_alfabeto(*cursor) || *cursor == '(') {
        printf("%-16s E -> T E'\n", cursor);
        NodoArbol* nodo_t = T();
        if (nodo_t != NULL) {
            return Edash(nodo_t);
        }
    }
    return NULL;
}

// Grammar rule: E' -> '|' T E' | $
NodoArbol* Edash(NodoArbol* izq) {
    if (*cursor == '|') {
        printf("%-16s E' -> | T E'\n", cursor);
        cursor++;
        NodoArbol* nodo_t = T();
        if (nodo_t != NULL) {
            NodoArbol* nuevo_izq = fabricarOperadorBinario('|', izq, nodo_t);
            return Edash(nuevo_izq);
        }
        return NULL;
    } else if (*cursor == ')' || *cursor == '\0') {
        printf("%-16s E' -> $\n", cursor);
        return izq;
    }
    return NULL;
}

// Grammar rule: T -> F T'
NodoArbol* T() {
    if (es_alfabeto(*cursor) || *cursor == '(') {
        printf("%-16s T -> F T'\n", cursor);
        NodoArbol* nodo_f = F();
        if (nodo_f != NULL) {
            return Tdash(nodo_f);
        }
    }
    return NULL;
}

// Grammar rule: T' -> '.' F T' | $
NodoArbol* Tdash(NodoArbol* izq) {
    if (*cursor == '.') {
        printf("%-16s T' -> . F T'\n", cursor);
        cursor++;
        NodoArbol* nodo_f = F();
        if (nodo_f != NULL) {
            NodoArbol* nuevo_izq = fabricarOperadorBinario('.', izq, nodo_f);
            return Tdash(nuevo_izq);
        }
        return NULL;
    } else if (*cursor == '|' || *cursor == ')' || *cursor == '\0') {
        printf("%-16s T' -> $\n", cursor);
        return izq;
    }
    return NULL;
}

// Grammar rule: F -> P F'
NodoArbol* F() {
    if (es_alfabeto(*cursor) || *cursor == '(') {
        printf("%-16s F -> P F'\n", cursor);
        NodoArbol* nodo_p = P();
        if (nodo_p != NULL) {
            return Fdash(nodo_p);
        }
    }
    return NULL;
}

// Grammar rule: F' -> '*' F' | $
NodoArbol* Fdash(NodoArbol* izq) {
    if (*cursor == '*') {
        printf("%-16s F' -> * F'\n", cursor);
        cursor++;
        NodoArbol* nuevo_izq = fabricarClausura(izq);
        return Fdash(nuevo_izq);
    } else if (*cursor == '|' || *cursor == '.' || *cursor == ')' || *cursor == '\0') {
        printf("%-16s F' -> $\n", cursor);
        return izq;
    }
    return NULL;
}

// Grammar rule: P -> '(' E ')' | a | b | c ...
NodoArbol* P() {
    if (*cursor == '(') {
        printf("%-16s P -> ( E )\n", cursor);
        cursor++;
        NodoArbol* nodo_e = E();
        if (nodo_e != NULL && *cursor == ')') {
            cursor++;
            return nodo_e;
        }
        return NULL;
    } else if (es_alfabeto(*cursor)) {
        printf("%-16s P -> %c\n", cursor, *cursor);
        NodoArbol* hoja = fabricarHoja(*cursor);
        cursor++;
        return hoja;
    }
    return NULL;
}
