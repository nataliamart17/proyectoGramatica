#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 70

typedef struct NodoRegla {
    char izq[5];
    char der[MAX_LEN];
    struct NodoRegla *anterior;
    struct NodoRegla *siguiente;
} ReglaNodo;

typedef struct {
    ReglaNodo *inicio;
    ReglaNodo *fin;
} Gramatica;

void inicializarLista(Gramatica *gramatica);
void dividirCadena(char *cadena, char *izq, char *der);
int agregarNodo(Gramatica *gramatica, char *cadena);
void cargarGramatica(Gramatica *gramatica, char *nombreArchivo);
void mostrarGramaticaCompacta(Gramatica *gramatica);
void eliminarSaltoLinea(char *cadena);
void buscarCoincidencias(Gramatica *gramatica);
void mostrarGramaticaSinDuplicados(Gramatica *gramatica);
void particionarCadena(char *cadena, int posicion, char *izq, char *der);
void primerRecorrido(Gramatica *gramatica);
void segundoRecorrido(Gramatica *gramatica);
void envolverConParentesis(char *cadena);

int main() {
    char nombreArchivo[MAX_LEN];
    Gramatica gramatica;

    inicializarLista(&gramatica);

    printf("Ingrese el nombre del archivo: ");
    scanf("%s", nombreArchivo);

    cargarGramatica(&gramatica, nombreArchivo);
    mostrarGramaticaCompacta(&gramatica);
    buscarCoincidencias(&gramatica);
    mostrarGramaticaSinDuplicados(&gramatica);
    primerRecorrido(&gramatica);
    segundoRecorrido(&gramatica);
    if (gramatica.inicio != NULL) {
        printf("\n");
        printf("Regla de la gramática regular:\n%s -> %s\n", gramatica.inicio->izq, gramatica.inicio->der);
    }

    return 0;
}

void inicializarLista(Gramatica *gramatica) {
    gramatica->inicio = NULL;
    gramatica->fin = NULL;
}

void dividirCadena(char *cadena, char *izq, char *der) {
    char *tempIzq = strtok(cadena, "->");
    char *tempDer = strtok(NULL, "->");

    if (tempIzq && tempDer) {
        strncpy(izq, tempIzq, 4);
        izq[4] = '\0';

        strncpy(der, tempDer, MAX_LEN - 1);
        der[MAX_LEN - 1] = '\0';
    }
}

int agregarNodo(Gramatica *gramatica, char *cadena) {
    ReglaNodo *actual = gramatica->inicio;
    char izq[5], der[MAX_LEN];
    dividirCadena(cadena, izq, der);
    while (actual != NULL) {
        if (strcmp(actual->izq, izq) == 0) {
            strcat(actual->der, "|");
            strcat(actual->der, der);
            return 1;
        }
        actual = actual->siguiente;
    }

    ReglaNodo *nuevo = (ReglaNodo *)malloc(sizeof(ReglaNodo));
    if (!nuevo)
        return 0;
    strcpy(nuevo->izq, izq);
    strcpy(nuevo->der, der);
    nuevo->anterior = gramatica->fin;
    nuevo->siguiente = NULL;

    if (gramatica->fin)
        gramatica->fin->siguiente = nuevo;
    else
        gramatica->inicio = nuevo;

    gramatica->fin = nuevo;
    return 1;
}

void cargarGramatica(Gramatica *gramatica, char *nombreArchivo) {
    FILE *archivo;
    char cadena[MAX_LEN];
    char cadenaAux[MAX_LEN];

    archivo = fopen(nombreArchivo, "r");
    if (!archivo) {
        printf("Archivo no encontrado\n");
        return;
    }
    printf("Gramática original:\n");

    while (fgets(cadena, MAX_LEN, archivo) != NULL) {
        eliminarSaltoLinea(cadena);
        int i = 0, j = 0;
        for (i = 0; cadena[i] != '\0'; i++) {
            if (cadena[i] != ' ' && cadena[i] != '\t') {
                cadenaAux[j++] = cadena[i];
            }
        }
        cadenaAux[j] = '\0';

        printf("%s\n", cadenaAux);

        if (!agregarNodo(gramatica, cadenaAux)) {
            printf("Error al crear nodo con la regla: %s\n", cadenaAux);
        }
    }
    printf("\n");

    fclose(archivo);
}

void mostrarGramaticaCompacta(Gramatica *gramatica) {
    ReglaNodo *actual = gramatica->inicio;
    printf("Gramática compactada:\n");

    while (actual) {
        printf("%s -> %s\n", actual->izq, actual->der);
        actual = actual->siguiente;
    }
}

void eliminarSaltoLinea(char *cadena) {
    size_t len = strlen(cadena);
    if (len > 0 && cadena[len - 1] == '\n') {
        cadena[len - 1] = '\0';
    }
}

void buscarCoincidencias(Gramatica *gramatica) {
    ReglaNodo *actual = gramatica->inicio;

    while (actual != NULL) {
        char nuevaParteDer[MAX_LEN] = "";
        char resultadoFinal[MAX_LEN] = "";
        int lenIzq = strlen(actual->izq);
        int lenDer = strlen(actual->der);
        int i = 0, posNueva = 0, posResultado = 0;
        char parteEliminar[MAX_LEN] = "";
        int eliminado[MAX_LEN] = {0};
        
        while (i < lenDer) {
            int coincidencia = 1;

            for (int j = 0; j < lenIzq; j++) {
                if (i + j >= lenDer || actual->der[i + j] != actual->izq[j]) {
                    coincidencia = 0;
                    break;
                }
            }

            if (coincidencia) {
                if (i > 0) {
                    nuevaParteDer[posNueva++] = '{';
                    nuevaParteDer[posNueva++] = actual->der[i - 1];
                    nuevaParteDer[posNueva++] = '}';
                    strncat(parteEliminar, &actual->der[i - 1], 1);
                }

                i += lenIzq;

                if (i < lenDer && actual->der[i] == '|') {
                    i++;
                }

            } else {
                nuevaParteDer[posNueva++] = actual->der[i++];
            }
        }

        nuevaParteDer[posNueva] = '\0';

        for (int k = 0; nuevaParteDer[k] != '\0'; k++) {
            char *pos = strchr(parteEliminar, nuevaParteDer[k]);
            if (pos != NULL && !eliminado[pos - parteEliminar]) {
                eliminado[pos - parteEliminar] = 1;
            } else {
                resultadoFinal[posResultado++] = nuevaParteDer[k];
            }
        }

        resultadoFinal[posResultado] = '\0';
        strcpy(actual->der, resultadoFinal);

        actual = actual->siguiente;
    }
}

void mostrarGramaticaSinDuplicados(Gramatica *gramatica) {
    ReglaNodo *actual = gramatica->inicio;
    printf("\nGramática compactada y sin duplicados:\n");

    while (actual) {
        printf("%s -> %s\n", actual->izq, actual->der);
        actual = actual->siguiente;
    }
}

void primerRecorrido(Gramatica *gramatica) {
    ReglaNodo *actual = gramatica->inicio;
    ReglaNodo *aux;
    char *pos;

    while (actual != NULL) {
        aux = actual->siguiente;
        while (aux != NULL) {
            pos = strstr(aux->der, actual->izq);
            if (pos != NULL) {
                char temp[MAX_LEN];
                strcpy(temp, pos + strlen(actual->izq));
                *pos = '\0';
                strcat(aux->der, actual->der);
                strcat(aux->der, temp);
            }
            aux = aux->siguiente;
        }
        actual = actual->siguiente;
    }
    actual = gramatica->inicio;
    while (actual != NULL) {
        envolverConParentesis(actual->der);
        actual = actual->siguiente;
    }
}

void segundoRecorrido(Gramatica *gramatica) {
    ReglaNodo *actual = gramatica->fin;
    ReglaNodo *aux;
    char *pos;

    while (actual != NULL) {
        aux = actual->anterior;
        while (aux != NULL) {
            pos = strstr(aux->der, actual->izq);
            if (pos != NULL) {
                char temp[MAX_LEN];
                strcpy(temp, pos + strlen(actual->izq));
                *pos = '\0';
                strcat(aux->der, actual->der);
                strcat(aux->der, temp);
            }
            aux = aux->anterior;
        }
        actual = actual->anterior;
    }
}

void envolverConParentesis(char *cadena) {
    if (strchr(cadena, '|') != NULL) {
        char resultado[MAX_LEN];
        snprintf(resultado, MAX_LEN, "(%s)", cadena);
        strcpy(cadena, resultado);
    }
}
