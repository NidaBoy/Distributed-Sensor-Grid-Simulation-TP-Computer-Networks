#include "neighbor_manager.h"

#define MAX_NEIGHBORS 3


int find_neighbor_index(NeighborManager* manager, int x, int y) {
    for (int i = 0; i < manager->neighbor_count; ++i) {
        if (manager->neighbors[i].coords[0] == x && manager->neighbors[i].coords[1] == y) {
            return i;
        }
    }
    return -1;
}

float calculate_priority(NeighborManager* manager, int x, int y) {
    return calculate_distance(manager->x, manager->y, x, y);
}

void sort_neighbors_by_priority(NeighborManager* manager) {
    for (int i = 0; i < manager->neighbor_count - 1; ++i) {
        for (int j = 0; j < manager->neighbor_count - i - 1; ++j) {
            if (manager->neighbors[j].priority > manager->neighbors[j + 1].priority) {
                Neighbor temp = manager->neighbors[j];
                manager->neighbors[j] = manager->neighbors[j + 1];
                manager->neighbors[j + 1] = temp;
            }
        }
    }
}

void init_neighbor_manager(NeighborManager* manager, int x, int y, float measurement) {
    manager->neighbor_count = 0;
    manager->x = x;
    manager->y = y;
    manager->measurement = measurement;
}

void add_sensor_if_not_exists(NeighborManager* manager, int x, int y) {

    int index = find_neighbor_index(manager, x, y);
    if (index == -1) {
        if (manager->neighbor_count < MAX_SENSORS) {
            manager->neighbors[manager->neighbor_count].coords[0] = x;
            manager->neighbors[manager->neighbor_count].coords[1] = y;
            manager->neighbors[manager->neighbor_count].priority = calculate_priority(manager, x, y);
            manager->neighbor_count++;
        }
    }
    sort_neighbors_by_priority(manager);
}

void remove_neighbor(NeighborManager* manager, int x, int y) {
    int index = find_neighbor_index(manager, x, y);
    if (index != -1) {
        for (int i = index; i < manager->neighbor_count - 1; ++i) {
            manager->neighbors[i] = manager->neighbors[i + 1];
        }
        manager->neighbor_count--;
    }
}

int is_not_a_neighbor(NeighborManager* manager, int x, int y){
    int neighbors = manager->neighbor_count < 3 ? manager->neighbor_count : 3;
    
    for (int i = 0; i < neighbors; i++){
        if (manager->neighbors[i].coords[0] == x && manager->neighbors[i].coords[1] == y){
            return 0;
        }
    }
    return 1;
}

int is_the_same_sensor(NeighborManager* manager, int x, int y){
    if(manager->x == x && manager->y == y){
        return 1;
    }
    return 0;
}

int is_sensor_not_in_neighbors(NeighborManager* manager, int x, int y){
    if(find_neighbor_index(manager, x, y) == -1){
        return 1;
    }
    return 0;
}

void print_neighbors(NeighborManager* manager) {
    printf("Current Neighbors:\n");
    for (int i = 0; i < manager->neighbor_count; ++i) {
        printf("Coords: (%d, %d), Priority: %f\n", manager->neighbors[i].coords[0], manager->neighbors[i].coords[1], manager->neighbors[i].priority);
    }
}