#include <CortidQCT/CortidQCT.h>

#include "CheckExtension.h"

#include <iostream>

int main(int argc, char **argv) {
  using namespace CortidQCT;

  if (argc != 3 && argc != 4) {
    std::cerr << "Usage: MeshConvert <InputMesh> [InputLabels] <OutputMesh>"
              << std::endl;
    return EXIT_FAILURE;
  }

  try {
    auto mesh = Mesh<double>{};
    if (argc == 3) {
      mesh.loadFromFile(argv[1]);
      mesh.writeToFile(argv[2]);
    } else {

      if (IO::extension(argv[2], true) == "yml" ||
          IO::extension(argv[2], true) == "yaml") {

        auto const map = ColorToLabelMaps::CustomMap::fromFile(argv[2]);
        mesh.loadFromFile(argv[1], map);
      } else {
        mesh.loadFromFile(argv[1], argv[2]);
      }

      mesh.writeToFile(argv[3]);
    }

  } catch (std::exception const &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
