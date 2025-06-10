#ifndef CELL_H
#define CELL_H

#include <string>
#include "emp/math/Random.hpp"
#include "emp/math/math.hpp"
#include "emp/Evolve/World_structure.hpp"
#include "ConfigSetup.h"
#include "World.h"

class Cell
{
    const MyConfigType &config;
    emp::Random random;
    unsigned int id;
    int linear_index;
    std::vector<Cell *> connections;
    int facing;
    bool has_org;

public:
    Cell(const MyConfigType &cfg = worldConfig) : 
    config(cfg), random(cfg.SEED()), 
    id(0), 
    linear_index(0),
    connections(std::vector<Cell *>(8)),
    has_org(false),
    facing(0)
    {
        ;
    }

    void SetID(unsigned int new_id) {id = new_id;}
    unsigned int GetID() { return id; }
    int GetIndex() { 
        return linear_index; 
    }
    void SetIndex(int new_idx) { linear_index = new_idx; }
    int GetFacing() { return facing; }
    void SetFacing(int new_facing) { facing = new_facing; }
    void RotateLeft() { facing = emp::Mod(facing - 1, 8); }
    void RotateRight() { facing = emp::Mod(facing + 1, 8); }

    Cell *GetConnection(int dir) { 
        // std::cout << "GetConnection 0" <<std::endl;
        // std::cout << "dir: " << dir <<std::endl;
        // std::cout << "connections[dir]: " << connections[dir]->GetIndex() <<std::endl;
        return connections[dir]; 
    }
    void SetConnection(int dir, Cell *new_connection) { connections[dir] = new_connection; }
    Cell *GetFacingCell() {return GetConnection(GetFacing());}

    bool GetHasOrg(){return has_org;}
    void SetHasOrg(bool state){has_org = state;}
};

#endif