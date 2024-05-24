#pragma once
#include <time.h>
#include <stdlib.h>
#include <math.h>

#include "Graph.h"
#include "ShortestPath.h"

typedef struct PathMatrix
{
    Path ***matrix;
    int size;
} PathMatrix;

/// @brief Crée une matrice de chemins
/// @param size le nombre de noeuds à mettre dans la liste du chemin
/// @return un PathMatrix correctement initialisé
PathMatrix *PathMatrix_create(int size);

/// @brief Détruit une matrice de chemins.
/// @param matrix la matrice de chemins.
void PathMatrix_destroy(PathMatrix *matrix);

/// @brief Calcule une tounée dans un graphe en se basant sur un algorithme glouton. Ne pas oublier de mettre %.1f quand on imprime la valeur pour qu'elle soit à la bonne taille
/// @param graph le graphe des distances. Il doit être complet.
/// @param station l'identifiant du sommet de départ et d'arrivée de la tournée.
/// @return La tournée.
Path *Graph_tspFromHeuristic(Graph *graph, int station);

/// @brief Calcule une tournée dans un graphe en se basant sur l'algorithme d'optimisation ACO.
/// @param graph le graphe des distances. Il doit être complet.
/// @param station l'identifiant du sommet de départ et d'arrivée de la tournée.
/// @param iterationCount le nombre d'itération de l'algorithme.
/// @param antCount le nombre de fourmis par itération.
/// @param alpha un réel positif qui contrôle l'influence des phéromones sur le choix des fourmis.
/// @param beta un réel supérieur à 1.f qui contrôle l'influence de la distance réelle sur le choix des fourmis.
/// @param rho un réel entre 0.f et 1.f qui contrôle l'évaporation des pheromones
/// @param q un réel positif qui contôle la quantité de phéromones déposées par les fourmis.
/// @return La tournée.
Path *Graph_tspFromACO(
    Graph *graph, int station, int iterationCount, int antCount,
    float alpha, float beta, float rho, float q);

/// @brief Depuis un sommet, calcule les probabilités de passer aux sommets suivants.
/// @param graph le graphe des distances. Il doit être complet.
/// @param pheromones le graphe des phéromones.
/// @param station l'identifiant du sommet actuel de la fourmi.
/// @param explored tableau indiquant les sommets déjà parcourus par la fourmi.
/// @param alpha un réel positif qui contrôle l'influence des phéromones sur le choix des fourmis.
/// @param beta un réel supérieur à 1.f qui contrôle l'influence de la distance réelle sur le choix des fourmis.
/// @return Le tableau aloué dynamiquement des probabilités de la fourmi.
float *Graph_acoGetProbabilities(
    Graph *graph, Graph *pheromones, int station,
    bool *explored, float alpha, float beta);

/// @brief Construit la tournée d'une fourmi.
/// @param graph le graphe des distances. Il doit être complet.
/// @param pheromones le graphe des phéromones.
/// @param station l'identifiant du sommet de départ et d'arrivée de la tournée.
/// @param alpha un réel positif qui contrôle l'influence des phéromones sur le choix de la fourmi.
/// @param beta un réel supérieur à 1.f qui contrôle l'influence de la distance réelle sur le choix de la fourmi.
/// @return La tournée de la fourmi.
Path *Graph_acoConstructPath(Graph *graph, Graph *pheromones, int station, float alpha, float beta);

/// @brief Ajoute des phéromones en prenant en compte la tournée d'une fourmi et sa qualité.
/// @param pheromones le graphe des phéromones.
/// @param path la tournée de la fourmi.
/// @param q un réel positif qui contôle la quantité de phéromones déposées par la fourmi.
void Graph_acoPheromoneUpdatePath(Graph *const pheromones, Path *const path, float q);

/// @brief Applique l'évaporation des phéromones à la fin d'une itération de l'algorithme ACO.
/// @param pheromones le graphe des phéromones.
/// @param rho le coefficient d'évaporation des phéromones (entre 0.f et 1.f).
void Graph_acoPheromoneGlobalUpdate(Graph *const pheromones, float rho);

/// @brief Obtenir le sous-graphe de graph
/// @param graph le graph dont on ne veut garder que les points d'intérêts
/// @param list la liste des points à parcourir
/// @param pathMatrix une matrice des chemins
/// @return le sous-graph
Graph *Graph_getSubGraph(Graph *graph, ListInt *list, PathMatrix *pathMatrix);

// @brief Crée un chemin inverse
// @param path le chemin à inverser
// @return le chemin inversé
Path *InversePath(Path *path);

/// @brief Calcule la distance d'un chemin sur un graph donné
/// @param path le chemin dont on cherche la distance
/// @param graph le graph qui donne le poids des arcs
/// @return le chemin avec une distance correctement allouée
Path *Graph_VerifyPathDistance(Path *path, Graph *graph);

/*
-----------------SNIF SNIF------------------------------------------------------------------------------------------
/// @brief l'appel récursif afin de de calculer la tournée parfaite
/// @param nb_node le nombre de node
/// @param tournee le chemin que l'on teste
/// @param graph le graph avec les nodes
/// @param best_tournee le meilleur chemin actullement trouvé
/// @return Renvoie le path afin de tester s'il est optimal
Path *Graph_tspFromBrutForce_Recursive(int nb_node, Path *tournee, Graph *graph, Path *best_tournee, float dist_min);

/// @brief Calcule la tournée parfaite quand le nombre de noeuds est inférieur ou égale à 8.
/// @param graph le graph des distances. Il doit être complet.
/// @param station l'identifiant du sommet de départ et d'arrivée de la tournée.
/// @return La tournée.
Path *Graph_tspFromBrutForce(Graph *graph);
*/