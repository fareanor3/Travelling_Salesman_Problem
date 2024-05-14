#include "ShortestPath.h"

Path *Graph_shortestPath(Graph *graph, int start, int end)
{
    int size = Graph_size(graph);

    assert(0 <= start && start < size);
    assert(end < size);

    int *predecessors = (int *)calloc(size, sizeof(int));
    AssertNew(predecessors);

    float *distances = (float *)calloc(size, sizeof(float));
    AssertNew(distances);

    Graph_dijkstra(graph, start, end, predecessors, distances);

    Path *path = Graph_dijkstraGetPath(predecessors, distances, end);

    free(predecessors);
    predecessors = NULL;
    free(distances);
    distances = NULL;

    return path;
}

void Graph_dijkstra(Graph *graph, int start, int end, int *predecessors, float *distances)
{
    int size = Graph_size(graph);

    assert(0 <= start && start < size);
    assert(end < size);

    bool *explored = (bool *)calloc(size, sizeof(bool));
    AssertNew(explored);

    Heap *priorityQueue = create_heap(size);
    AssertNew(priorityQueue);

    // Initialization of distances and predecessors
    for (int i = 0; i < size; i++)
    {
        predecessors[i] = -1;
        distances[i] = INFINITY;
    }
    distances[start] = 0.0f;
    // Add the start node to the heap
    insert_heap(priorityQueue, 0.0f, start);

    while (priorityQueue->size > 0)
    {
        // Remove the first element from the heap
        BinNode minNode = delete_min(priorityQueue);

        // Mark the node as explored
        int currID = minNode.data;
        if (explored[currID])
            continue;
        explored[currID] = true;

        // If we have reached the destination
        if (currID == end)
            break;
        ArcList *successors = Graph_getArcList(graph, currID);

        while (successors)
        {
            // Update distances and predecessors
            if (!explored[successors->target])
            {
                // Update distances and predecessors only if the node is not explored
                float newDist = distances[currID] + successors->weight;
                if (newDist < distances[successors->target])
                {
                    distances[successors->target] = newDist;
                    predecessors[successors->target] = currID;
                    insert_heap(priorityQueue, newDist, successors->target);
                }
            }

            successors = successors->next;
        }
        free(successors);
    }

    free(explored);
    destroy_heap(priorityQueue);
}

Path *Graph_dijkstraGetPath(int *predecessors, float *distances, int end)
{
    assert(predecessors && distances);
    assert(end >= 0);

    if (predecessors[end] < 0)
    {
        return NULL;
    }

    Path *path = Path_create(end);
    int currID = end;

    while (predecessors[currID] >= 0)
    {
        currID = predecessors[currID];
        ListInt_insertFirst(path->list, currID);
    }

    path->distance = distances[end];

    return path;
}

Path *Path_create(int start)
{
    Path *path = (Path *)calloc(1, sizeof(Path));
    AssertNew(path);

    path->distance = 0.0f;
    path->list = ListInt_create();
    ListInt_insertLast(path->list, start);

    return path;
}

void Path_destroy(Path *path)
{
    if (path == NULL)
        return;

    ListInt_destroy(path->list);
    free(path);
}

void Path_print(Path *path)
{
    if (path == NULL)
    {
        printf("path : NULL\n");
        return;
    }

    printf("path (distance = %f) : ", path->distance);
    ListInt_print(path->list);
}
