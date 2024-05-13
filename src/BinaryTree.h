#include "Settings.h"

// Structure représentant un noeud du tas binaire
typedef struct HeapPair
{
    float value; // distance
    int data;    // indice du noeud
} BinNode;

// Structure représentant un tas binaire
typedef struct Heap
{
    BinNode *root; // racine du tas
    int size;      // nombre d'éléments dans le tas
    int capacity;  // la taille du tableau maximal
} Heap;

// Inicialise un tas binaire vide
Heap *create_heap(int capacity);

// Supprime un tas binaire
void destroy_heap(Heap *heap);

// Insère un élément dans le tas binaire
void insert_heap(Heap *heap, float value, int data);

// Supprime l'élément minimum du tas binaire
BinNode delete_min(Heap *heap);

// Affiche le tas binaire
void print_heap(Heap *heap);