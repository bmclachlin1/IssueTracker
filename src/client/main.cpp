#include "ClientAppManager.h"

int main(int argc, char* argv[]) {
  ClientAppManager clientApp;

  if (!clientApp.Init(argc, argv)) {
    return EXIT_FAILURE;
  }

  clientApp.Run();

  return EXIT_SUCCESS;
}
