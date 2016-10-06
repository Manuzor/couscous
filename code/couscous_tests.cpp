
using test_proc = void(*)(machine* M);

struct test
{
  const char* Name;
  test_proc Procedure;
  test* Next;
};

static test* GlobalFirstTest;

struct impl_test_reg
{
  impl_test_reg(char const* Name, test_proc NewProc)
  {
    auto NewTest = new test{ Name, NewProc };
    NewTest->Next = Exchange(GlobalFirstTest, NewTest);
  }
};

#define REGISTER_TEST(Name) \
  void MTB_Concat(TESTPROC_, MTB_Line)(machine* M); \
  static impl_test_reg MTB_Concat(TESTREG_, MTB_Line){ Name, &MTB_Concat(TESTPROC_, MTB_Line) }; \
  auto ::MTB_Concat(TESTPROC_, MTB_Line)(machine* M) -> void

//
// ===============================================
//

REGISTER_TEST("CLS")
{
  instruction Inst{ instruction_type::CLS };
  ExecuteInstruction(M, Inst);
  for (int Y = 0; Y < M->Screen.Height; ++Y)
  {
    for (int X = 0; X < M->Screen.Width; ++X)
    {
      bool32* Pixel = M->Screen.Pixels + (Y * M->Screen.Width) + X;
      MTB_Require(*Pixel == false);
    }
  }
}
