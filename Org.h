#ifndef ORG_H
#define ORG_H

#include <string>
#include "CPU.h"
#include "OrgState.h"
#include "emp/Evolve/World_structure.hpp"
#include "ConfigSetup.h"

class Organism {
  CPU cpu;
  const MyConfigType& config;
  std::string message;
  std::string inbox;

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
    //cpu.state.task_done = false;
    cpu.state.current_location = current_location;
    cpu.RunCPUStep(10);
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