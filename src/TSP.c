#include "TSP.h"

PathMatrix *PathMatrix_create(int size)
{
    PathMatrix *matrix = (PathMatrix *)calloc(1, sizeof(PathMatrix));
    AssertNew(matrix);
    matrix->size = size;
    matrix->matrix = (Path **)calloc(size, sizeof(Path *));
    for (int i = 0; i < size; i++)
    {
        matrix->matrix[i] = (Path *)calloc(size, sizeof(Path));
        AssertNew(matrix->matrix[i]);
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
    assert(graph);

    int size = graph->size;
    if ((station < 0) || (station >= size) || (graph == NULL))
    {
        printf("Problem with the starting vertex or the graph");
        return NULL;
    }
    int prev = station;

    // Create a path
    Path *tournee = Path_create(station);
    if (tournee == NULL)
    {
        printf("Problem with creating the path");
        return NULL;
    }

    // Create an array to track if a node has been visited
    bool *passage = (bool *)calloc(size, sizeof(bool));
    if (passage == NULL)
    {
        printf("Problème avec l'allocation du tableau passage");
        Path_destroy(tournee);
        return NULL;
    }

    // Iterate until all nodes have been visited
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
            if (!passage[arclist->target] && arclist->weight < weight)
            {
                follower = arclist->target;
                weight = arclist->weight;
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

        // Add the closest node to the path
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
    assert(graph && list && ListInt_size(list) > 0);

    const int size = ListInt_size(list);
    Graph *subGraph = Graph_create(size);

    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            if (i == j)
            {
                pathMatrix->matrix[i][j].distance = 0.f;
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

float *Graph_acoGetProbabilities(Graph *graph, Graph *pheromones, int station, bool *explored, float alpha, float beta)
{
    assert(graph && pheromones && explored);

    const int size = Graph_size(graph);
    float *probabilities = (float *)calloc(size, sizeof(float));
    float sum = 0.f;

    for (int i = 0; i < size; i++)
    {
        if (!explored[i])
        {
            probabilities[i] = powf(*Graph_getArc(pheromones, station, i), alpha) * powf(1.f / *Graph_getArc(graph, station, i), beta);
            sum += probabilities[i];
        }
    }
    for (int i = 0; i < size; i++)
    {
        probabilities[i] /= sum;
    }

    return probabilities;
}

Path *Graph_acoConstructPath(Graph *graph, Graph *pheromones, int station, float alpha, float beta)
{
    assert(graph && pheromones);

    int size = Graph_size(graph);
    Path *path = Path_create(station);
    bool *explored = (bool *)calloc(size, sizeof(bool));
    int prev = station;

    while (path->list->nodeCount < size)
    {
        explored[prev] = true;
        float *probabilities = Graph_acoGetProbabilities(graph, pheromones, prev, explored, alpha, beta);

#ifdef _MSC_VER
        const float random = ((float)(rand() << 16) + (float)(rand() & 0xFFFFF)) / (float)RAND_MAX;
#else
        const float random = (float)rand() / (float)RAND_MAX;
#endif
        float sum = 0.f;
        int next = -1;

        for (int i = 0; i < size; i++)
        {
            if (!explored[i])
            {
                sum += probabilities[i];
                if (random <= sum)
                {
                    next = i;
                    break;
                }
            }
        }

        ListInt_insertLast(path->list, next);
        path->distance += *Graph_getArc(graph, prev, next);
        prev = next;
        free(probabilities);
    }

    free(explored);
    return path;
}

void Graph_acoPheromoneUpdatePath(Graph *const pheromones, Path *const path, float q)
{
    assert(pheromones && path);

    const float w = q / path->distance;

    int size = path->list->nodeCount;
    for (int i = 0; i < size - 1; i++)
    {
        int u = ListInt_get(path->list, i);
        int v = ListInt_get(path->list, i + 1);
        *Graph_getArc(pheromones, u, v) += w;
    }
}

void Graph_acoPheromoneGlobalUpdate(Graph *const pheromones, float rho)
{
    assert(pheromones);
    const float c = 1.f - rho;

    const int size = Graph_size(pheromones);
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            *Graph_getArc(pheromones, i, j) *= c;
        }
    }
}