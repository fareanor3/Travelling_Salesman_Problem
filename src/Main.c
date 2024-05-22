#include "Settings.h"
#include "Graph.h"
#include "ListInt.h"
#include "ShortestPath.h"
#include "Interface.h"
#include "TSP.h"

#define File "../TPF_Donnees/Data/laval_graph.txt"

int main()
{

    srand((unsigned int)time(NULL));

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
    pathMatrix->matrix[0][0].distance;
    Graph *graph2 = Graph_getSubGraph(graph, list, pathMatrix);
    Path *path2 = Graph_tspFromHeuristic(graph2, 0);
    Path *path3 = Graph_tspFromACO(graph2, 0, 1000, 100, 2.0, 3.0, 0.1, 2.0);
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
    printf("Distance avec ACO : %.1f\n", path3->distance);
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
    Path_destroy(path3);
    // TODO : Afficher le chemin le plus court entre les noeuds 1 et 1608 : sauvegarder le chemin dans un fichier geojson puis le charger dans umap.openstreetmap.fr

    return EXIT_SUCCESS;
}

int Passage_Dijkstra()
{
    /*
        #include "Settings.h"
        #include "Graph.h"
        #include "ListInt.h"
        #include "ShortestPath.h"
    */

    char *fpath = calloc(100, sizeof(char));
    char *fcoor = calloc(100, sizeof(char));
    int start = 0, end = 0;
    scanf("%s", fpath);
    scanf("%s", fcoor);
    scanf("%d %d", &start, &end);
    Graph *graph = Graph_load(fpath);
    if (graph == NULL)
    {
        printf("Erreur en chargeant le graph\n");
        free(fpath);
        free(fcoor);
        return EXIT_FAILURE;
    }

    Path *path = Graph_shortestPath(graph, start, end);
    if (path == NULL)
    {
        printf("Erreur en chargeant le path\n");
        free(fpath);
        free(fcoor);
        Graph_destroy(graph);
    }
    printf("%.1f\n", path->distance);
    printf("%d\n", path->list->nodeCount);
    for (ListIntNode *pnt = path->list->sentinel.next; pnt != &path->list->sentinel; pnt = pnt->next)
        printf("%d ", pnt->value);
    free(fpath);
    free(fcoor);
    Path_destroy(path);
    Graph_destroy(graph);
    return 0;
}

int Passage_PathMatrix()
{
    /*
        #include "Settings.h"
        #include "Graph.h"
        #include "ListInt.h"
        #include "ShortestPath.h"
        #include "TSP.h"
    */
    char *fpath = calloc(100, sizeof(char));
    char *fcoor = calloc(100, sizeof(char));
    int nb_points = 0;
    int point = -1;
    scanf("%s", fpath);
    scanf("%s", fcoor);
    scanf("%d", &nb_points);
    ListInt *list = ListInt_create();
    for (int i = 0; i < nb_points; i++)
    {
        scanf("%d ", &point);
        ListInt_insertLast(list, point);
    }
    PathMatrix *pathMatrix = PathMatrix_create(nb_points);
    Graph *graph = Graph_load(fpath);
    if ((graph == NULL) || (pathMatrix == NULL))
    {
        printf("Erreur en créant le graph ou le pathMatrix\n");
        free(fpath);
        free(fcoor);
        ListInt_destroy(list);
        PathMatrix_destroy(pathMatrix);
        Graph_destroy(graph);
        return EXIT_FAILURE;
    }
    Graph *sub_graph = Graph_getSubGraph(graph, list, pathMatrix);
    if ((sub_graph == NULL))
    {
        printf("Erreur en créant le sous-graph ou le path\n");
        free(fpath);
        free(fcoor);
        ListInt_destroy(list);
        PathMatrix_destroy(pathMatrix);
        Graph_destroy(graph);
        Graph_destroy(sub_graph);
        return EXIT_FAILURE;
    }
    for (int i = 0; i < nb_points; i++)
    {
        for (int j = 0; j < nb_points; j++)
        {
            printf("%d %d %.1f\n", i, j, pathMatrix->matrix[i][j].distance);
        }
    }
    free(fpath);
    free(fcoor);
    ListInt_destroy(list);
    PathMatrix_destroy(pathMatrix);
    Graph_destroy(graph);
    Graph_destroy(sub_graph);
    return 0;
}

int Passage_TSPHeuristic()
{
    /*
        #include "Settings.h"
        #include "Graph.h"
        #include "ListInt.h"
        #include "ShortestPath.h"
        #include "TSP.h"
    */

    char *fpath = calloc(100, sizeof(char));
    char *fcoor = calloc(100, sizeof(char));
    int nb_points = 0;
    int point = -1;
    scanf("%s", fpath);
    scanf("%s", fcoor);
    scanf("%d", &nb_points);
    ListInt *list = ListInt_create();
    for (int i = 0; i < nb_points; i++)
    {
        scanf("%d ", &point);
        ListInt_insertLast(list, point);
    }

    PathMatrix *pathMatrix = PathMatrix_create(nb_points);
    Graph *graph = Graph_load(fpath);
    if ((graph == NULL) || (pathMatrix == NULL))
    {
        printf("Erreur en créant le graph ou le pathMatrix\n");
        free(fpath);
        free(fcoor);
        ListInt_destroy(list);
        PathMatrix_destroy(pathMatrix);
        Graph_destroy(graph);
        return EXIT_FAILURE;
    }
    Graph *sub_graph = Graph_getSubGraph(graph, list, pathMatrix);
    Path *path = Graph_tspFromHeuristic(sub_graph, 0);
    if ((sub_graph == NULL) || (path == NULL))
    {
        printf("Erreur en créant le sous-graph ou le path\n");
        free(fpath);
        free(fcoor);
        ListInt_destroy(list);
        PathMatrix_destroy(pathMatrix);
        Path_destroy(path);
        Graph_destroy(graph);
        Graph_destroy(sub_graph);
        return EXIT_FAILURE;
    }
    printf("%.1f %d\n", path->distance, path->list->nodeCount);
    for (ListIntNode *pnt = path->list->sentinel.next; pnt != &path->list->sentinel; pnt = pnt->next)
    {
        printf("%d ", pnt->value);
    }
    free(fpath);
    free(fcoor);
    ListInt_destroy(list);
    PathMatrix_destroy(pathMatrix);
    Path_destroy(path);
    Graph_destroy(graph);
    Graph_destroy(sub_graph);
    return 0;
}

// A tester dès que l'ACO fonctionne
int Passage_TSPACO()
{
    /*
        #include "Settings.h"
        #include "Graph.h"
        #include "ListInt.h"
        #include "ShortestPath.h"
        #include "TSP.h"
    */

    char *fpath = calloc(100, sizeof(char));
    char *fcoor = calloc(100, sizeof(char));
    int nb_points = 0;
    int point = -1;
    scanf("%s", fpath);
    scanf("%s", fcoor);
    scanf("%d", &nb_points);
    ListInt *list = ListInt_create();
    for (int i = 0; i < nb_points; i++)
    {
        scanf("%d ", &point);
        ListInt_insertLast(list, point);
    }
    Graph *graph = Graph_load(fpath);

    Path *path = Graph_tspFromACO(graph, 0, 1000, 100, 2.0f, 3.0f, 0.1f, 2.0f);

    printf("%.1f %d\n", path->distance, path->list->nodeCount);
    for (ListIntNode *pnt = path->list->sentinel.next; pnt != &path->list->sentinel; pnt = pnt->next)
    {
        printf("%d ", pnt->value);
    }
    free(fpath);
    free(fcoor);
    ListInt_destroy(list);
    Path_destroy(path);
    Graph_destroy(graph);
    return 0;
}