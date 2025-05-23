#ifndef TASK_H
#define TASK_H

// #include <_types/_uint32_t.h>
#include <cmath>
#include <string>
#include <iostream>

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
    double CheckOutput(float output, uint32_t inputs[4]) {
      static emp::Random rng{ worldConfig.SEED() };
      emp::vector<size_t> check_schedule = emp::GetPermutation(rng, 4);
      for (size_t i : check_schedule) {
        double pts = CheckOne(output, inputs[i]);
        if (pts > 0.0) return pts;
      }
      return 0.0;
    }
  
    /// Human-readable name
    virtual std::string name() const = 0;
  
  protected:
    /**
     * Input: An output value and an input the organism received.
     * 
     * Output: The number of points for (if any) the task was completed.
     * 
     * Purpose: Be overwritten in subclasses and checks if the output is correct for any input.
    */
    virtual double CheckOne(float output, uint32_t input) = 0;
  };

// Task: Nothing. This is just for the purpose of categorizing the organisms
class Initial : public Task {
  protected:
    double CheckOne(float output, uint32_t input) override {
      return 0.0;
    }
  public:
    std::string name() const override { return "Initial"; }
  };

// Task: returns a value equal to the input
class ComputeEqual : public Task {
protected:
  double CheckOne(float output, uint32_t input) override {
    return (std::fabs(output - input) < 0.001) ? 1.0 : 0.0;
  }
public:
  std::string name() const override { return "Compute Equal"; }
};

// Task: returns a value one more than the input
class ComputeIncrement : public Task {
  protected:
  double CheckOne(float output, uint32_t input) override {
    return (std::fabs(output - (input+1)) < 0.001) ? 2.0 : 0.0;
  }
public:
  std::string name() const override { return "Compute Increment"; }
};

// Task: returns a value less than the input
class ComputeDecrement : public Task {
  protected:
  double CheckOne(float output, uint32_t input) override {
    return (std::fabs(output - (input-1)) < 0.001) ? 2.0 : 0.0;
  }
public:
  std::string name() const override { return "Compute Decrement"; }
};

// Task: returns a value double the input
class ComputeDouble : public Task {
  protected:
  double CheckOne(float output, uint32_t input) override {
    return (std::fabs(output - input*2) < 0.001) ? 100.0 : 0.0;
  }
public:
  std::string name() const override { return "Compute Double"; }
};

// Task: returns a value half the input
class ComputeHalf : public Task {
  protected:
  double CheckOne(float output, uint32_t input) override {
    return (std::fabs(output - input/2) < 0.001) ? 50.0 : 0.0;
  }
public:
  std::string name() const override { return "Compute Half"; }
};

// Task: returns a value equal to the input squared
class ComputeSquare : public Task {
  protected:
    double CheckOne(float output, uint32_t input) override {
      return (std::fabs(output - input*input) < 0.001) ? 1000.0 : 0.0;
    }
  public:
    std::string name() const override { return "Compute Square"; }
};

// Task: returns a value equal to the input square root
class ComputeSquareRoot : public Task {
  protected:
    double CheckOne(float output, uint32_t input) override {
      return (std::fabs(output - std::sqrt(input)) < 0.001) ? 1000.0 : 00;
    }
  public:
    std::string name() const override { return "Compute Square Root"; }
};


// Task: returns a value equal to the natural log of the input
class ComputeLog : public Task {
  protected:
  double CheckOne(float output, uint32_t input) override {
    return (std::fabs(output - std::log(input)) < 0.001) ? 10000.0 : 0.0;
  }
public:
  std::string name() const override { return "Compute Log"; }
};

#endif
