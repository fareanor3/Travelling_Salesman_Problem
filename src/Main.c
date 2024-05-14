#include "Settings.h"
#include "Graph.h"
#include "ListInt.h"
#include "ShortestPath.h"

#define FILE "../TPF_Donnees/Data/laval_graph.txt"

int main()
{
    Graph *Graph = Graph_load(FILE);
    if (Graph == NULL)
    {
        printf("Error loading the graph\n");
        return EXIT_FAILURE;
    }
    Path *path = Graph_shortestPath(Graph, 1, 1608);
    Path_print(path);
    Path_destroy(path);
    Graph_destroy(Graph);

    // TODO : Afficher le chemin le plus court entre les noeuds 1 et 1608 : sauvegarder le chemin dans un fichier geojson puis le charger dans umap.openstreetmap.fr

    return EXIT_SUCCESS;
}