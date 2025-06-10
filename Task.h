#ifndef TASK_H
#define TASK_H

#include <cmath>
#include <string>
#include <algorithm>
#include <iostream>
#include "OrgState.h"
#include "World.h"

/**
 * The interface for a task that organisms can complete.
 */
class Task {
  public:
    virtual ~Task() = default;
  
    /**
     * Input: An output value and the last four inputs the organism received.
     * 
     * Output: The number of points for (if any) the task was completed.
     * 
     * Purpose: Checks if the output is correct for any input.
    */
    virtual double CheckOutput(OrgState &state) {
      return 0.0;
    }
  
    /// Human-readable name
    virtual std::string name() const = 0;
  
  };

// Task: Nothing. This is just for the purpose of categorizing the organisms
class Initial : public Task {
  public:
    double CheckOutput(OrgState &state) override {
      return 0.0;
    }

    std::string name() const override { return "Initial"; }
  };

  
// Task: returns the highest value between received cell ID and org's cell's cell ID
class TargetAnother : public Task {
public:
  double CheckOutput(OrgState &state) override {
    Cell* cur_cell = state.cell;
    Cell* tar_cell = cur_cell->GetFacingCell();
    if (tar_cell->GetHasOrg()) {
      return 1.0;
    }
    else {
      return 0.0;
    }
  }
  std::string name() const override { return "Target Another Organism"; }
};

  
// Task: returns the highest value between received cell ID and org's cell's cell ID
class FaceAnother : public Task {
public:
  double CheckOutput(OrgState &state) override {
    Cell* cur_cell = state.cell;
    Cell* tar_cell = cur_cell->GetFacingCell();
    if (tar_cell->GetHasOrg() && tar_cell->GetFacingCell() == cur_cell ) {
      return 10.0;
    }
    else {
      return 0.0;
    }
  }
  std::string name() const override { return "Face Another Organism"; }
};

// Task: returns the highest value between received cell ID and org's cell's cell ID
class PrepMessage : public Task {
public:
  double CheckOutput(OrgState &state) override {

    if (state.message > 0) {
      return 1.0;
    }
    else {
      return 0.0;
    }
  }
  std::string name() const override { return "Prepare Message"; }
};



// Task: returns the highest value between received cell ID and org's cell's cell ID
class PrepHighest : public Task {
public:
  double CheckOutput(OrgState &state) override {
    Cell* cur_cell = state.cell;
    Cell* tar_cell = cur_cell->GetFacingCell();
    unsigned int cell_id  = cur_cell->GetID();
    unsigned int retrieved = state.retrieved;

    unsigned int max_val = std::max(cell_id, retrieved);

    if (state.message == max_val) {
      return 20.0;
    }
    else {
      return 0.0;
    }
  }
  std::string name() const override { return "Prepare Highest Value"; }
};


// Task: returns the highest value between received cell ID and org's cell's cell ID
class SendHighest : public Task {
public:
  double CheckOutput(OrgState &state) override {
    Cell* cur_cell = state.cell;
    Cell* tar_cell = cur_cell->GetFacingCell();
    unsigned int cell_id  = cur_cell->GetID();
    unsigned int retrieved = state.retrieved;

    unsigned int max_val = std::max(cell_id, retrieved);

    if (tar_cell->GetHasOrg() && tar_cell->GetFacingCell() == cur_cell && state.message == max_val ) {
      return 30.0;
    }
    else {
      return 0.0;
    }
  }
  std::string name() const override { return "Send Highest"; }
};

// Task: returns the highest value between received cell ID and org's cell's cell ID
class SendSelf : public Task {
public:
  double CheckOutput(OrgState &state) override {
    Cell* cur_cell = state.cell;
    Cell* tar_cell = cur_cell->GetFacingCell();
    unsigned int cell_id  = cur_cell->GetID();

    if (state.message == cell_id ) {
      return 10.0;
    }
    else {
      return 0.0;
    }
  }
  std::string name() const override { return "Send Self ID"; }
};

// Task: returns the highest value between received cell ID and org's cell's cell ID
class SendID : public Task {
public:
  double CheckOutput(OrgState &state) override {

    Cell* cur_cell = state.cell;
    Cell* tar_cell = cur_cell->GetFacingCell();
    unsigned int cell_id  = cur_cell->GetID();

    if (state.retrieved_values.count(state.message) || state.message == cell_id) {
      return 20.0;
    }
    else {
      return 0.0;
    }
  }
  std::string name() const override { return "Send Any ID"; }
};

// Task: returns the highest value between received cell ID and org's cell's cell ID
class SendNonID : public Task {
public:
  double CheckOutput(OrgState &state) override {

    Cell* cur_cell = state.cell;
    Cell* tar_cell = cur_cell->GetFacingCell();
    unsigned int cell_id  = cur_cell->GetID();

    if (!(state.retrieved_values.count(state.message) || state.message == cell_id)) {
      return 0.0;
    }
    else {
      return -5.0;
    }
  }
  std::string name() const override { return "Send Non ID"; }
};

// Task: returns the highest value between received cell ID and org's cell's cell ID
class MaxKnown : public Task {
public:
  double CheckOutput(OrgState &state) override {

    Cell* cur_cell = state.cell;
    Cell* tar_cell = cur_cell->GetFacingCell();

    if (state.max_known && state.message == state.max_known) {
      return 30.0;
    }
    else {
      return 0.0;
    }
  }
  std::string name() const override { return "Send Max Known"; }
};



#endif
