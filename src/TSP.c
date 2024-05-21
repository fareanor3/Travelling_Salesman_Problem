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