#ifndef NEIGHBOR_MANAGER_H
#define NEIGHBOR_MANAGER_H

#include "sensor_utils.h"
#include "neighbor_manager.h"
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_SENSORS 12


typedef struct {
    int coords[2];
    float priority;
} Neighbor;

typedef struct {
    Neighbor neighbors[MAX_SENSORS];
    int neighbor_count;
    int x;
    int y;
    float measurement;
} NeighborManager;

void init_neighbor_manager(NeighborManager* manager, int x, int y, float measurement);
void add_sensor_if_not_exists(NeighborManager* manager, int x, int y);
void remove_neighbor(NeighborManager* manager, int x, int y);
float calculate_priority(NeighborManager* manager, int x, int y);
int is_not_a_neighbor(NeighborManager* manager, int x, int y);
int is_the_same_sensor(NeighborManager* manager, int x, int y);
int is_sensor_not_in_neighbors(NeighborManager* manager, int x, int y);
void print_neighbors(NeighborManager* manager);

#endif