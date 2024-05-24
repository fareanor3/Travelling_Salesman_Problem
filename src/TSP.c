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

Path *Graph_VerifyPathDistance(Path *path, Graph *graph)
{
    if ((path == NULL) || (graph == NULL) || (path->list == NULL))
    {
        printf("Le path ou le graph ou la list du path n'existe pas\n");
        return NULL;
    }
    ListIntIter *iter = ListIntIter_create(path->list);
    float ecart = 0.f;
    for (int i = 0; i < path->list->nodeCount - 1; i++)
    {
        ArcList *arclist = Graph_getArcList(graph, iter->current->value);
        if (iter->current->next->value == iter->current->value)
        {
            printf("DID I FOUND THE PROBLEM ?\n");
            return NULL;
        }
        while (arclist->target != iter->current->next->value)
            arclist = arclist->next;
        ecart += arclist->weight;
        ListIntIter_next(iter);
    }
    path->distance = ecart;
    ListIntIter_destroy(iter);
    return path;
}
/*
-------------------------TENTATIVE 2---------------------------------------------------------------------------
Path *Graph_tspFromBrutForce_Recursive(int nb_node, Path *tournee, Graph *graph, Path *best_tournee, float dist_min)
{

    if (nb_node == 1)
        return best_tournee;
    else
    {
        // printf("NOIRS\n");
        for (int i = 0; i < nb_node; i++)
        {
            // On vérifie si on a trouvé une meilleure tournée
            if (tournee->distance > dist_min)
            {
                // printf("QUI VIENNENT\n");
                Graph_tspFromBrutForce_Recursive(nb_node - 1, tournee, graph, best_tournee, dist_min);
                // Si c'est un nombre impair
                if (nb_node % 2 == 1)
                {
                    // printf("DU NORD\n");
                    ListInt_permute(tournee->list, 0, nb_node - 1);
                    // printf("COLORENT LA TERRE\n");
                    Graph_VerifyPathDistance(tournee, graph);
                    // printf("LES LACS LES RIVIERES\n");
                }
                else
                {
                    ListInt_permute(tournee->list, i, nb_node - 1);
                    Graph_VerifyPathDistance(tournee, graph);
                }
            }
            else
            {
                printf("C'EST LE DECOR\n");
                best_tournee = tournee;
                dist_min = best_tournee->distance;
                Graph_tspFromBrutForce_Recursive(nb_node - 1, tournee, graph, best_tournee, dist_min);
                // Si c'est un nombre impair
                if (nb_node % 2 == 1)
                {
                    ListInt_permute(tournee->list, 0, nb_node - 1);
                    Graph_VerifyPathDistance(tournee, graph);
                }
                else
                {
                    ListInt_permute(tournee->list, i, nb_node - 1);
                    Graph_VerifyPathDistance(tournee, graph);
                }
            }
        }
        printf("Mes couilles\n");
        return tournee;
    }
}

Path *Graph_tspFromBrutForce(Graph *graph)
{
    // Vérification d'usage
    if (graph == NULL)
    {
        printf("Problème avec le graph\n");
        return NULL;
    }
    // Avec Heap pour faire les permutations

    Path *tournee = Path_create(0);
    Path *best_tournee = Path_create(0);
    if ((best_tournee == NULL) || (tournee == NULL))
    {
        printf("Problème avec la création des chemins\n");
        return NULL;
    }

    float dist_min = INFINITY;

    for (int i = 1; i < graph->size; i++)
    {
        ListInt_insertLast(tournee->list, i);

        ListInt_insertLast(best_tournee->list, i);
    }
    ListInt_insertLast(tournee->list, 0);
    ListInt_insertLast(best_tournee->list, 0);
    printf("DES\n");
    Graph_VerifyPathDistance(tournee, graph);
    Graph_VerifyPathDistance(best_tournee, graph);
    printf("NUAGES\n");
    printf("%d\n", graph->size);
    best_tournee = Graph_tspFromBrutForce_Recursive(graph->size, tournee, graph, best_tournee, dist_min);
    printf("DU CONNEMARA\n");
    if (best_tournee == NULL)
    {
        printf("Ze crois que z'ai trouvé le problème\n");
        return NULL;
    }
    Path_destroy(tournee);
    return best_tournee;
}


-------------------------TENTATIVE 1---------------------------------------------------------------------
// Non fonctionnel (tourne en boucle)

Path *Graph_tspFromBrutForce(Graph *graph)
{
    // Vérification d'usage
    if (graph == NULL)
    {
        printf("Problème avec le graph\n");
        return NULL;
    }

    Path *best_tournee = Path_create(0);

    if ((best_tournee == NULL))
    {
        printf("Problème avec la création des chemins\n");
        return NULL;
    }

    float distance_min = INFINITY;
    int factor = 1;
    int size = graph->size;

    // On calcule combien de chemin on est sensé avoir par point de départ
    for (int i = size; i > 1; i--)
        factor *= i;
    // printf("%d\n", factor);
    for (int i = 0; i < graph->size; i++)
    {
        Path *tournee = Path_create(i);
        ListInt_insertFirst(tournee->list, i);
        bool first2 = true, first3 = true, first4 = true, first5 = true, first6 = true, first7 = true;
        int tmp = 0;

        for (int j = 0; j < graph->size; j++)
        {
            if (j == i)
                continue;

            if (first2)
            {
                ListIntNode *node2 = calloc(1, sizeof(ListIntNode));
                node2->next = NULL;
                node2->prev = NULL;
                node2->value = j;
                ListInt_insertNodeAfter(tournee->list, tournee->list->sentinel.next, node2);
                tournee->distance += *Graph_getArc(graph, i, j);
            }
            else
            {
                while (tournee->list->nodeCount > 2)
                    tmp = ListInt_popLast(tournee->list);
                ListIntNode *node2 = calloc(1, sizeof(ListIntNode));
                node2->next = NULL;
                node2->prev = NULL;
                node2->value = j;
                ListInt_insertNodeAfter(tournee->list, tournee->list->sentinel.next, node2);
                tournee->distance += *Graph_getArc(graph, i, j);
            }

            for (int k = 0; k < graph->size; k++)
            {
                if (graph->size < 3)
                    break;
                if ((k == j) || (k == i))
                    continue;
                if (first3)
                {
                    ListIntNode *node3 = calloc(1, sizeof(ListIntNode));
                    node3->next = NULL;
                    node3->prev = NULL;
                    node3->value = j;
                    ListInt_insertNodeAfter(tournee->list, tournee->list->sentinel.next->next, node3);
                    tournee->distance += *Graph_getArc(graph, j, k);
                }
                else
                {
                    while (tournee->list->nodeCount > 3)
                        tmp = ListInt_popLast(tournee->list);
                    ListIntNode *node3 = calloc(1, sizeof(ListIntNode));
                    node3->next = NULL;
                    node3->prev = NULL;
                    node3->value = j;
                    ListInt_insertNodeAfter(tournee->list, tournee->list->sentinel.next->next, node3);
                    tournee->distance += *Graph_getArc(graph, j, k);
                }

                for (int l = 0; l < graph->size; k++)
                {
                    if (graph->size < 4)
                        break;
                    if ((l == j) || (l == i) || (l == k))
                        continue;
                    if (first4)
                    {
                        ListIntNode *node4 = calloc(1, sizeof(ListIntNode));
                        node4->next = NULL;
                        node4->prev = NULL;
                        node4->value = j;
                        ListInt_insertNodeAfter(tournee->list, tournee->list->sentinel.next->next->next, node4);
                        tournee->distance += *Graph_getArc(graph, k, l);
                    }
                    else
                    {
                        while (tournee->list->nodeCount > 4)
                            tmp = ListInt_popLast(tournee->list);
                        ListIntNode *node4 = calloc(1, sizeof(ListIntNode));
                        node4->next = NULL;
                        node4->prev = NULL;
                        node4->value = j;
                        ListInt_insertNodeAfter(tournee->list, tournee->list->sentinel.next->next->next, node4);
                        tournee->distance += *Graph_getArc(graph, k, l);
                    }

                    for (int m = 0; m < graph->size; m++)
                    {
                        if (graph->size < 5)
                            break;
                        if ((m == j) || (m == i) || (m == k) || (m == l))
                            continue;
                        if (first5)
                        {
                            ListIntNode *node5 = calloc(1, sizeof(ListIntNode));
                            node5->next = NULL;
                            node5->prev = NULL;
                            node5->value = j;
                            ListInt_insertNodeAfter(tournee->list, tournee->list->sentinel.next->next->next->next, node5);
                            tournee->distance += *Graph_getArc(graph, l, m);
                        }
                        else
                        {
                            while (tournee->list->nodeCount > 5)
                                tmp = ListInt_popLast(tournee->list);
                            ListIntNode *node5 = calloc(1, sizeof(ListIntNode));
                            node5->next = NULL;
                            node5->prev = NULL;
                            node5->value = j;
                            ListInt_insertNodeAfter(tournee->list, tournee->list->sentinel.next->next->next->next, node5);
                            tournee->distance += *Graph_getArc(graph, l, m);
                        }

                        for (int n = 0; n < graph->size; n++)
                        {
                            if (graph->size < 6)
                                break;
                            if ((n == j) || (n == i) || (n == k) || (n == l) || (n == m))
                                continue;
                            if (first6)
                            {
                                ListIntNode *node6 = calloc(1, sizeof(ListIntNode));
                                node6->next = NULL;
                                node6->prev = NULL;
                                node6->value = j;
                                ListInt_insertNodeAfter(tournee->list, tournee->list->sentinel.next->next->next->next->next, node6);
                                tournee->distance += *Graph_getArc(graph, m, n);
                            }
                            else
                            {
                                while (tournee->list->nodeCount > 6)
                                    tmp = ListInt_popLast(tournee->list);
                                ListIntNode *node6 = calloc(1, sizeof(ListIntNode));
                                node6->next = NULL;
                                node6->prev = NULL;
                                node6->value = j;
                                ListInt_insertNodeAfter(tournee->list, tournee->list->sentinel.next->next->next->next->next, node6);
                                tournee->distance += *Graph_getArc(graph, m, n);
                            }

                            for (int o = 0; o < graph->size; o++)
                            {
                                if (graph->size < 7)
                                    break;
                                if ((o == j) || (o == i) || (o == k) || (o == l) || (o == m))
                                    continue;
                                if (first7)
                                {
                                    ListIntNode *node7 = calloc(1, sizeof(ListIntNode));
                                    node7->next = NULL;
                                    node7->prev = NULL;
                                    node7->value = j;
                                    ListInt_insertNodeAfter(tournee->list, tournee->list->sentinel.next->next->next->next->next->next, node7);
                                    tournee->distance += *Graph_getArc(graph, n, o);
                                }
                                else
                                {
                                    while (tournee->list->nodeCount > 7)
                                        tmp = ListInt_popLast(tournee->list);
                                    ListIntNode *node7 = calloc(1, sizeof(ListIntNode));
                                    node7->next = NULL;
                                    node7->prev = NULL;
                                    node7->value = j;
                                    ListInt_insertNodeAfter(tournee->list, tournee->list->sentinel.next->next->next->next->next->next, node7);
                                    tournee->distance += *Graph_getArc(graph, n, o);
                                }
                            }
                        }
                    }
                }
            }
        }
        ListInt_insertLast(tournee->list, i);
        tournee->distance += *Graph_getArc(graph, tournee->list->sentinel.prev->prev->value, i);
        // On vérifie si la distance de la tournée actuelle est inférieur à celle qu'on a gardé en mémoire (en vérifiant que la distance n'est pas égale à 0)
        if ((tournee->distance < distance_min) && (tournee->distance != 0.0f))
        {
            best_tournee = tournee;
            distance_min = best_tournee->distance;
        }
        Path_destroy(tournee);
    }

    // On renvoit la meilleure tournée (celle avec la distance la plus courte)
    return best_tournee;
}
*/
