#include "BinaryTree.h"
#include "Settings.h"

Heap *create_heap(int capacity)
{
    if (capacity <= 0)
        return NULL; // invalid capacity

    Heap *heap = calloc(1, sizeof(Heap));
    if (heap == NULL)
        return NULL; // out of memory

    heap->root = calloc(sizeof(BinNode), capacity);
    if (heap->root == NULL)
    {
        free(heap); // out of memory
        return NULL;
    }
    heap->size = 0;
    heap->capacity = capacity;
    return heap;
}

// free the heap
void destroy_heap(Heap *heap)
{
    if (heap != NULL)
    {
        if (heap->root != NULL)
            free(heap->root);
        free(heap);
    }
}

void insert_heap(Heap *heap, float value, int data)
{
    if (heap == NULL || heap->size >= heap->capacity)
    {
        // invalid heap or heap is full, do nothing
        return;
    }
    int index = heap->size;
    heap->size++;
    heap->root[index].value = value;
    heap->root[index].data = data;
    // Percolate up
    while (index > 0 && heap->root[(index - 1) / 2].value > heap->root[index].value)
    {
        // Swap parent and child
        BinNode temp = heap->root[(index - 1) / 2];
        heap->root[(index - 1) / 2] = heap->root[index];
        heap->root[index] = temp;
        index = (index - 1) / 2;
    }
}

// Supprime la racine du tas binaire (l'�l�ment le plus petit) et retourne sa valeur ; pour supprimer retir le premier et réinsert le dernier à la place

BinNode delete_min(Heap *heap)
{
    if (heap == NULL || heap->size == 0)
    {
        // invalid heap or heap is empty, return NULL
        printf("heap is empty or not exist\n");
        return (BinNode){0};
    }

    // sauvegarde la racine pour la retourner à la fin de la fonction
    BinNode min_node = heap->root[0];
    // deplace le noeud de la derniere position vers la racine
    heap->root[0] = heap->root[heap->size - 1];
    heap->size--;

    // percolate down pour maintenir la propriété du tas
    int index = 0;
    while (index * 2 + 1 < heap->size)
    {
        int left_child_index = index * 2 + 1;
        int right_child_index = index * 2 + 2;
        int min_child_index = left_child_index;

        if (right_child_index < heap->size && heap->root[right_child_index].value < heap->root[left_child_index].value)
        {
            min_child_index = right_child_index;
        }

        if (heap->root[index].value > heap->root[min_child_index].value)
        {
            // swap parent and child nodes
            BinNode temp = heap->root[index];
            heap->root[index] = heap->root[min_child_index];
            heap->root[min_child_index] = temp;
            // move down to child node
            index = min_child_index;
        }
        else
        {
            // heap order property is satisfied, stop percolating down
            break;
        }
    }

    return min_node;
}

void print_heap(Heap *heap)
{
    if (heap == NULL || heap->size <= 0)
    {
        printf("Empty heap\n");
        return;
    }

    printf("Heap contents: ");
    for (int i = 0; i < heap->size; i++)
    {
        // affiche le noeud et sa valeur (distance)
        printf("(%d, %.2f) ", heap->root[i].data, heap->root[i].value);
    }
    printf("\n");
}