//==-- SystemZISelDAGToDAG.cpp - A dag to dag inst selector for SystemZ ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines an instruction selector for the SystemZ target.
//
//===----------------------------------------------------------------------===//

#include "SystemZ.h"
#include "SystemZISelLowering.h"
#include "SystemZTargetMachine.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Function.h"
#include "llvm/Intrinsics.h"
#include "llvm/CallingConv.h"
#include "llvm/Constants.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
using namespace llvm;

/// SystemZDAGToDAGISel - SystemZ specific code to select SystemZ machine
/// instructions for SelectionDAG operations.
///
namespace {
  class SystemZDAGToDAGISel : public SelectionDAGISel {
    SystemZTargetLowering &Lowering;
    const SystemZSubtarget &Subtarget;

  public:
    SystemZDAGToDAGISel(SystemZTargetMachine &TM, CodeGenOpt::Level OptLevel)
      : SelectionDAGISel(TM, OptLevel),
        Lowering(*TM.getTargetLowering()),
        Subtarget(*TM.getSubtargetImpl()) { }

    virtual void InstructionSelect();

    virtual const char *getPassName() const {
      return "SystemZ DAG->DAG Pattern Instruction Selection";
    }

    // Include the pieces autogenerated from the target description.
  #include "SystemZGenDAGISel.inc"

  private:
    SDNode *Select(SDValue Op);

  #ifndef NDEBUG
    unsigned Indent;
  #endif
  };
}  // end anonymous namespace

/// createSystemZISelDag - This pass converts a legalized DAG into a
/// SystemZ-specific DAG, ready for instruction scheduling.
///
FunctionPass *llvm::createSystemZISelDag(SystemZTargetMachine &TM,
                                        CodeGenOpt::Level OptLevel) {
  return new SystemZDAGToDAGISel(TM, OptLevel);
}


/// InstructionSelect - This callback is invoked by
/// SelectionDAGISel when it has created a SelectionDAG for us to codegen.
void SystemZDAGToDAGISel::InstructionSelect() {
  DEBUG(BB->dump());

  // Codegen the basic block.
#ifndef NDEBUG
  DOUT << "===== Instruction selection begins:\n";
  Indent = 0;
#endif
  SelectRoot(*CurDAG);
#ifndef NDEBUG
  DOUT << "===== Instruction selection ends:\n";
#endif

  CurDAG->RemoveDeadNodes();
}

SDNode *SystemZDAGToDAGISel::Select(SDValue Op) {
  SDNode *Node = Op.getNode();
  DebugLoc dl = Op.getDebugLoc();

  // Dump information about the Node being selected
  #ifndef NDEBUG
  DOUT << std::string(Indent, ' ') << "Selecting: ";
  DEBUG(Node->dump(CurDAG));
  DOUT << "\n";
  Indent += 2;
  #endif

  // If we have a custom node, we already have selected!
  if (Node->isMachineOpcode()) {
    #ifndef NDEBUG
    DOUT << std::string(Indent-2, ' ') << "== ";
    DEBUG(Node->dump(CurDAG));
    DOUT << "\n";
    Indent -= 2;
    #endif
    return NULL;
  }

  // Select the default instruction
  SDNode *ResNode = SelectCode(Op);

  #ifndef NDEBUG
  DOUT << std::string(Indent-2, ' ') << "=> ";
  if (ResNode == NULL || ResNode == Op.getNode())
    DEBUG(Op.getNode()->dump(CurDAG));
  else
    DEBUG(ResNode->dump(CurDAG));
  DOUT << "\n";
  Indent -= 2;
  #endif

  return ResNode;
}
