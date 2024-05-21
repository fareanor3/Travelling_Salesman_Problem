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
    bool *visited = (bool *)calloc(size, sizeof(bool));
    if (visited == NULL)
    {
        printf("Problem allocating the visited array");
        Path_destroy(tournee);
        return NULL;
    }

    // Iterate until all nodes have been visited
    while (tournee->list->nodeCount < size)
    {
        visited[prev] = true;
        int nextNode = -1;
        float minWeight = INFINITY;

        // Get the list of arcs from the current node
        ArcList *arcList = Graph_getArcList(graph, prev);
        if (arcList == NULL)
        {
            printf("Problem with the arc list");
            free(visited);
            Path_destroy(tournee);
            return NULL;
        }

        // Iterate through the arcs to find the closest unvisited node
        for (int i = 0; i < Graph_getArcCount(graph, prev); i++)
        {
            if (!visited[arcList->target] && arcList->weight < minWeight)
            {
                nextNode = arcList->target;
                minWeight = arcList->weight;
            }
            arcList = arcList->next;
        }

        if (nextNode == -1)
        {
            printf("No following node found, there might be a problem in the graph");
            free(visited);
            Path_destroy(tournee);
            return NULL;
        }

        // Add the closest node to the path
        ListInt_insertLast(tournee->list, nextNode);

        // Add the distance
        tournee->distance += minWeight;
        prev = nextNode;
    }

    // Free the visited array and return the path
    free(visited);
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