
#include "vtkSmartPointer.h"
#include "vtkMultiProcessController.h"
#include "vtkTestUtilities.h"

#define rootPrint(x) \
if (vtkMultiProcessController::GetGlobalController()->GetLocalProcessId() == 0) printf(x)


int main (int argc, char * argv[])
{

  return 0;
}
