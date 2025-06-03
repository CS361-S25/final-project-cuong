#ifndef ORG_H
#define ORG_H


#include "CPU.h"
#include "OrgState.h"
#include "Cell.h"
#include "emp/Evolve/World_structure.hpp"
#include "ConfigSetup.h"

class Organism {
  CPU cpu;
  const MyConfigType& config;

public:

  Organism(emp::Ptr<OrgWorld> world, const MyConfigType& cfg = worldConfig, double points = 0.0) : cpu(world), config(cfg) {
    SetPoints(points);
  }

  // Basic data processing
  void SetPoints(double _in) { cpu.state.points = _in; }
  void AddPoints(double _in) { cpu.state.points += _in; }
  double GetPoints() { return cpu.state.points; }
  // size_t GetAge() { return cpu.state.age; }
  // size_t GetBestTask() { return cpu.state.best_task; }

  emp::WorldPosition GetLocation(){return cpu.state.current_location;}
 
  void SetCell(Cell* new_cell) {cpu.state.cell = new_cell;}
  Cell* GetCell() {return cpu.state.cell;}

  void SetFacing(int new_facing) {cpu.state.facing = new_facing;}
  
  void SetMessage(unsigned int new_message) {cpu.state.message = new_message;}
  unsigned int GetMessage() {return cpu.state.message;}
  
  void SetInbox(unsigned int new_inbox) {cpu.state.inbox = new_inbox;}
  unsigned int GetInbox() {return cpu.state.inbox;}
  
  void Reset() { cpu.Reset(); }
  void Mutate() { cpu.Mutate(); }

  /**
   * Attempt to produce a child organism, if this organism has enough points.
   */
  std::optional<Organism> CheckReproduction() {
    Organism offspring = *this;
    offspring.Reset();
    offspring.Mutate();
    return offspring;
  }

  /**
   * Input: the current index location of the organism.
   *
   * Output: None
   *
   * Purpose: Saves the information in the CPU, run the CPU for some cycles, and age up the organism.
   */
  void Process(emp::WorldPosition current_location) {
    // std::cout << "Org Process 0" <<std::endl;
    //cpu.state.task_done = false;
    AddPoints(1.0);
    std::cout << cpu.state.points <<std::endl;
    // std::cout << "Org Process 1" <<std::endl;
    cpu.state.current_location = current_location;
    // std::cout << "Org Process 2" <<std::endl;
    // std::cout << "Organism at " << current_location.GetIndex() << " has " << cpu.state.points << " points." <<std::endl;
    // std::cout << "Org Process 3" <<std::endl;
    cpu.RunCPUStep(10);
    // std::cout << "Org Process 4" <<std::endl;
    cpu.state.age++;
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Prints out the genome of the organism.
   */
  void PrintGenome() {
    std::cout << "program ------------" << std::endl;
    cpu.PrintGenome();
    std::cout << "end ---------------" << std::endl;
  }
};

#endif