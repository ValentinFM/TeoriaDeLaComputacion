#include <stdio.h>
#include <stdlib.h>

// ID global para generar el gráfico en formato dot del pto 2
int id_global_graphviz = 0;

// definimnos tipo de nodo, 'E' (lambda), unión '|', concatenación '.', Kleene '*'
typedef enum {
    NODO_OPERANDO,
    NODO_UNION,
    NODO_CONCAT,
    NODO_KLEENE
} TipoNodo;

// Estructura principal del arbol
typedef struct NodoArbol {
    int id;
    TipoNodo tipo;
    char caracter;
    struct NodoArbol *rama_izq;
    struct NodoArbol *rama_der;
} NodoArbol;

// Funciones

NodoArbol* asignarMemoriaNodo() {
    NodoArbol* n = (NodoArbol*)malloc(sizeof(NodoArbol));
    if (n == NULL) {
        printf("Error fatal: No hay memoria RAM disponible.\n");
        exit(1); // sale
    }
    n->id = id_global_graphviz++;
    n->rama_izq = NULL;
    n->rama_der = NULL;
    return n;
}

// Constructores

// 1. Constructor para hojas (letras, números, o 'E' para lambda)
NodoArbol* fabricarHoja(char simbolo) {
    NodoArbol* nodo = asignarMemoriaNodo();
    nodo->tipo = NODO_OPERANDO;
    nodo->caracter = simbolo;
    return nodo;
}

// 2. Constructor para operadores binarios ( | y . )
NodoArbol* fabricarOperadorBinario(char operador, NodoArbol* izq, NodoArbol* der) {
    NodoArbol* nodo = asignarMemoriaNodo();
    nodo->tipo = (operador == '|') ? NODO_UNION : NODO_CONCAT;
    nodo->caracter = operador;
    nodo->rama_izq = izq;
    nodo->rama_der = der;
    return nodo;
}

// 3. Constructor para operador ( * )
NodoArbol* fabricarClausura(NodoArbol* base) {
    NodoArbol* nodo = asignarMemoriaNodo();
    nodo->tipo = NODO_KLEENE;
    nodo->caracter = '*';
    nodo->rama_izq = base; // El operando siempre cuelga de la rama izquierda
    return nodo;
}

// Utilidades

// Recorrido InOrder
void mostrarRegex(NodoArbol* raiz) {
    if (raiz == NULL) return;

    if (raiz->tipo != NODO_OPERANDO) printf("(");
    mostrarRegex(raiz->rama_izq);

    printf("%c", raiz->caracter);

    mostrarRegex(raiz->rama_der);
    if (raiz->tipo != NODO_OPERANDO) printf(")");
}

// Limpieza de memoria (PostOrder)
void destruirArbol(NodoArbol* raiz) {
    if (raiz == NULL) return;
    destruirArbol(raiz->rama_izq);
    destruirArbol(raiz->rama_der);
    free(raiz);
}

// --- FUNCIONES DEL PUNTO 2 (SALIDA FORMATO DOT) ---

void recorrer_nodos_dot(NodoArbol* raiz) {
    if (raiz == NULL) return;

    // Hijo izquierdo
    if (raiz->rama_izq != NULL) {
        printf("  N%d[label=\"%c\"];\n", raiz->rama_izq->id, raiz->rama_izq->caracter);
        printf("  N%d -> N%d\n", raiz->id, raiz->rama_izq->id); // Sin punto y coma al final
        recorrer_nodos_dot(raiz->rama_izq);
    }

    // Hijo derecho
    if (raiz->rama_der != NULL) {
        printf("  N%d[label=\"%c\"];\n", raiz->rama_der->id, raiz->rama_der->caracter);
        printf("  N%d -> N%d\n", raiz->id, raiz->rama_der->id); // Sin punto y coma al final
        recorrer_nodos_dot(raiz->rama_der);
    }
}

void generar_salida_dot(NodoArbol* raiz) {
    printf("digraph G {\n");

    if (raiz != NULL) {
        // Imprime primero el nodo principal antes de recorrer sus hijos
        printf("  N%d[label=\"%c\"];\n", raiz->id, raiz->caracter);
        recorrer_nodos_dot(raiz);
    }

    printf("}\n");
}


// Bloque de Prueba
int main() {
    printf("--- Generador del arbol para Expresiones Regulares ---\n\n");

    // Simulamos la construcción de: (x.y)|z*

    // 1. Armamos (x.y)
    NodoArbol* literalX = fabricarHoja('x');
    NodoArbol* literalY = fabricarHoja('y');
    NodoArbol* bloqueConcat = fabricarOperadorBinario('.', literalX, literalY);

    // 2. Armamos z*
    NodoArbol* literalZ = fabricarHoja('z');
    NodoArbol* bloqueKleene = fabricarClausura(literalZ);

    // 3. Unimos todo con el OR
    NodoArbol* arbolFinal = fabricarOperadorBinario('|', bloqueConcat, bloqueKleene);

    // Salida InOrder
    printf("Expresion regular resultante: ");
    mostrarRegex(arbolFinal);
    printf("\n\n");

    // Salida DOT (Punto 2)
    printf("--- Salida en formato DOT ---\n");
    generar_salida_dot(arbolFinal);

    // Liberar recursos
    destruirArbol(arbolFinal);

    return 0;
}
