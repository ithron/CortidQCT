#pragma once

#include "MeasurementModel.h"
#include "Mesh.h"
#include "VoxelVolume.h"

#include "Optional.h"
#include "propagate_const.h"

#include <array>
#include <memory>
#include <variant>

namespace CortidQCT {

struct Coordinate3D {
  enum class Type { absolute, relative };

  std::array<float, 3> xyz{{0, 0, 0}};
  Type type{Type::absolute};
};

/**
 * @nosubgrouping .
 */
class MeshFitter {
public:
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
  /**
   * @brief Configuration type for MeshFitter
   */
  struct Configuration {
    enum class OriginType { untouched, centered };
    using Origin = std::variant<OriginType, Coordinate3D>;
    using ScaleVector = std::array<float, 3>;

    using RotationVector = std::array<float, 3>;

    /// The measurement model
    MeasurementModel model;
    /// The reference mesh
    Mesh<float> referenceMesh;
    /// Scale parameter for ARAP energy term
    double sigmaE = 5.4;
    /// Scale parameter for the latent variable s
    double sigmaS = 2.0;
    /// Maximum number of iterations
    std::size_t maxIterations = 100;
    /// Minimum number of non decreasing iterations before starting decay
    std::size_t minNonDecreasing = 10;
    /// Decay factor
    float decay = 0.9f;

    /**
     * @brief Reference mesh origin
     *
     * There are N possible values:
     *   - `OriginType::untouched` - do not change the origin of the mesh
     *   - `OriginType::centered` - use the image center as the origin
     *   - `absolute` - override the origin using absolute coordinates
     *   - `relative` - override the origin using relative coordinates
     */
    Origin referenceMeshOrigin = OriginType::untouched;

    /**
     * @brief Reference mesh scale factor
     */
    ScaleVector referenceMeshScale = {{1.f, 1.f, 1.f}};

    /**
     * @brief Reference mesh rotation angled.
     *
     * Rotation is performed in this order: z, y, x. Defaults to 0.
     */
    RotationVector referenceMeshRotation = {{0.f, 0.f, 0.f}};

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

    /**
     * @brief Returns a tranlation vector that translates the centroid of the
     * reference mesh into the centroid as specified in the configuration
     *
     * @param volume voxel volume used for the computation of 'centered' or
     * 'relatice' originsl
     * @return  (x, y, z) translation vector
     */
    std::array<float, 3> meshTranslation(VoxelVolume const &volume) const;
  };
#pragma clang diagnostic pop

  /**
   * @brief Result type
   */
  struct Result {
    /// The deformed mesh or nullopt if fitting failed
    std::optional<Mesh<float>> deformedMesh;
  };

  /**
   * @name Public Properties
   * @{
   */

  /// The configuration
  Configuration configuration;

  /// @}

  /**
   * @brief Constructs a MeshFitter object with given configuration
   * @param config Configuration object
   */
  MeshFitter(Configuration config);

  ~MeshFitter();
  MeshFitter(MeshFitter const &);
  MeshFitter(MeshFitter &&) noexcept;
  MeshFitter &operator=(MeshFitter const &);
  MeshFitter &operator=(MeshFitter &&) noexcept;

  void swap(MeshFitter &rhs) noexcept;

  /**
   * @brief Convenience constructor that reads the configuration from the given
   * configuration file
   * @param configFilename path to the configuration file
   */
  inline MeshFitter(std::string const &configFilename)
      : MeshFitter(Configuration::fromFile(configFilename)) {}

  /**
   * @brief Fits the reference mesh to the given voxel volume
   *
   * @param volume VoxelVolume object representing the target scan
   * @return A `Result` struct containing the deformed mesh
   */
  Result fit(VoxelVolume const &volume);

private:
  class Impl;

#ifndef CORTIDQCT_NO_PROPAGATE_CONST
  std::propagate_const<std::unique_ptr<Impl>> pImpl_;
#else
  std::unique_ptr<Impl> pImpl_;
#endif
};

} // namespace CortidQCT
