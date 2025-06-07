#ifndef WORLD_H
#define WORLD_H

#include "emp/Evolve/World.hpp"
#include "emp/data/DataFile.hpp"
#include <vector>
#include "Org.h"
#include "Task.h"
#include "Cell.h"
#include "ConfigSetup.h"

class OrgWorld : public emp::World<Organism>
{
  emp::vector<emp::WorldPosition> reproduce_queue;
  std::vector<Task *> tasks;
  std::vector<emp::Ptr<emp::DataMonitor<int>>> solve_monitors;
  std::vector<int> solve_counts;
  const int num_h_boxes = worldConfig.WORLD_LEN();
  const int num_w_boxes = worldConfig.WORLD_WIDTH();
  emp::Random random{ worldConfig.SEED() };

  std::vector<std::vector<Cell*>> cell_grid = std::vector<std::vector<Cell*>>( num_w_boxes, std::vector<Cell*>(num_h_boxes) );
  // direction‐vectors for 8 neighbors:
  //    0: N   ( 0,-1)
  //    1: NE  ( 1,-1)
  //    2: E   ( 1, 0)
  //    3: SE  ( 1, 1)
  //    4: S   ( 0, 1)
  //    5: SW  (-1, 1)
  //    6: W   (-1, 0)
  //    7: NW  (-1,-1)
  static constexpr int dx[8] = {0, 1, 1, 1, 0, -1, -1, -1};
  static constexpr int dy[8] = {-1, -1, 0, 1, 1, 1, 0, -1};

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
    AddTask(new SendHighest());
    SetupWorld();
    SetupCellGrid();
    LinkAllNeighbors();
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
   * Output: Returns various local variables.
   *
   * Purposes: To be called from other files
   */
  const pop_t &GetPopulation() { return pop; }
  auto GetTasks() { return tasks; }
  auto GetSolveMonitors() { return solve_monitors; }
  Cell * GetCellByLinearIndex(int idx) const {
    const int total = num_w_boxes * num_h_boxes;
    if (idx < 0 || idx >= total) return nullptr;

    // linear index = x*H + y
    const int x = idx / num_h_boxes;
    const int y = idx % num_h_boxes;
    return cell_grid[x][y];
  }

  /**
     * Input: None
     *
     * Output: None
     *
     * Purpose: Setup the world's grid structure
     */
    void SetupWorld()
    {
        // setup the world
        SetPopStruct_Grid(num_w_boxes, num_h_boxes);
        Resize(num_h_boxes, num_w_boxes);
    }

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
     * Input: None
     *
     * Output: None
     *
     * Purpose: Setup the cell grid, iterate through them and set linear index equivalent to how organism indices are set.
     */
    void SetupCellGrid()
    {
        int org_num = 0;
        for (int x = 0; x < num_w_boxes; x++)
        {
            for (int y = 0; y < num_h_boxes; y++)
            {
                Cell* new_cell = new Cell();
                new_cell->SetIndex(org_num);
                int random_dir = static_cast<int>( random.GetUInt(8) );
                new_cell->SetFacing(random_dir);
                // std::cout << "Made cell index " << org_num <<std::endl;
                cell_grid[x][y] = new_cell;
                // std::cout << cell_grid[x][y]->GetIndex() << std::endl;
                org_num++;
            }
        }
    }

   void LinkAllNeighbors() {
    for (int x = 0; x < num_w_boxes; ++x) {
      for (int y = 0; y < num_h_boxes; ++y) {
        Cell * C = cell_grid[x][y];
        // Fill its 8‐entry connections[] vector:
        for (int dir = 0; dir < 8; ++dir) {
          int nx = emp::Mod(x + dx[dir], num_w_boxes);
          int ny = emp::Mod(y + dy[dir], num_h_boxes);
          C->SetConnection(dir, cell_grid[nx][ny]);
          // std::cout << "Connected (" << x << "," << y <<") with dir " << dir << " (" << nx << "," << ny <<")" <<std::endl;
        }
      }
    }
  }

  
  bool CellsAreFacing(Cell* cell1, Cell* cell2){
      return (cell1->GetFacingCell() == cell2 && cell2->GetFacingCell() == cell1);
  }
  
  /**
   * Input: index of a known organism.
   *
   * Output: the organism at the index
   *
   * Purpose: Removes the organism from the world but keeping a reference to it.
   */
  // emp::Ptr<Organism> ExtractOrganism(int i) {
  //   emp::Ptr<Organism> org = pop[i];
  //   pop[i] = nullptr;
  //   org->SetCell(nullptr);
  //   return org;
  // }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Runs Process() on all organisms in the world at random order.
   */
  void ProcessAllOrganisms() {
    // std::cout << "Process 0" <<std::endl;
    emp::vector<size_t> schedule = emp::GetPermutation(GetRandom(), GetSize());
    // std::cout << "Process 1" <<std::endl;
    for (int i : schedule) {
      if (!IsOccupied(i)) {
        continue;
      }
      // std::cout << "Process 2" <<std::endl;
      pop[i]->Process(i);
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
        // std::cout << "Organism at index " << location.GetIndex() << " reproduced!" << std::endl;
        DoBirth(offspring.value(), location.GetIndex());
      }
    }
    reproduce_queue.clear();
  }

  void BindAllOrganismsToCell(){
    for(int i = 0; i < GetSize(); i++){
      if (!IsOccupied(i))
        continue;
      pop[i]->SetCell(GetCellByLinearIndex(i));
    }
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
    // std::cout << "Update 0" <<std::endl;
    this->BindAllOrganismsToCell();
    // std::cout << "Update 1" <<std::endl;
    this->ProcessAllOrganisms();
    // this->MoveAllOrganisms();
    // std::cout << "Update 2" <<std::endl;
    this->ReproduceAllValidOrganisms();
    // std::cout << "Update 3" <<std::endl;
    // this->BindAllOrganismsToCell();
  }
    
  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Make all organisms that can reproduce, do so at a random order.
   */
  void CheckOutput(OrgState &state) {
    // TODO: Check if the organism solved a task!

    for (size_t i = 0; i < tasks.size(); ++i) {
      double pts = tasks[i]->CheckOutput(state);
      if (pts > 0.0) {
        // int completed_linear_index = state.cell->GetIndex();
        // Organism* achiever = pop[completed_linear_index];
        std::cout << "Org at index " 
        // << completed_linear_index 
        << " Completed: " << tasks[i]->name() << " (+ " << pts << " points)\n";
        state.points += pts;
        RecordSolve(i);
        state.best_task = std::max(state.best_task, i);
        // exit(0);
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

  void SendMessage(int location, int message) {
    // std::cout << "SendMessage 0" <<std::endl;
    Organism* sender = pop[location];
    // std::cout << "SendMessage 1" <<std::endl;
    // std::cout << "sender location: "  << location <<std::endl;
    // std::cout << "sending to dir: "  << dir <<std::endl;
    Cell* sender_cell = sender->GetCell();
    // std::cout << "SendMessage 2" <<std::endl;
    // std::cout << "sender index: "  << sender_cell->GetIndex() <<std::endl;
    Cell* target_cell = sender_cell->GetFacingCell();
    // std::cout << "SendMessage 3" <<std::endl;
    int receiver_index = target_cell->GetIndex();
    // std::cout << "SendMessage 4" <<std::endl;
    if (IsOccupied(receiver_index) && CellsAreFacing(sender_cell, target_cell)) {
    // std::cout << "SendMessage 5" <<std::endl;
      pop[receiver_index]->SetInbox(message);
    }
  }
};

#endif