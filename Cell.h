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
    std::string id;
    int index;
    std::vector<Cell *> connections
    int facing;

public:
    Cell(const MyConfigType &cfg = worldConfig) : 
    config(cfg), random(cfg.SEED()), 
    id(std::to_string(random.GetUInt())), 
    index(0),
    connections(std::vector<Cell *>(8)),
    facing(static_cast<int>(random.GetUInt(8)))
    {
    }

    std::string GetID() { return id; }
    int GetIndex() { 
        std::cout << "Fetching cell linear index" <<std::endl;
        return index; 
    }
    void SetIndex(int new_idx) { index = new_idx; }
    int GetFacing() { return facing; }
    void SetFacing(int new_facing) { facing = new_facing; }
    void RotateLeft() { facing = emp::Mod(facing - 1, 8); }
    void RotateRight() { facing = emp::Mod(facing + 1, 8); }

    Cell *GetConnection(int dir) { 
        std::cout << "GetConnection 0" <<std::endl;
        std::cout << "dir: " << dir <<std::endl;
        std::cout << "connections[dir]: " << connections[dir]->GetIndex() <<std::endl;
        return connections[dir]; 
    }
    void SetConnection(int dir, Cell *new_connection) { connections[dir] = new_connection; }
};

#endif