
internal
bool
operator==(machine const& A, machine const& B)
{
  return AreBytesEqual(sizeof(machine), &A, &B);
}

internal
bool
operator!=(machine const& A, machine const& B)
{
  return !(A == B);
}


//
// ===============================================
//

#define INST0(InstType) instruction{ instruction_type::InstType }
#define INST1(InstType, Arg1Type, Arg1Val) instruction{ instruction_type::InstType, argument{ argument_type::Arg1Type, Arg1Val } }
#define INST2(InstType, Arg1Type, Arg1Val, Arg2Type, Arg2Val) instruction{ instruction_type::InstType, argument{ argument_type::Arg1Type, Arg1Val }, argument{ argument_type::Arg2Type, Arg2Val } }
#define INST3(InstType, Arg1Type, Arg1Val, Arg2Type, Arg2Val, Arg3Type, Arg3Val) instruction{ instruction_type::InstType, argument{ argument_type::Arg1Type, Arg1Val }, argument{ argument_type::Arg2Type, Arg2Val }, argument{ argument_type::Arg3Type, Arg3Val } }

internal
void
RunTests()
{
  using inst = instruction_type;
  using arg = argument_type;

  machine MachineA;
  machine MachineB;
  machine* A = &MachineA;
  machine* B = &MachineB;

  {
    *A = {};
    *B = {};

    A->Screen[0] = 1;

    MTB_Require( *A != *B );

    instruction Inst{ instruction_type::CLS };
    ExecuteInstruction(A, Inst);
    MTB_Require( *A == *B );
  }

  {
    *A = {};
    *B = {};

    instruction Inst = INST2(LD, V, 0, BYTE, 42);
    ExecuteInstruction(A, Inst);
    MTB_Require( *A != *B );
    B->V[0x0] = 42;
    MTB_Require( *A == *B );
  }

  // {
  //   u8 ROM[]
  //   {
  //     INSTRUCTION(0xF01E),
  //     INSTRUCTION(0x7142),
  //   };

  //   instruction Inst{ instruction_type::CLS };
  //   ExecuteInstruction(A, Inst);
  //   for (int Y = 0; Y < SCREEN_HEIGHT; ++Y)
  //   {
  //     for (int X = 0; X < SCREEN_WIDTH; ++X)
  //     {
  //       bool32* Pixel = A->Screen + (Y * SCREEN_WIDTH) + X;
  //       MTB_Require(*Pixel == false);
  //     }
  //   }
  // }
}

#undef INST3
#undef INST2
#undef INST1
#undef INST0
