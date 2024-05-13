#include "Settings.h"
#include "Graph.h"
#include "ListInt.h"
#include "ShortestPath.h"
#include "BinaryTree.h"

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
    if (path == NULL)
    {
        printf("No path found\n");
    }
    else
    {
        Path_print(path);
        Path_destroy(path);
    }

    // TODO : Afficher le chemin le plus court entre les noeuds 1 et 1608 : sauvegarder le chemin dans un fichier geojson puis le charger dans umap.openstreetmap.fr

    Graph_destroy(Graph);
    Path_destroy(path);
    return EXIT_SUCCESS;

    // path (distance = 4591.143066) : [1,173,174,175,266,267,268,269,272,275,276,429,430,431,586,589,738,739,740,741,742,743,865,869,874,875,876,871,870,1003,1004,1005,1021,1017,1018,1012,1146,1145,1141,1166,1167,1348,1326,1328,1351,1350,1349,1456,1458,1455,1446,1451,1450,1454,1597,1595,1608]
}
