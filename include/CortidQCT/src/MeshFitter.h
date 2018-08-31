#pragma once

#include "MeasurementModel.h"
#include "Mesh.h"

#include "propagate_const.h"

#include <memory>

namespace CortidQCT {

class MeshFitter {
public:

  /**
   * @brief Configuration type for MeshFitter
   */
  struct Configuration {
    /// The measurement model
    MeasurementModel model;
    /// The reference mesh
    Mesh<float> referenceMesh;
    /// Scale parameter for ARAP energy term
    double sigmaE = 2.0;
    /// Scale parameter for the latent variable s
    double sigmaS = 2.0;

    /**
     * @brief Load the configuration from a file
     * @param filename Path to the configuration file
     * @return reference to the loaded configuration object
     * @throws std::invalid_argument if the file could not be read
     */
    Configuration &loadFromFile(std::string const &filename);

    /**
     * @brief Convenience initializer to load a configuration from file
     * @see loadFromFile
     * @return Configuration object
     */
    inline static Configuration fromFile(std::string const &filename) {
      return Configuration{}.loadFromFile(filename);
    }
  };

  /**
   * @brief Constructs a MeshFitter object with given configuration
   * @param config Configuration object
   */
  MeshFitter(Configuration const &config);

  ~MeshFitter();
  MeshFitter(MeshFitter const &);
  MeshFitter(MeshFitter &&);
  MeshFitter &operator=(MeshFitter const &);
  MeshFitter &operator=(MeshFitter &&);

  /**
   * @brief Convenience constructor that reads the configuration from the given
   * configuration file
   * @param configFilename path to the configuration file
   */
  inline MeshFitter(std::string const &configFilename)
      : MeshFitter(Configuration::fromFile(configFilename)) {}

private:
  struct Impl;

  std::propagate_const<std::unique_ptr<Impl>> pImpl_;
};

} // namespace CortidQCT
