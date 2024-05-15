#pragma once

#include "Settings.h"
#include "Graph.h"
#include "ListInt.h"
#include "ShortestPath.h"

/// @brief Fonction permettant la création d'un fichier geojson
/// @param path le chemin qu'on veut voir tracé
/// @param size le nombre de noeuds dans le graph
/// @return 0 si ça a fonctionné, 1 sinon
int Creation_geojson(Path *path, int size);