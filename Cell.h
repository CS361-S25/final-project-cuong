#ifndef CELL_H
#define CELL_H

#include <string>
#include "emp/math/Random.hpp"
#include "emp/Evolve/World_structure.hpp"
#include "ConfigSetup.h"
#include "Org.h"
#include "World.h"

class Cell
{
    const MyConfigType &config;
    emp::Random random{worldConfig.SEED()};
    emp::Ptr<OrgWorld> world;
    std::string id = std::to_string(random.GetUInt());
    int index = 0;
    std::vector<Cell*> connections = std::vector<Cell*>(8);
    int facing = static_cast<int>( random.GetUInt(8) );;
    Organism* occupant;

public:
    Cell(const MyConfigType &cfg = worldConfig) : config(cfg)
    {
        ;
    }
    
    std::string GetID(){return id;}
    int GetIndex() {return index;}
    void SetIndex(int new_idx) {index = new_idx;}
    int GetFacing() {return facing;}
    void SetFacing(int new_facing) {facing = new_facing;}
    void TurnLeft() {
        facing = facing - 1;
        if (facing <= 0) {facing = 7};
    }
    void TurnRight(){
        facing = facing + 1;
        if (facing >= 8) {facing = 0};
    }
    Cell* GetConnection(int dir){return connections[dir];}
    void SetConnection(int dir, Cell* new_connection){connections[dir] = new_connection;}





};

#endif