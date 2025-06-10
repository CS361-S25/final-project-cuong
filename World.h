// World.h
#ifndef WORLD_H
#define WORLD_H

#include "emp/Evolve/World.hpp"
#include "emp/data/DataFile.hpp"
#include <vector>
#include <unordered_map>
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

  std::vector<unsigned int> all_cell_ids;
  std::unordered_map<unsigned int, int> id_to_idx;

  std::vector<emp::Ptr<emp::DataMonitor<int>>> send_monitors;
  std::vector<emp::Ptr<emp::DataMonitor<int>>> recv_monitors;
  std::vector<int> send_counts;
  std::vector<int> recv_counts;

  emp::Ptr<emp::DataMonitor<int>> send_other_mon;
  emp::Ptr<emp::DataMonitor<int>> recv_other_mon;
  int send_other_count = 0;
  int recv_other_count = 0;

  const int num_h_boxes = worldConfig.WORLD_LEN();
  const int num_w_boxes = worldConfig.WORLD_WIDTH();
  emp::Random random{worldConfig.SEED()};

  std::vector<std::vector<Cell *>> cell_grid = std::vector<std::vector<Cell *>>(num_w_boxes, std::vector<Cell *>(num_h_boxes));
  unsigned int max_id;
  unsigned int min_id;
  // direction‐vectors for 8 neighbors:
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
  OrgWorld(emp::Random &_random) : emp::World<Organism>(_random)
  {
    AddTask(new Initial());
    // AddTask(new TargetAnother());
    // AddTask(new FaceAnother());
    // AddTask(new PrepMessage());
    // AddTask(new PrepHighest());
    // AddTask(new SendHighest());
    AddTask(new SendNonID());
    // AddTask(new SendSelf());
    AddTask(new SendID());
    AddTask(new MaxKnown());

    SetupWorld();
    SetupCellGrid();
    LinkAllNeighbors();
    SetupSendRecvMonitors();
    
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
   * Output: None
   *
   * Purpose: Setup data monitor for messages being sent and retrieved
   */
  void SetupSendRecvMonitors(){
    const int total_cells = num_w_boxes * num_h_boxes;
    send_counts.assign(total_cells, 0);
    recv_counts.assign(total_cells, 0);
    send_monitors.resize(total_cells);
    recv_monitors.resize(total_cells);

    for (int i = 0; i < total_cells; ++i)
    {
      send_monitors[i].New();
      recv_monitors[i].New();
    }

    all_cell_ids.reserve(num_w_boxes * num_h_boxes);
    for (int x = 0; x < num_w_boxes; ++x)
    {
      for (int y = 0; y < num_h_boxes; ++y)
      {
        unsigned id = cell_grid[x][y]->GetID();
        int idx = (int)all_cell_ids.size();
        all_cell_ids.push_back(id);
        id_to_idx[id] = idx;
      }
    }

    send_other_mon.New();
    recv_other_mon.New();

    OnUpdate([this](size_t)
             {
    // reset & record per‐cell
    for (int i = 0; i < (int)send_monitors.size(); ++i) {
      auto & m = *send_monitors[i];
      m.Reset();
      m.AddDatum( send_counts[i] );
      send_counts[i] = 0;
    }

    for (int i = 0; i < (int)recv_monitors.size(); ++i) {
      auto & m = *recv_monitors[i];
      m.Reset();
      m.AddDatum( recv_counts[i] );
      recv_counts[i] = 0;
    }

    send_other_mon->Reset();
    send_other_mon->AddDatum( send_other_count );
    send_other_count = 0;

    recv_other_mon->Reset();
    recv_other_mon->AddDatum( recv_other_count );
    recv_other_count = 0; });

  }

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
  auto GetSendMonitors() { return send_monitors; }
  auto GetRecvMonitors() { return recv_monitors; }
  auto GetSendOtherMon() { return send_other_mon; }
  auto GetRecvOtherMon() { return recv_other_mon; }
  auto GetIdToIdx() { return &id_to_idx; }
  int GetMsgBinCount() const
  {
    return (int)all_cell_ids.size() + 1;
  }
  std::string GetMsgBinLabel(int b) const
  {
    if (b == 0)
      return "Other";
    return "ID_" + std::to_string(all_cell_ids[b - 1]);
  }

  Cell *GetCellByLinearIndex(int idx) const
  {
    const int total = num_w_boxes * num_h_boxes;
    if (idx < 0 || idx >= total)
      return nullptr;

    const int x = idx / num_h_boxes;
    const int y = idx % num_h_boxes;
    return cell_grid[x][y];
  }
  Cell *GetCellByGridCoord(int x, int y)
  {
    return cell_grid[x][y];
  }

  unsigned int GetMaxID() { return max_id; }
  unsigned int GetMinID() { return min_id; }

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
  void AddTask(Task *task)
  {
    tasks.push_back(task);

    const size_t idx = tasks.size() - 1;
    solve_counts.resize(tasks.size(), 0);
    solve_monitors.resize(tasks.size());
    solve_monitors[idx].New();
    auto &dm = *solve_monitors[idx];

    OnUpdate([this, idx, &dm](size_t)
             {
      dm.Reset();
      dm.AddDatum( solve_counts[idx] );
      solve_counts[idx] = 0; });
  }

  /**
   * Input: Id of a task from the internal vector
   *
   * Output: None
   *
   * Purpose: Increment corresponding count at datafiles
   */
  void RecordSolve(size_t task_id)
  {
    if (task_id < solve_counts.size())
    {
      ++solve_counts[task_id];
    }
  }
  void RecordSend(int cell_idx)
  {
    if (cell_idx >= 0 && cell_idx < (int)send_counts.size())
      ++send_counts[cell_idx];
  }
  void RecordReceive(int cell_idx)
  {
    if (cell_idx >= 0 && cell_idx < (int)recv_counts.size())
      ++recv_counts[cell_idx];
  }

  /**
   * Input: A filename string
   *
   * Output: A Datafile
   *
   * Purpose: Setup the Datafiles for data tracking
   */
  emp::DataFile &SetupSolveFile(const std::string &filename)
  {
    auto &file = SetupFile(filename);
    file.AddVar(update, "update", "Update step");

    // one column per task
    for (size_t i = 0; i < tasks.size(); ++i)
    {
      const std::string name = tasks[i]->name();
      file.AddTotal(*solve_monitors[i], "solves_" + name, "Total solves of " + name);
    }
    file.PrintHeaderKeys();
    return file;
  }

  emp::DataFile &SetupSendRecvFile(const std::string &filename)
  {
    auto &file = SetupFile(filename);
    file.AddVar(update, "update", "Update step");

    // one column per actual cell ID
  for (size_t i = 0; i < all_cell_ids.size(); ++i) {
    const auto id = all_cell_ids[i];
    file.AddTotal(*send_monitors[i],
                  "send_ID_" + std::to_string(id),
                  "Sends to cell " + std::to_string(id));
    file.AddTotal(*recv_monitors[i],
                  "recv_ID_" + std::to_string(id),
                  "Retrieves from cell " + std::to_string(id));
  }

  file.AddTotal(*send_other_mon, "send_other", "Sends to non‐cell ID");
  file.AddTotal(*recv_other_mon, "recv_other", "Retrieves non‐cell ID");

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
        Cell *new_cell = new Cell();
        new_cell->SetIndex(org_num);
        int random_dir = static_cast<int>(random.GetUInt(8));
        new_cell->SetFacing(random_dir);
        unsigned int random_id = random.GetUInt();
        new_cell->SetID(random_id);
        // std::cout << "Made cell index " << org_num <<std::endl;
        cell_grid[x][y] = new_cell;
        unsigned int new_id = new_cell->GetID();
        if (max_id)
        {
          max_id = std::max(max_id, new_id);
        }
        else
        {
          max_id = new_id;
        }
        if (min_id)
        {
          min_id = std::min(min_id, new_id);
        }
        else
        {
          min_id = new_id;
        }
        // std::cout << cell_grid[x][y]->GetIndex() << std::endl;
        org_num++;
      }
    }
  }

  /**
   * Input: None
   * 
   * Output: None
   * 
   * Purpoose: Set up pointers conncting each cell to its neighbors.
   */
  void LinkAllNeighbors()
  {
    for (int x = 0; x < num_w_boxes; ++x)
    {
      for (int y = 0; y < num_h_boxes; ++y)
      {
        Cell *C = cell_grid[x][y];
        // Fill its 8‐entry connections[] vector:
        for (int dir = 0; dir < 8; ++dir)
        {
          int nx = emp::Mod(x + dx[dir], num_w_boxes);
          int ny = emp::Mod(y + dy[dir], num_h_boxes);
          C->SetConnection(dir, cell_grid[nx][ny]);
        }
      }
    }
  }

  /**
   * Input: index of a known organism.
   *
   * Output: the organism at the index
   *
   * Purpose: Removes the organism from the world but keeping a reference to it.
   */
  emp::Ptr<Organism> ExtractOrganism(int i)
  {
    emp::Ptr<Organism> org = pop[i];
    pop[i] = nullptr;
    Cell *blank_cell = org->GetCell();
    org->SetCell(nullptr);
    blank_cell->SetHasOrg(false);
    return org;
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Runs Process() on all organisms in the world at random order.
   */
  void ProcessAllOrganisms()
  {
    emp::vector<size_t> schedule = emp::GetPermutation(GetRandom(), GetSize());
    for (int i : schedule)
    {
      if (!IsOccupied(i))
      {
        continue;
      }
      pop[i]->Process(i);
      if (pop[i]->GetPoints() < 0)
      {
        ExtractOrganism(i);
      }
    }
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Make all organisms that can reproduce, do so at a random order.
   */
  void ReproduceAllValidOrganisms()
  {
    for (emp::WorldPosition location : reproduce_queue)
    {
      if (!IsOccupied(location))
      {
        return;
      }
      Organism *org = pop[location.GetIndex()];
      std::optional<Organism> offspring =
          org->CheckReproduction();
      if (offspring.has_value())
      {
        // std::cout << "Organism at index " << location.GetIndex() << " reproduced!" << std::endl;
        DoBirth(offspring.value(), location.GetIndex());
        org->AddReproduced(1);
      }
    }
    reproduce_queue.clear();
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Establish link between organisms and cells so their data can be used together
   */
  void BindAllOrganismsToCell()
  {
    for (int i = 0; i < GetSize(); i++)
    {
      if (!IsOccupied(i))
        continue;
      Cell *cur_cell = GetCellByLinearIndex(i);
      pop[i]->SetCell(cur_cell);
      cur_cell->SetHasOrg(true);
    }
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Update the state of the board
   */
  void Update()
  {
    emp::World<Organism>::Update();
    this->BindAllOrganismsToCell();
    this->ProcessAllOrganisms();
    this->ReproduceAllValidOrganisms();
  }

  /**
   * Input: None
   *
   * Output: None
   *
   * Purpose: Make all organisms that can reproduce, do so at a random order.
   */
  void CheckOutput(OrgState &state)
  {

    for (size_t i = 0; i < tasks.size(); ++i)
    {
      double pts = tasks[i]->CheckOutput(state);

      if (pts != 0.0)
      {
        state.points += pts;
        RecordSolve(i);
        state.best_task = std::max(state.best_task, i);
      }
    }
  }

  void ReproduceOrg(emp::WorldPosition location)
  {
    // Wait until after all organisms have been processed to perform
    // reproduction. If reproduction happened immediately then the child could
    // ovewrite the parent, and then we would be running the code of a deleted
    // organism
    reproduce_queue.push_back(location);
  }

  /**
   * Input: An organism's location and the message they want to send.
   *
   * Output: 0 or 1 showing if a send is successful.
   *
   * Purpose: Send a message, and custom print statements to write the report when the web version is broken.
   */
  int SendMessage(int location, unsigned int message)
  {
    Organism *sender = pop[location];
    Cell *sender_cell = sender->GetCell();
    unsigned int sender_id = sender_cell->GetID();
    int sender_idx = sender_cell->GetIndex();

    Cell *target_cell = sender_cell->GetFacingCell();
    unsigned int target_id = target_cell->GetID();
    int target_idx = target_cell->GetIndex();

    if (IsOccupied(target_idx) && target_cell->GetFacingCell() == sender_cell && message)
    {
      auto it = id_to_idx.find(message);
      std::string isID;
      if (it != id_to_idx.end())
      {
        send_counts[it->second]++;
        isID = " ID ";
      }
      else
      {
        send_other_count++;
        isID = " No ";
      }

      std::cout << "Org " << sender_idx << "-" << sender_id << " sent " << message << isID << " to Org " << target_idx << "-" << target_id << std::endl;
      pop[target_idx]->SetInbox(message);
      return 1;
    }
    return 0;
  }

  /**
   * Input: An organism's location and the message they will retrieve.
   *
   * Output: None
   *
   * Purpose: Retrieve a message, and update the organism's knowledge pool of other cell-IDs.
   */
  void RetrieveMessage(int location, unsigned int msg_id)
  {
    Organism *retriever = pop[location];
    Cell *retriever_cell = retriever->GetCell();
    unsigned int retriever_id = retriever_cell->GetID();
    int retriever_idx = retriever_cell->GetIndex();
    unsigned int inbox_content = retriever->GetInbox();

    if (inbox_content)
    {
      retriever->SetRetrieved(inbox_content);
      retriever->AddRetrievedValue(inbox_content);

      unsigned int max_known = retriever->GetMaxKnown();
      if (max_known)
      {
        retriever->SetMaxKnown(std::max(max_known, inbox_content));
      }
      else
      {
        retriever->SetMaxKnown(std::max(max_known, retriever_id));
      }
    }
    auto it = id_to_idx.find(msg_id);
    if (it != id_to_idx.end())
    {
      recv_counts[it->second]++;
    }
    else
    {
      recv_other_count++;
    }
  }
};

#endif