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
    if ((station < 0) || (station >= size) || (graph == NULL))
    {
        printf("Problème avec le sommet de départ ou le graph");
        return NULL;
    }
    int prev = station;

    // On crée un path
    Path *tournee = Path_create(station);
    if (tournee == NULL)
    {
        printf("Problème avec la création du path");
        return NULL;
    }

    // On crée un tableau qui permet de savoir si on est déjà passé quelque part
    bool *passage = (bool *)calloc(size, sizeof(bool));
    if (passage == NULL)
    {
        printf("Problème avec l'allocation du tableau passage");
        Path_destroy(tournee);
        return NULL;
    }

    // On tourne tant qu'on n'a pas visité tout les noeuds
    while (tournee->list->nodeCount < size)
    {
        passage[prev] = true;
        // printf("Voici le point actuel %d -> ", prev);
        int follower = -1;
        float weight = INFINITY;

        // On récupère la liste des noeuds accessibles par le point auquel on est
        ArcList *arclist = Graph_getArcList(graph, prev);
        if (arclist == NULL)
        {
            printf("Problème avec la liste des arcs");
            free(passage);
            Path_destroy(tournee);
            return NULL;
        }

        // On va chercher les points accessibles par celui sur lequel on est actuellement
        for (int i = 0; i < Graph_getArcCount(graph, prev); i++)
        {
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

        if (follower == -1)
        {
            printf("Aucun point suivant trouvé, il y a peut-être un problème dans le graph");
            free(passage);
            Path_destroy(tournee);
            return NULL;
        }

        // On rajoute au chemin le point le plus proche de celui dont on est parti
        ListInt_insertLast(tournee->list, follower);

        // On ajoute la distance
        // printf("Voici le follower %d\n", follower);
        tournee->distance += weight;
        prev = follower;

        // On ajoute la node de début à la fin afin de rentrer
        if (tournee->list->nodeCount == size)
        {
            arclist = Graph_getArcList(graph, follower);
            while (arclist != NULL)
            {
                // On vérifie si on a atteint le lien entre le dernier point et le premier afin de boucler
                if (arclist->target == station)
                {
                    tournee->distance += arclist->weight;
                    ListInt_insertLast(tournee->list, station);
                }
                arclist = arclist->next;
            }
        }
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