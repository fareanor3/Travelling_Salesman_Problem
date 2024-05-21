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
    int start = 1, end = 1608;
    Path *path = Graph_shortestPath(graph, start, end);
    Path *path2 = Graph_tspFromHeuristic(graph, start);
    Path_print(path);
    Path_print(path2);
    int test = Creation_geojson(path, Graph_size(graph));
    if (test == 1)
    {
        printf("Problème dans la création du fichier geojson\n");
        Path_destroy(path);
        Graph_destroy(graph);
        return EXIT_FAILURE;
    }

    Path_destroy(path);
    Graph_destroy(graph);

    // TODO : Afficher le chemin le plus court entre les noeuds 1 et 1608 : sauvegarder le chemin dans un fichier geojson puis le charger dans umap.openstreetmap.fr

    return EXIT_SUCCESS;
}