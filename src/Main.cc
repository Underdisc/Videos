#include <Result.h>
#include <VarkorMain.h>

int main(int argc, char* argv[])
{
  Result result = VarkorInit("Varkor Videos", argc, argv);
  if (!result.Success())
  {
    return 0;
  }
  VarkorRun();
  VarkorPurge();
}

