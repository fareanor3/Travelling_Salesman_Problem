#include "Settings.h"
#include "Graph.h"
#include "ListInt.h"
#include "ShortestPath.h"
#include "Interface.h"
#include "TSP.h"

#define File "../TPF_Donnees/Data/laval_graph.txt"

int main()
{
    Graph *graph = Graph_load(File);
    if (graph == NULL)
    {
        printf("Error loading the graph\n");
        return EXIT_FAILURE;
    }
    // int start = 1, end = 1608;
    // Path *path = Graph_shortestPath(graph, start, end);

    ListInt *list = ListInt_create();
    ListInt_insertLast(list, 482);
    ListInt_insertLast(list, 2504);
    ListInt_insertLast(list, 733);
    ListInt_insertLast(list, 826);
    ListInt_insertLast(list, 172);
    ListInt_insertLast(list, 2361);
    ListInt_insertLast(list, 2605);

    PathMatrix *pathMatrix = PathMatrix_create(7);

    Graph *graph2 = Graph_getSubGraph(graph, list, pathMatrix);
    Graph_print(graph2);
    Path *path2 = Graph_tspFromHeuristic(graph2, 0);
    // Path_print(path);
    // Path_print(path2);
    /*
    int test = Creation_geojson(path, Graph_size(graph));
    if (test == 1)
    {
        printf("Problème dans la création du fichier geojson\n");
        Path_destroy(path);
        Path_destroy(path2);
        Graph_destroy(graph);
        return EXIT_FAILURE;
    }
    */
    printf("Distance avec TSP : %.1f\n", path2->distance);
    for (ListIntNode *pnt = path2->list->sentinel.next; pnt != &path2->list->sentinel; pnt = pnt->next)
    {
        printf("%d ", pnt->value);
    }
    printf("\n");
    // Path_destroy(path);
    Path_destroy(path2);
    Graph_destroy(graph);
    ListInt_destroy(list);
    PathMatrix_destroy(pathMatrix);
    Graph_destroy(graph2);
    // TODO : Afficher le chemin le plus court entre les noeuds 1 et 1608 : sauvegarder le chemin dans un fichier geojson puis le charger dans umap.openstreetmap.fr

    return EXIT_SUCCESS;
}