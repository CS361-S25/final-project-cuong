#ifndef WORLD_H
#define WORLD_H

#include "emp/Evolve/World.hpp"
#include "emp/data/DataFile.hpp"
#include <vector>
#include "Org.h"
#include "Task.h"
#include "ConfigSetup.h"

class OrgWorld : public emp::World<Organism>
{
  emp::vector<emp::WorldPosition> reproduce_queue;
  std::vector<Task *> tasks;
  std::vector<emp::Ptr<emp::DataMonitor<int>>> solve_monitors;
  std::vector<int> solve_counts;

public:

 /**
  * Input: A random number generator
  * 
  * Output: None
  * 
  * Purpose: Constructor for the world. Sets up the tasks and monitors.
  */
  OrgWorld(emp::Random &_random) : emp::World<Organism>(_random) {
    AddTask(new Initial());
    AddTask(new ComputeEqual());
    AddTask(new ComputeIncrement());
    // AddTask(new ComputeDecrement());
    AddTask(new ComputeDouble());
    // AddTask(new ComputeHalf());
    AddTask(new ComputeSquare());
    // AddTask(new ComputeSquareRoot());
    AddTask(new ComputeLog());
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Destructor for the world.
   */
  ~OrgWorld() {}

  /**
   * Input: None
   *
   * Output: The size of the world
   *
   * Purposes: Returns various local variables.
   */
  const pop_t &GetPopulation() { return pop; }
  auto GetTasks() { return tasks; }
  auto GetSolveMonitors() { return solve_monitors; }

  /**
   * Input: A task pointer
   *
   * Output: None
   *
   * Purpose: Add a task to the world.
   */
  void AddTask(Task * task) {
    tasks.push_back(task);
  
    const size_t idx = tasks.size() - 1;
    solve_counts.resize(tasks.size(), 0);
    solve_monitors.resize(tasks.size());
    solve_monitors[idx].New();
    auto & dm = *solve_monitors[idx];
  
    OnUpdate([this, idx, &dm](size_t) {
      dm.Reset();
      dm.AddDatum( solve_counts[idx] );
      solve_counts[idx] = 0;
    });
  }
  

  /**
   * Input: Id of a task from the internal vector
   *
   * Output: None
   *
   * Purpose: Increment corresponding count at solve_counts
   */
  void RecordSolve(size_t task_id) {
    if (task_id < solve_counts.size()) {
      ++solve_counts[task_id];
    }
  }

  /**
   * Input: A filename string
   *
   * Output: A Datafile
   *
   * Purpose: Setup the Datafile for data tracking
   */
  emp::DataFile &SetupSolveFile(const std::string &filename) {
    auto &file = SetupFile(filename);
    file.AddVar(update, "update", "Update step");

    // one column per task
    for (size_t i = 0; i < tasks.size(); ++i) {
      const std::string name = tasks[i]->name();
      file.AddTotal(*solve_monitors[i], "solves_" + name, "Total solves of " + name);
    }
    file.PrintHeaderKeys();
    return file;
  }

  /**
   * Input: index of a known organism.
   *
   * Output: the organism at the index
   *
   * Purpose: Removes the organism from the world but keeping a reference to it.
   */
  emp::Ptr<Organism> ExtractOrganism(int i) {
    emp::Ptr<Organism> org = pop[i];
    pop[i] = nullptr;
    return org;
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Runs Process() on all organisms in the world at random order.
   */
  void ProcessAllOrganisms() {
    emp::vector<size_t> schedule = emp::GetPermutation(GetRandom(), GetSize());
    for (int i : schedule) {
      if (!IsOccupied(i)) {
        continue;
      }
      pop[i]->Process(i);
    }
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Move all organisms in the world around at random order into empty space if possible.
   */
  void MoveAllOrganisms() {
    size_t world_size = GetSize();
    emp::vector<size_t> move_schedule = emp::GetPermutation(GetRandom(), world_size);
    for (size_t i : move_schedule) {
      if (!IsOccupied(i))
        continue;
      emp::Ptr<Organism> mover = ExtractOrganism(i);
      int target = i;
      for (int attempt = 0; attempt < 8; ++attempt) {
        emp::WorldPosition pos = GetRandomNeighborPos(i);
        if (!pos.IsValid()) continue;
        if (IsOccupied(pos)) continue;
        target = pos.GetIndex();
        break;
      }

      AddOrgAt(mover, target);
    }
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Make all organisms that can reproduce, do so at a random order.
   */
  void ReproduceAllValidOrganisms() {
    for (emp::WorldPosition location : reproduce_queue) {
      if (!IsOccupied(location)) { return; }
      std::optional<Organism> offspring =
          pop[location.GetIndex()]->CheckReproduction();
      if (offspring.has_value()) {
        DoBirth(offspring.value(), location.GetIndex());
      }
    }
    reproduce_queue.clear();
  }
  

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Update the state of the board
   */
  void Update() {

    emp::World<Organism>::Update();

    this->ProcessAllOrganisms();
    this->MoveAllOrganisms();
    this->ReproduceAllValidOrganisms();
    
  }
    
  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Make all organisms that can reproduce, do so at a random order.
   */
  void CheckOutput(float output, OrgState &state) {
    // TODO: Check if the organism solved a task!

    for (size_t i = 0; i < tasks.size(); ++i) {
      double pts = tasks[i]->CheckOutput(output, state.last_inputs);
      if (pts > 0.0) {
        std::cout << "Solved: " << tasks[i]->name()
        << " (+ " << pts << " points)\n";
        state.points += pts;
        RecordSolve(i);
        state.best_task = std::max(state.best_task, i);
      }
    }
  }

  void ReproduceOrg(emp::WorldPosition location) {
    // Wait until after all organisms have been processed to perform
    // reproduction. If reproduction happened immediately then the child could
    // ovewrite the parent, and then we would be running the code of a deleted
    // organism
    reproduce_queue.push_back(location);
  }
};

#endif