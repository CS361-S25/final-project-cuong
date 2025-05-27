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
    int index = 0;
    std::vector<Cell *> connections = std::vector<Cell *>(8);
    int facing;

public:
    Cell(const MyConfigType &cfg = worldConfig) : config(cfg), random(cfg.SEED()), id(std::to_string(random.GetUInt())), facing(static_cast<int>(random.GetUInt(8)))
    {
    }

    std::string GetID() { return id; }
    int GetIndex() { return index; }
    void SetIndex(int new_idx) { index = new_idx; }
    int GetFacing() { return facing; }
    void SetFacing(int new_facing) { facing = new_facing; }
    void RotateLeft() { facing = emp::Mod(facing - 1, 8); }
    void RotateRight() { facing = emp::Mod(facing + 1, 8); }

    Cell *GetConnection(int dir) { return connections[dir]; }
    void SetConnection(int dir, Cell *new_connection) { connections[dir] = new_connection; }
};

#endif