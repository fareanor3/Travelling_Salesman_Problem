#include "TSP.h"

PathMatrix *PathMatrix_create(int size)
{
    PathMatrix *matrix = (PathMatrix *)calloc(1, sizeof(PathMatrix));
    AssertNew(matrix);
    matrix->size = size;
    matrix->matrix = (Path ***)calloc(size, sizeof(Path **));
    for (int i = 0; i < size; i++)
    {
        matrix->matrix[i] = (Path **)calloc(size, sizeof(Path *));
    }
    return matrix;
}

void PathMatrix_destroy(PathMatrix *matrix)
{
    if (matrix == NULL)
        return;

    for (int i = 0; i < matrix->size; i++)
    {
        for (int j = 0; j < matrix->size; j++)
        {
            if (matrix->matrix[i][j] != NULL)
            {
                Path_destroy(matrix->matrix[i][j]);
            }
        }
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
        printf("Problème avec la station ou le graphe\n");
        return NULL;
    }
    int prev = station;

    // Create a path
    Path *tournee = Path_create(station);
    if (tournee == NULL)
    {
        printf("Problème avec la création du path\n");
        return NULL;
    }

    // Create an array to track if a node has been visited
    bool *passage = (bool *)calloc(size, sizeof(bool));
    if (passage == NULL)
    {
        printf("Problème avec l'allocation du tableau passage\n");
        Path_destroy(tournee);
        return NULL;
    }

    // Iterate until all nodes have been visited
    while (tournee->list->nodeCount < size)
    {
        passage[prev] = true;
        int follower = -1;
        float weight = INFINITY;

        // On récupère la liste des noeuds accessibles par le point auquel on est
        ArcList *arclist = Graph_getArcList(graph, prev);
        if (arclist == NULL)
        {
            printf("Problème avec la liste des arcs\n");
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
            printf("Aucun point suivant trouvé, il y a peut-être un problème dans le graph\n");
            free(passage);
            Path_destroy(tournee);
            return NULL;
        }

        // Add the closest node to the path
        ListInt_insertLast(tournee->list, follower);

        // On ajoute la distance
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

    // on parcour tous les points de la liste
    for (int i = 0; i < size; i++)
    {
        // on récupère l'id du point
        for (int j = i + 1; j < size; j++)
        {
            // sinon on récupère le chemin le plus court entre les deux points
            // et on met la distance dans la matrice
            {
                int id1 = ListInt_get(list, i);
                int id2 = ListInt_get(list, j);
                Path *path = Graph_shortestPath(graph, id1, id2);
                Path *inversePath = InversePath(path);
                pathMatrix->matrix[i][j] = path;
                pathMatrix->matrix[j][i] = inversePath;
                if (path->distance != inversePath->distance)
                    printf("Problème avec les chemins");
                Graph_setArc(subGraph, i, j, path->distance);
                Graph_setArc(subGraph, j, i, inversePath->distance);
            }
        }
    }
    return subGraph;
}

float *Graph_acoGetProbabilities(Graph *graph, Graph *pheromones, int station, bool *explored, float alpha, float beta)
{
    assert(graph && pheromones && explored);

    // On récupère la taille du graphe, on alloue un tableau de probabilités, on initialise la somme à 0
    const int size = Graph_size(graph);
    float *probabilities = (float *)calloc(size, sizeof(float));
    float sum = 0.f;

    // On parcourt tous les points du graphe
    for (int i = 0; i < size; i++)
    {
        // Si le point n'a pas été exploré, on calcule la probabilité de passer par ce point et on l'ajoute à la somme
        if (!explored[i])
        {
            probabilities[i] = powf(*Graph_getArc(pheromones, station, i), alpha) * powf(1.f / *Graph_getArc(graph, station, i), beta);
            sum += probabilities[i];
        }
    }
    // On normalise les probabilités
    for (int i = 0; i < size; i++)
    {
        probabilities[i] /= sum;
        if (probabilities[i] < 1e-20f) // On évite les erreurs d'arrondi
        {
            probabilities[i] = 0.f;
        }
    }
    // On renvoit le tableau de probabilités
    return probabilities;
}

Path *Graph_acoConstructPath(Graph *graph, Graph *pheromones, int station, float alpha, float beta)
{
    assert(graph && pheromones);

    // On récupère la taille du graphe, on crée un path, on alloue un tableau de booléens pour savoir si un point a été exploré
    int size = Graph_size(graph);
    Path *path = Path_create(station);
    bool *explored = (bool *)calloc(size, sizeof(bool));
    // On initialise le point précédent à la station de départ
    int prev = station;

    // On construit le path
    while (path->list->nodeCount < size)
    {
        // On marque le point précédent comme exploré
        explored[prev] = true;
        // On récupère les probabilités de passer par un point
        float *probabilities = Graph_acoGetProbabilities(graph, pheromones, prev, explored, alpha, beta);

        // On tire un nombre aléatoire entre 0 et 1 sur windows et linux
#ifdef _MSC_VER
        const float random = ((float)(rand() << 16) + (float)(rand() & 0xFFFFF)) / (float)RAND_MAX;
#else
        const float random = (float)rand() / (float)RAND_MAX;
#endif
        // On initialise la somme à 0 et le point suivant à -1
        float sum = 0.f;
        int next = -1;

        // On parcourt tous les points du graphe
        for (int i = 0; i < size; i++)
        {
            // Si le point n'a pas été exploré, on ajoute la probabilité de passer par ce point à la somme
            if (!explored[i])
            {
                sum += probabilities[i];
                // Si la somme est supérieure au nombre aléatoire, on a trouvé le point suivant
                if (random <= sum)
                {
                    next = i;
                    // donc on sort de la boucle
                    break;
                }
            }
        }

        // Si on n'a pas trouvé de point suivant, on sort de la boucle
        // sinon on ajoute le point suivant au path et on ajoute la distance
        ListInt_insertLast(path->list, next); //
        path->distance += *Graph_getArc(graph, prev, next);
        prev = next;         // on met à jour le point précédent
        free(probabilities); // on libère la mémoire
    }
    // on ajoute la node de début à la fin afin de rentrer
    ListInt_insertLast(path->list, station);
    path->distance += *Graph_getArc(graph, prev, station);

    // On libère la mémoire et on renvoit le path
    free(explored);
    return path;
}

void Graph_acoPheromoneUpdatePath(Graph *const pheromones, Path *const path, float q)
{
    // On vérifie que les paramètres sont corrects
    assert(pheromones && path);
    // On récupère la quantité de phéromones déposée par la fourmi
    const float w = q / path->distance;

    // On parcourt tous les points du path et on ajoute la quantité de phéromones déposée
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
    // On récupère le coefficient d'évaporation des phéromones
    const float c = 1.f - rho;

    // On parcourt tous les points du graphe et on multiplie la quantité de phéromones par le coefficient d'évaporation
    const int size = Graph_size(pheromones);
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            *Graph_getArc(pheromones, i, j) *= c;
        }
    }
}

Path *Graph_tspFromACO(Graph *graph, int station, int iterationCount, int antCount, float alpha, float beta, float rho, float q)
{
    assert(graph);

    // On crée un graphe de phéromones
    const int size = Graph_size(graph);
    // On crée un graphe de phéromones
    Graph *pheromones = Graph_create(size);

    // initialisation des arc de phéromones : faire un glouton plus tard
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            Graph_setArc(pheromones, i, j, 1);
        }
    }

    // On initialise le meilleur path à NULL et la meilleure distance à l'infini
    Path *bestPath = NULL;
    float bestDistance = INFINITY;

    // On lance l'algorithme ACO, on crée un path, on met à jour les phéromones et on met à jour le meilleur path
    for (int i = 0; i < iterationCount; i++)
    {
        // les fourmis se déplacent
        for (int j = 0; j < antCount; j++)
        {
            // On crée un path, on met à jour les phéromones et on met à jour le meilleur path
            Path *path = Graph_acoConstructPath(graph, pheromones, station, alpha, beta);
            Graph_acoPheromoneUpdatePath(pheromones, path, q);

            // On met à jour le meilleur path
            if (path->distance < bestDistance)
            {
                bestDistance = path->distance;
                // si le meilleur path n'est pas NULL, on le détruit
                if (bestPath != NULL)
                {
                    Path_destroy(bestPath);
                }
                bestPath = path;
            }
            // sinon on le détruit
            else
            {
                Path_destroy(path);
            }
        }
        // On met à jour les phéromones à la fin de chaque itération
        Graph_acoPheromoneGlobalUpdate(pheromones, rho);
    }

    Graph_destroy(pheromones);
    return bestPath;
}

Path *InversePath(Path *path)
{
    Path *inversePath = Path_create(path->list->sentinel.next->value);
    ListIntIter *iter = ListIntIter_create(path->list);
    for (; ListIntIter_isValid(iter) && iter->current->next != iter->sentinel; ListIntIter_next(iter))
    {
        ListInt_insertFirst(inversePath->list, iter->current->next->value);
    }
    inversePath->distance = path->distance;
    ListIntIter_destroy(iter);
    return inversePath;
}