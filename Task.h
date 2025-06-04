#ifndef TASK_H
#define TASK_H

// #include <_types/_uint32_t.h>
#include <cmath>
#include <string>
#include <algorithm>
#include <iostream>
#include "OrgState.h"

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
class SendHighest : public Task {
public:
  double CheckOutput(OrgState &state) override {
    // std::cout << "CheckOutput SendHighest 0" <<std::endl;
    // std::cout << "Checking output of organism index: " << state.cell->GetIndex() <<std::endl;
    // std::cout << "CheckOutput SendHighest 1" <<std::endl;
    unsigned int cell_id  = state.cell->GetID();
    // std::cout << "CheckOutput SendHighest 2" <<std::endl;
    unsigned int retrieved = state.retrieved;

    unsigned int max_val = std::max(cell_id, retrieved);

    if ( state.message == max_val ) {
      return 10.0;
    }
    else {
      return 0.0;
    }
  }
  std::string name() const override { return "Send Highest"; }
};


#endif
