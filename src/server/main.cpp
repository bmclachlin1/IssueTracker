#include "ServerAppManager.h"

int main(int argc, char* argv[]) {
  ServerAppManager serverApp;

  if (!serverApp.Init(argc, argv)) {
    return 1;
  }
  serverApp.Run();

  return 0;
}
