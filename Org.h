#ifndef ORG_H
#define ORG_H

#include <cmath>
#include "CPU.h"
#include "OrgState.h"
#include "Cell.h"
#include "emp/Evolve/World_structure.hpp"
#include "ConfigSetup.h"

class Organism {
  CPU cpu;
  const MyConfigType& config;

public:

  Organism(emp::Ptr<OrgWorld> world, const MyConfigType& cfg = worldConfig, double points = 30.0) : cpu(world), config(cfg) {
    SetPoints(points);
  }

  // Local variables data processing
  void SetPoints(double _in) { cpu.state.points = _in; }
  void AddPoints(double _in) { cpu.state.points += _in; }
  double GetPoints() { return cpu.state.points; }
  // size_t GetAge() { return cpu.state.age; }
  size_t GetBestTask() { return cpu.state.best_task; }

  emp::WorldPosition GetLocation(){return cpu.state.current_location;}


  void AddReproduced(int inc){cpu.state.reproduced += inc;}
  void SetReproduced(int new_rep){cpu.state.reproduced = new_rep;}
  int GetReproduced(){return cpu.state.reproduced;}
 
  void SetCell(Cell* new_cell) {cpu.state.cell = new_cell;}
  Cell* GetCell() {return cpu.state.cell;}

  void SetFacing(int new_facing) {cpu.state.facing = new_facing;}
  
  void SetMessage(unsigned int new_message) {cpu.state.message = new_message;}
  unsigned int GetMessage() {return cpu.state.message;}
  
  void SetInbox(unsigned int new_inbox) {cpu.state.inbox = new_inbox;}
  unsigned int GetInbox() {return cpu.state.inbox;}

  void SetRetrieved(unsigned int new_retrieved) {cpu.state.retrieved = new_retrieved;}
  unsigned int GetRetrieved() {return cpu.state.retrieved;}

  void AddRetrievedValue(unsigned int new_retrieved_value) {cpu.state.retrieved_values.insert(new_retrieved_value);}
  void SetRetrievedValues(std::unordered_set<unsigned int> new_retrieved_values) {cpu.state.retrieved_values = new_retrieved_values;}
  std::unordered_set<unsigned int> GetRetrievedValues() {return cpu.state.retrieved_values;}
  
  void SetMaxKnown(unsigned int new_max_known) {cpu.state.max_known = new_max_known;}
  unsigned int GetMaxKnown() {return cpu.state.max_known;}

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
   * Purpose: Add initial points boost, saves the information in the CPU, run the CPU for some cycles, and age up the organism.
   */
  void Process(emp::WorldPosition current_location) {
    if (GetReproduced() < 2) {AddPoints(1.0);}
    cpu.state.current_location = current_location;
    Cell* cur_cell = cpu.state.cell;
    cpu.RunCPUStep(10);
    cpu.state.age++;
    double penalty = std::log10( static_cast<double>(cpu.state.age) + 1.0 ) - 1;
    // Uncomment for penalty expansion
    // AddPoints( -penalty );
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