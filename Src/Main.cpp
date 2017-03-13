#include <stdio.h>
#include <string.h>

#include "TrussFEM.h"

static void printUsage()
{
	printf("TrussFEM usage:  \n");
  printf("TrussFEM.exe materialPath nodePath elementPath boundaryConditionsPath outFilePath\n");
}

int main(int argc, char *argv[])
{
  if (argc != 6) {
    printf("Expected 5 arguments. Only %d supplied.\n", argc - 1);
    printUsage();
    return -1;
  }

  // Try constructing the FEM problem from the data supplied. Throw an error if there is a problem.
  try
  {
    TFM::TrussFEMProblem problem(argv[1], argv[2], argv[3], argv[4], argv[5]);
  }
  catch (std::runtime_error &error)
  {
    printf((std::string(error.what())+"\n").c_str());
    printUsage();
    return -1;
  }

  return 0;
}