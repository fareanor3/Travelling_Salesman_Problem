#include "Settings.h"
#include "ListInt.h"

ListInt *ListInt_create()
{
    ListInt *list = (ListInt *)calloc(1, sizeof(ListInt));
    AssertNew(list);

    ListIntNode *sentinel = &(list->sentinel);

    sentinel->next = sentinel;
    sentinel->prev = sentinel;
    sentinel->value = 0;

    list->nodeCount = 0;
    return list;
}

void ListInt_destroy(ListInt *list)
{
    if (list == NULL)
        return;

    ListIntNode *sentinel = &(list->sentinel);
    ListIntNode *curr = sentinel->next;
    while (curr != sentinel)
    {
        ListIntNode *next = curr->next;
        free(curr);
        curr = next;
    }
    free(list);
}

int ListInt_size(ListInt *list)
{
    assert(list);
    return list->nodeCount;
}

bool ListInt_isEmpty(ListInt *list)
{
    assert(list);
    return (list->nodeCount <= 0);
}

void ListInt_print(ListInt *list)
{
    assert(list);

    ListIntNode *sentinel = &(list->sentinel);
    ListIntNode *curr = sentinel->next;
    printf("[");
    while (curr != sentinel->prev)
    {
        printf("%d,", curr->value);
        curr = curr->next;
    }
    if (curr != sentinel)
    {
        printf("%d", curr->value);
    }
    printf("]\n");
}

void ListInt_popNode(ListInt *list, ListIntNode *node)
{
    assert(list->nodeCount > 0);
    assert(node != &(list->sentinel));

    ListIntNode *prev = node->prev;
    ListIntNode *next = node->next;
    prev->next = next;
    next->prev = prev;
    node->prev = node;
    node->next = node;

    list->nodeCount--;
}

void ListInt_insertNodeAfter(ListInt *list, ListIntNode *ref, ListIntNode *node)
{
    assert(list && ref && node);

    ListIntNode *next = ref->next;
    node->prev = ref;
    node->next = next;
    ref->next = node;
    next->prev = node;

    list->nodeCount++;
}

int ListInt_get(ListInt *list, int index)
{
    assert(list);
    assert(index >= 0 && index < list->nodeCount);

    ListIntNode *sentinel = &(list->sentinel);
    ListIntNode *node = sentinel->next;
    for (int i = 0; i < index; i++)
    {
        node = node->next;
    }
    return node->value;
}

int ListInt_getFirst(ListInt *list)
{
    return list->sentinel.next->value;
}

int ListInt_getLast(ListInt *list)
{
    return list->sentinel.prev->value;
}

void ListInt_insertFirst(ListInt *list, int value)
{
    ListIntNode *node = (ListIntNode *)calloc(1, sizeof(ListIntNode));
    AssertNew(node);
    node->value = value;
    ListInt_insertNodeAfter(list, &(list->sentinel), node);
}

void ListInt_insertLast(ListInt *list, int value)
{
    ListIntNode *node = (ListIntNode *)calloc(1, sizeof(ListIntNode));
    AssertNew(node);
    node->value = value;
    ListInt_insertNodeAfter(list, list->sentinel.prev, node);
}

int ListInt_popFirst(ListInt *list)
{
    if (list->nodeCount <= 0)
    {
        assert(false);
        return 0;
    }
    ListIntNode *node = list->sentinel.next;
    int value = node->value;
    ListInt_popNode(list, node);
    free(node);
    return value;
}

int ListInt_popLast(ListInt *list)
{
    if (list->nodeCount <= 0)
    {
        assert(false);
        return 0;
    }
    ListIntNode *node = list->sentinel.prev;
    int value = node->value;
    ListInt_popNode(list, node);
    free(node);
    return value;
}

int ListInt_isIn(ListInt *list, int element)
{
    ListIntNode *sentinel = &(list->sentinel);
    ListIntNode *node = sentinel->next;
    while (node != sentinel)
    {
        if (node->value == element)
            return true;
        node = node->next;
    }
    return false;
}

ListInt *ListInt_copy(ListInt *list)
{
    ListInt *newList = ListInt_create();

    ListIntNode *sentinel = &(list->sentinel);
    ListIntNode *node = sentinel->next;
    while (node != sentinel)
    {
        ListInt_insertLast(newList, node->value);
        node = node->next;
    }

    return newList;
}

void ListInt_concatenate(ListInt *list1, ListInt *list2)
{
    if (list2->nodeCount <= 0)
    {
        return;
    }

    list1->nodeCount += list2->nodeCount;
    list2->nodeCount = 0;

    ListIntNode *last1 = list1->sentinel.prev;
    ListIntNode *last2 = list2->sentinel.prev;
    ListIntNode *first2 = list2->sentinel.next;
    ListIntNode *sentinel1 = &(list1->sentinel);
    ListIntNode *sentinel2 = &(list2->sentinel);

    last1->next = first2;
    first2->prev = last1;

    last2->next = sentinel1;
    sentinel1->prev = last2;

    sentinel2->next = sentinel2;
    sentinel2->prev = sentinel2;
}

void ListInt_permute(ListInt *list, int indice1, int indice2)
{
    // Vérification des valeurs données au début
    if (list == NULL)
    {
        printf("Problème avec la liste\n");
        return;
    }
    if ((indice1 > (list->nodeCount - 1)) || (indice2 > (list->nodeCount - 1)))
    {
        printf("L'indice est plus haut que la quantité de point dans la liste\n");
        return;
    }
    ListIntIter *iter1 = ListIntIter_create(list);
    ListIntIter *iter2 = ListIntIter_create(list);
    for (int i = 0; i < indice1; i++)
        ListIntIter_next(iter1);
    for (int i = 0; i < indice2; i++)
        ListIntIter_next(iter2);

    int tmp = iter1->current->value;
    iter1->current->value = iter2->current->value;
    iter2->current->value = tmp;

    // On vérifie si l'indice 1 ou 2 sont la première valeur de la liste afin de changer la dernière valeur
    if (indice1 == 0)
        list->sentinel.prev->value = iter1->current->value;
    else if (indice2 == 0)
        list->sentinel.prev->value = iter2->current->value;

    ListIntIter_destroy(iter1);
    ListIntIter_destroy(iter2);
}

//--------------------------------------------------------------------------------------------------

ListIntIter *ListIntIter_create(ListInt *list)
{
    assert(list);

    ListIntIter *iter = (ListIntIter *)calloc(1, sizeof(ListIntIter));
    AssertNew(iter);

    iter->sentinel = &(list->sentinel);
    iter->current = iter->sentinel->next;

    return iter;
}

void ListIntIter_destroy(ListIntIter *iter)
{
    if (!iter)
        return;

    free(iter);
}

int ListIntIter_get(ListIntIter *iter)
{
    assert(iter && (iter->current != iter->sentinel));
    return iter->current->value;
}

void ListIntIter_next(ListIntIter *iter)
{
    assert(iter && (iter->current != iter->sentinel));
    iter->current = iter->current->next;
}

bool ListIntIter_isValid(ListIntIter *iter)
{
    assert(iter);
    return (iter->current != iter->sentinel);
}
