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

namespace Internal {
struct PrivateStateAccessor;
}

struct Coordinate3D {
  enum class Type { absolute, relative };

  std::array<float, 3> xyz{{0, 0, 0}};
  Type type{Type::absolute};
};

/**
 * @nosubgrouping .
 */
class MeshFitter {
  class Impl;

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
    /// Calibration slope
    float calibrationSlope = 1.0f;
    /// Calibration intercept
    float calibrationIntercept = 0.0f;
    /// Ignore samples outisde the volumes?
    bool ignoreExteriorSamples = false;

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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpadded"
  /**
   * @brief Result type
   */
  struct Result {
    /// The reference mesh
    Mesh<float> referenceMesh;
    /// The deformed mesh or nullopt if fitting failed
    Mesh<float> deformedMesh;
    /// The displacement vector
    std::vector<float> displacementVector;
    /// the weght vector
    std::vector<float> weights;
    /// per-vertex normals
    std::vector<std::array<float, 3>> vertexNormals;
    /// Volume sampling positions
    std::vector<std::array<float, 3>> volumeSamplingPositions;
    /// Volume samples
    std::vector<float> volumeSamples;
    /// Minimum distacne norm
    float minDisNorm = std::numeric_limits<float>::max();
    /// Log likeihood of deformedMesh
    float logLikelihood = -std::numeric_limits<float>::max();
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation-deprecated-sync"
    /// @brief Per-vertex log likelihood vector (**deprecated**)
    /// @deprecated Deprecated since version 1.3. The the vertex normals stored
    // in `deformedMesh` instead. Will be removed in version 2.0.
    std::vector<float> perVertexLogLikelihood;
#pragma clang diagnostic pop
    /// Effective sigmaS
    float effectiveSigmaS = .0f;
    /// Iteration count
    std::size_t iteration = 1;
    /// Converged?
    bool converged = false;
    /// Successfull?
    bool success = false;
    /// Number of non-decreasing iterations
    std::size_t nonDecreasing = 0;
  };

  /// Internal State type
  struct State : public Result {
    State() = default;
    State(State const &);
    State(State &&) = default;
    State(Result const &);
    State(Result &&);
    ~State();
    State &operator=(State const &);
    State &operator=(State &&) = default;

  private:
    friend class Impl;
    friend struct Internal::PrivateStateAccessor;
    struct HiddenState;
    using HiddenStatePtr = std::unique_ptr<HiddenState>;

    /// For internal use only, do not touch!
    HiddenStatePtr hiddenState_;
  };

#pragma clang diagnostic pop

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
  Result fit(VoxelVolume const &volume) const;

  /**
   * @brief Initializes the fitting algorithm
   *
   * Initializes the state and performed the first volume sampling step by
   * calling `volumeSamplingStep()`.
   *
   * @param volume VoxelVolume object representing the target scan
   * @return State object that must be passed to subsequent calls
   * @see fit()
   */
  State init(VoxelVolume const &volume) const;

  /**
   * @brief Runs the fitting algorithm for a single iteration
   *
   * Performed the following steps:
   *  1. optimalDisplacementStep()
   *  2. optimalDeformationStep()
   *  3. logLikelihoodStep()
   *  4. convergenceTestStep()
   *  5. increase interation count
   *  6. volumeSamplingStep()
   *
   * @param[in,out] state State object returned by `init`.
   * @throw std::invalid_argument iff state was not initialized properly
   * @see fit()
   */
  void fitOneIteration(State &state) const;

  /**
   * @brief Samples the input volume at lines perpendicular to the vertices of
   * the current deformed mesh.
   *
   * @param[in,out] state Optimization state
   * @pre `state` has been initialized by calling `init()`.
   * @throw std::invalid_argument iff state was not initialized properly
   * @see fitOneIteration
   */
  void volumeSamplingStep(State &state) const;

  /**
   * @brief Finds the optimal displacements of the model and computes the
   * per-vertex weights.
   *
   * @param[in,out] state Optimization state
   * @pre `state` has been initialized by calling `init()`.
   * @throw std::invalid_argument iff state was not initialized properly
   * @see fitOneIteration
   */
  void optimalDisplacementStep(State &state) const;

  /**
   * @brief Find the deformation that minimizes the point-to-plane distance of
   * the current deformed mesh to the displaced vertices with respect to the
   * ARAP energy.
   *
   * @param[in,out] state Optimization state
   * @pre `state` has been initialized by calling `init()`.
   * @throw std::invalid_argument iff state was not initialized properly
   * @see fitOneIteration
   */
  void optimalDeformationStep(State &state) const;

  /**
   * @brief Computes the log likelihood of the current deformed mesh given the
   * input volume.
   *
   * @param[in,out] state Optimization state
   * @pre `state` has been initialized by calling `init()`.
   * @throw std::invalid_argument iff state was not initialized properly
   * @see fitOneIteration
   */
  void logLikelihoodStep(State &state) const;

  /**
   * @brief Tests if the algorithm has converged.
   *
   * @param[in,out] state Optimization state
   * @pre `state` has been initialized by calling `init()`.
   * @throw std::invalid_argument iff state was not initialized properly
   * @see fitOneIteration
   */
  void convergenceTestStep(State &state) const;

private:
#ifndef CORTIDQCT_NO_PROPAGATE_CONST
  std::propagate_const<std::unique_ptr<Impl>> pImpl_;
#else
  std::unique_ptr<Impl> pImpl_;
#endif
};

} // namespace CortidQCT
