#include "TSP.h"

PathMatrix *PathMatrix_create(int size)
{
    PathMatrix *matrix = (PathMatrix *)calloc(1, sizeof(PathMatrix));
    matrix->size = size;
    matrix->matrix = (Path **)calloc(size, sizeof(Path *));
    for (int i = 0; i < size; i++)
    {
        matrix->matrix[i] = (Path *)calloc(size, sizeof(Path));
    }
    return matrix;
}

void PathMatrix_destroy(PathMatrix *matrix)
{
    if (matrix == NULL)
        return;

    for (int i = 0; i < matrix->size; i++)
    {
        free(matrix->matrix[i]);
    }
    free(matrix->matrix);
    free(matrix);
}

Path *Graph_tspFromHeuristic(Graph *graph, int station)
{
    int size = graph->size;
    if ((station < 0) || (station > size) || (graph == NULL))
    {
        printf("Problème avec le sommet de départ ou le graph");
        return NULL;
    }
    int prev = station;

    // On crée un path
    Path *tournee = Path_create(station);

    // On crée un tableau qui permet de savoir si on est déjà passé quelque part
    bool *passage = (bool *)calloc(size, sizeof(bool));

    // On tourne tant qu'on n'a pas visité tout les noeuds
    while (tournee->list->nodeCount < size)
    {
        passage[prev] = true;
        int follower = -1;
        float weight = INFINITY;
        // On va chercher les points accessibles par celui sur lequel on est actuellement
        for (int i = 0; i < Graph_getArcCount(graph, prev); i++)
        {
            ArcList *arclist = Graph_getArcList(graph, prev);
            if (passage[arclist->target] == false)
            {
                // On vérifie si ce nouvel arc est plus léger que l'ancien ou pas
                if (arclist->weight < weight)
                {
                    follower = arclist->target;
                    weight = arclist->weight;
                }
            }
            arclist = arclist->next;
        }

        // On rajoute au chemin le point le plus proche de celui dont on est parti
        ListInt_insertLast(tournee->list, follower);

        // On ajoute la distance
        tournee->distance += weight;
        prev = follower;
    }
    // On fait nos free et on renvoit le path
    free(passage);
    return tournee;
}

Graph *Graph_getSubGraph(Graph *graph, ListInt *list, PathMatrix *pathMatrix)
{
    assert(graph || list || ListInt_size(list) > 0);

    int size = ListInt_size(list);
    Graph *subGraph = Graph_create(size);

    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if (i == j)
            {
                pathMatrix->matrix[i][j].distance = 0;
            }
            else if (i < j)
            {
                int id1 = ListInt_get(list, i);
                int id2 = ListInt_get(list, j);
                Path *path = Graph_shortestPath(graph, id1, id2);
                pathMatrix->matrix[i][j] = *path;
                pathMatrix->matrix[j][i] = *path;
                Graph_setArc(subGraph, i, j, path->distance);
                Graph_setArc(subGraph, j, i, path->distance);
                Path_destroy(path);
            }
        }
    }
    return subGraph;
}