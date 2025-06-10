#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include "OrgState.h"
#include "emp/math/math.hpp"
#include "sgpl/library/OpLibraryCoupler.hpp"
#include "sgpl/library/prefab/ArithmeticOpLibrary.hpp"
#include "sgpl/library/prefab/NopOpLibrary.hpp"
#include "sgpl/operations/flow_global/Anchor.hpp"
#include "sgpl/program/Instruction.hpp"
#include "sgpl/program/Program.hpp"
#include "sgpl/spec/Spec.hpp"
#include <string>
// #include <_types/_uint32_t.h>

/**
 * A custom instruction that outputs the value of a register as the (possible)
 * solution to a task, and then gets a new input value and stores it in the same
 * register.
 * Redudant with new Cell class usage. Retained in case of needing to use it again.
 */
// struct IOInstruction
// {
//   template <typename Spec>
//   static void run(sgpl::Core<Spec> &core, const sgpl::Instruction<Spec> &inst,
//                   const sgpl::Program<Spec> &,
//                   typename Spec::peripheral_t &state) noexcept
//   {
//     // uint32_t output = core.registers[inst.args[0]];
//     state.world->CheckOutput(state);

//     uint32_t input = sgpl::tlrand.Get().GetUInt();
//     core.registers[inst.args[0]] = input;
//     // state.add_input(input);
//   }

//   static std::string name() { return "IO"; }
//   static size_t prevalence() { return 1; }
// };

struct NandInstruction
{
  template <typename Spec>
  static void run(sgpl::Core<Spec> &core, const sgpl::Instruction<Spec> &inst,
                  const sgpl::Program<Spec> &,
                  typename Spec::peripheral_t &state) noexcept
  {
    uint32_t reg_b = core.registers[inst.args[1]];
    uint32_t reg_c = core.registers[inst.args[2]];
    uint32_t nand_val = ~(reg_b & reg_c);

    core.registers[inst.args[0]] = nand_val;
  }
  static std::string name() { return "Nand"; }
  static size_t prevalence() { return 1; }
};

/**
 * A custom instruction that attempts to reproduce and produce a child organism,
 * if this organism has enough points.
 */
struct ReproduceInstruction
{
  template <typename Spec>
  static void run(sgpl::Core<Spec> &core, const sgpl::Instruction<Spec> &inst,
                  const sgpl::Program<Spec> &,
                  typename Spec::peripheral_t &state) noexcept
  {
    if (state.points > 20)
    {
      state.world->ReproduceOrg(state.current_location);
      state.points -= 0;
    }
  }

  static std::string name() { return "Reproduce"; }
  static size_t prevalence() { return 10; }
};

struct GetFacing { 
    template <typename Spec>
    static void run(sgpl::Core<Spec> &core, const sgpl::Instruction<Spec> &inst,
                  const sgpl::Program<Spec> &,
                  typename Spec::peripheral_t &state) noexcept {
        core.registers[inst.args[0]] = state.cell->GetFacing();
    }

    static std::string name() { return "GetFacing"; } 
    static size_t prevalence() { return 1; }
};

struct RotateLeft { 
    template <typename Spec>
    static void run(sgpl::Core<Spec> &core, const sgpl::Instruction<Spec> &inst,
                  const sgpl::Program<Spec> &,
                  typename Spec::peripheral_t &state) noexcept {
        state.cell->RotateLeft();
    }

    static std::string name() { return "RotateLeft"; } 
    static size_t prevalence() { return 1; }
};

struct RotateRight { 
    template <typename Spec>
    static void run(sgpl::Core<Spec> &core, const sgpl::Instruction<Spec> &inst,
                  const sgpl::Program<Spec> &,
                  typename Spec::peripheral_t &state) noexcept {
        state.cell->RotateRight();
    }

    static std::string name() { return "RotateRight"; } 
    static size_t prevalence() { return 1; }
};

struct GetID { 
    template <typename Spec>
    static void run(sgpl::Core<Spec> &core, const sgpl::Instruction<Spec> &inst,
                  const sgpl::Program<Spec> &,
                  typename Spec::peripheral_t &state) noexcept {
        
        core.registers[inst.args[0]] = state.cell->GetID();
    }

    static std::string name() { return "GetID"; } 
    static size_t prevalence() { return 1; }
};

struct SendMessage { 
    template <typename Spec>
    static void run(sgpl::Core<Spec> &core, const sgpl::Instruction<Spec> &inst,
                  const sgpl::Program<Spec> &,
                  typename Spec::peripheral_t &state) noexcept {
        emp::WorldPosition loc = state.current_location;
        unsigned int message = core.registers[inst.args[0]];
        state.message = message;
        int sent = state.world->SendMessage(loc.GetIndex(), state.message);
        if (sent){state.world->CheckOutput(state);}
        core.registers[inst.args[0]] =  sgpl::tlrand.Get().GetUInt();
    }

    static std::string name() { return "SendMessage"; } 
    static size_t prevalence() { return 1; }
};

struct RetrieveMessage { 
    template <typename Spec>
    static void run(sgpl::Core<Spec> &core, const sgpl::Instruction<Spec> &inst,
                  const sgpl::Program<Spec> &,
                  typename Spec::peripheral_t &state) noexcept {
        emp::WorldPosition loc = state.current_location;
        if (state.inbox) {
          state.world->RetrieveMessage(loc.GetIndex(), state.inbox);
          core.registers[inst.args[0]] = state.retrieved;
        }
        
        
    }

    static std::string name() { return "RetrieveMessage"; } 
    static size_t prevalence() { return 1; }
};


using Library =
    sgpl::OpLibraryCoupler<sgpl::NopOpLibrary, 
                           sgpl::TerminateIf,
                           sgpl::Add,
                           sgpl::Divide,
                           sgpl::Modulo,
                           sgpl::Multiply,
                           sgpl::Subtract,
                           sgpl::BitwiseAnd,
                           sgpl::BitwiseNot,
                           sgpl::BitwiseOr,
                           sgpl::BitwiseShift,
                           sgpl::BitwiseXor,
                           sgpl::CountOnes,
                           sgpl::RandomFill,
                           sgpl::Equal,
                           sgpl::GreaterThan,
                           sgpl::LessThan,
                           sgpl::LogicalAnd,
                           sgpl::LogicalOr,
                           sgpl::NotEqual,
                           sgpl::global::Anchor,
                           sgpl::global::JumpIf,
                           sgpl::global::JumpIfNot,
                           sgpl::global::RegulatorAdj<>,
                           sgpl::global::RegulatorDecay<>,
                           sgpl::global::RegulatorGet<>,
                           sgpl::global::RegulatorSet<>,
                           sgpl::local::Anchor,
                           sgpl::local::JumpIf,
                           sgpl::local::JumpIfNot,
                           sgpl::local::RegulatorAdj,
                           sgpl::local::RegulatorDecay,
                           sgpl::local::RegulatorGet,
                           sgpl::local::RegulatorSet,
                           sgpl::Decrement,
                           sgpl::Increment,
                           sgpl::Negate,
                           sgpl::Not,
                           sgpl::RandomBool,
                           sgpl::RandomDraw,
                           sgpl::Terminal, 
                          //  IOInstruction, 
                           NandInstruction,
                           ReproduceInstruction,
                           GetFacing, RotateLeft, RotateRight, GetID, SendMessage, RetrieveMessage>;

using Spec = sgpl::Spec<Library, OrgState>;

#endif