#include "Settings.h"
#include "Graph.h"
#include "ListInt.h"
#include "ShortestPath.h"
#include "Interface.h"
#include "TSP.h"
#include "SpatialHashing.h"

#define File "../TPF_Donnees/Tests/5_Grande_instance/input.txt"
#define CsvFile "../TPF_Donnees/Data/communes-departement-region.csv"

int TestFonction(int NbTest)
{
    char PathDebugFile[256];
    snprintf(PathDebugFile, sizeof(PathDebugFile), "../TPF_Donnees/Tests/4_TSP_ACO/input%d.txt", NbTest);

    FILE *file = fopen(PathDebugFile, "r");
    if (file == NULL)
    {
        printf("Error opening the file\n");
        return EXIT_FAILURE;
    }

    char PathOutpoutFile[256];
    snprintf(PathOutpoutFile, sizeof(PathOutpoutFile), "../TPF_Donnees/Tests/4_TSP_ACO/output%d.txt", NbTest);

    FILE *fileOutpout = fopen(PathOutpoutFile, "r");
    if (fileOutpout == NULL)
    {
        printf("Error opening the file\n");
        fclose(file);
        return EXIT_FAILURE;
    }

    char GraphFile[256];
    char CoordFile[256];
    int NbPoints;

    if (fscanf(file, "%s", GraphFile) != 1 || fscanf(file, "%s", CoordFile) != 1 || fscanf(file, "%d", &NbPoints) != 1)
    {
        printf("Error reading from the file\n");
        fclose(file);
        return EXIT_FAILURE;
    }

    Graph *graph = Graph_load(GraphFile);
    if (graph == NULL)
    {
        printf("Error loading the graph\n");
        fclose(file);
        return EXIT_FAILURE;
    }

    ListInt *list = ListInt_create();
    int *tabtoGraph = calloc(NbPoints, sizeof(int));
    PathMatrix *pathMatrix = PathMatrix_create(NbPoints);

    for (int i = 0; i < NbPoints; i++)
    {
        int point;
        if (fscanf(file, "%d", &point) != 1)
        {
            printf("Error reading point from the file\n");
            ListInt_destroy(list);
            PathMatrix_destroy(pathMatrix);
            Graph_destroy(graph);
            fclose(file);
            return EXIT_FAILURE;
        }
        ListInt_insertLast(list, point);
        tabtoGraph[i] = point;
    }
    Graph *graph2 = Graph_getSubGraph(graph, list, pathMatrix);

    fclose(file);

    Path *path2 = Graph_tspFromHeuristic(graph2, 0);
    Path *path3 = Graph_tspFromACO(graph2, 0, 1000, 100, 2.0, 3.0, 0.1, 2.0);

    float TailleOutPut = 0.0;
    fscanf(fileOutpout, "%f", &TailleOutPut);
    printf("Distance prévu ACO : %.1f\n", TailleOutPut);
    printf("Distance avec ACO : %.1f\n", path3->distance);
    printf("Distance avec TSP : %.1f\n", path2->distance);

    int NbdeNode = 0;
    fscanf(fileOutpout, "%d\n", &NbdeNode);
    for (int i = 0; i < NbdeNode; i++)
    {
        int node = 0;
        fscanf(fileOutpout, "%d", &node);
        printf("%d ", node);
    }
    printf("\n");
    fclose(fileOutpout);

    for (ListIntNode *pnt = path3->list->sentinel.next; pnt != &path3->list->sentinel; pnt = pnt->next)
    {
        printf("%d ", pnt->value);
    }
    printf("\n");

    for (ListIntNode *pnt = path2->list->sentinel.next; pnt != &path2->list->sentinel; pnt = pnt->next)
    {
        printf("%d ", pnt->value);
    }
    printf("\n");

    int reussiteGeo = Creation_geojson(path3, CoordFile, tabtoGraph, pathMatrix);
    if (reussiteGeo == EXIT_FAILURE)
    {
        printf("Erreur en créant le fichier geojson\n");
    }

    Path_destroy(path2);
    Graph_destroy(graph);
    ListInt_destroy(list);
    PathMatrix_destroy(pathMatrix);
    Graph_destroy(graph2);
    Path_destroy(path3);
    free(tabtoGraph);

    return EXIT_SUCCESS;
}

int main()
{
    srand((unsigned int)time(NULL));

    for (int i = 1; i < 2; i++)
    {
        clock_t start = clock();
        printf("###### Test %d ######\n", i);
        TestFonction(i);
        clock_t end = clock();
        double time_spent = (double)(end - start) / CLOCKS_PER_SEC;
        printf("Time spent: %f\n", time_spent);
        printf("\n");
    }

    printf("###### Welcome in to the Global ProblemS System.e.s ######\n");
    printf("\t\t Please choose a Code Postal :\n");
    int code_postal = 0;
    scanf("%d", &code_postal);
    Point *point = NearestPointFromCSV(CsvFile, code_postal);
    if (point == NULL)
    {
        printf("Error in the postal code\n");
        return EXIT_FAILURE;
    }
    printf("The nearest point is : %f %f\n", point->latitude, point->longitude);
    int index = CoordToGraphIndex(point, "../TPF_Donnees/Data/france_inter.txt");
    if (index == EXIT_FAILURE)
    {
        printf("Error in the index\n");
        return EXIT_FAILURE;
    }
    printf("\t\t Please choose another Code Postal :\n");
    scanf("%d", &code_postal);
    Point *point2 = NearestPointFromCSV(CsvFile, code_postal);
    if (point2 == NULL)
    {
        printf("Error in the postal code\n");
        return EXIT_FAILURE;
    }
    printf("The nearest point is : %f %f\n", point2->latitude, point2->longitude);
    int index2 = CoordToGraphIndex(point2, "../TPF_Donnees/Data/france_inter.txt");
    if (index2 == EXIT_FAILURE)
    {
        printf("Error in the index\n");
        return EXIT_FAILURE;
    }
    Graph *graph = Graph_load("../TPF_Donnees/Data/france_graph.txt");
    if (graph == NULL)
    {
        printf("Error loading the graph\n");
        return EXIT_FAILURE;
    }
    Path *path = Graph_shortestPath(graph, index, index2);
    if (path == NULL)
    {
        printf("Error in the path\n");
        return EXIT_FAILURE;
    }
    printf("The distance is : %.1f\n", path->distance);
    printf("The path is : ");
    for (ListIntNode *pnt = path->list->sentinel.next; pnt != &path->list->sentinel; pnt = pnt->next)
    {
        printf("%d ", pnt->value);
    }
    printf("\n");

    printf("\n");
    Path_destroy(path);

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
            printf("%d %d %.1f\n", i, j, pathMatrix->matrix[i][j]->distance);
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

/*
test 1 = 10.00/20
test 2 = 10.00/20
test 3 = 10.00/20
test 4 = 10.31/20
*/
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
    PathMatrix *pathMatrix = PathMatrix_create(nb_points);
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

    Path *path = Graph_tspFromACO(sub_graph, 0, 1000, 100, 2.0f, 3.0f, 0.1f, 2.0f);
    if ((path == NULL))
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

    printf("%.1f %d\n", path->distance, path->list->nodeCount);
    for (ListIntNode *pnt = path->list->sentinel.next; pnt != &path->list->sentinel; pnt = pnt->next)
    {
        printf("%d ", pnt->value);
    }
    free(fpath);
    free(fcoor);
    ListInt_destroy(list);
    Path_destroy(path);
    PathMatrix_destroy(pathMatrix);
    Graph_destroy(graph);
    Graph_destroy(sub_graph);
    return 0;
}