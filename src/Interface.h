#pragma once

#include "Settings.h"
#include "Graph.h"
#include "ListInt.h"
#include "ShortestPath.h"

/// @brief Permet la création d'un point dans un fichier
/// @param fichier le fichier geojson où il faut ajouter un point
/// @param latitude la latitude du point
/// @param longitude la longitude du point
/// @param name le nom que l'on souhaite donner au point (visible sur la carte)
void Creation_Point(FILE *fichier, float latitude, float longitude, char *name);

/// @brief Permet la création du ligne entre deux points dans un fichier
/// @param fichier le fichier geojson où il faut ajouter une ligne
/// @param latitude_1 la latitude du point de départ
/// @param longitude_1 la longitude du point de départ
/// @param latitude_2 la latitude du point d'arrivée
/// @param longitude_2 la longitude du point d'arrivée
void Creation_LineString(FILE *fichier, float latitude_1, float longitude_1, float latitude_2, float longitude_2);

/// @brief Fonction permettant la création d'un fichier geojson
/// @param path le chemin qu'on veut voir tracé
/// @param size le nombre de noeuds dans le graph
/// @return 0 si ça a fonctionné, 1 sinon
int Creation_geojson(Path *path, int size);