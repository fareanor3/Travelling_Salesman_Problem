#include "TSP.h"

/// @brief Calcule une tounée dans un graphe en se basant sur un algorithme glouton.
/// @param graph le graphe des distances. Il doit être complet.
/// @param station l'identifiant du sommet de départ et d'arrivée de la tournée.
/// @return La tournée.
Path *Graph_tspFromHeuristic(Graph *graph, int station)
{
    int size = Graph_size(graph);
    assert(0 <= station && station < size);

    bool *explored = (bool *)calloc(size, sizeof(bool));
    AssertNew(explored);

    Path *path = Path_create(station);

    int curr = station;
    while (path->list->nodeCount < size)
    {
        explored[curr] = true;

        Path *shortestPath = NULL;
        for (int i = 0; i < size; i++)
        {
            if (!explored[i])
            {
                Path *p = Graph_shortestPath(graph, curr, i);
                if (shortestPath == NULL || p->distance < shortestPath->distance)
                {
                    if (shortestPath != NULL)
                    {
                        Path_destroy(shortestPath);
                    }
                    shortestPath = p;
                }
                else
                {
                    Path_destroy(p);
                }
            }
        }

        curr = ListInt_getFirst(shortestPath->list);
        ListInt_popFirst(shortestPath->list);
        path->distance += shortestPath->distance;

        Path_destroy(shortestPath);
    }

    return path;
}

Graph *Graph_getSubGraph(Graph *graph, ListInt *list)
{
    int size = ListInt_size(list);
    Graph *subGraph = Graph_create(size);

    for (int i = 0; i < size; i++)
    {
    }
    Graph_print(subGraph);
    return subGraph;
}