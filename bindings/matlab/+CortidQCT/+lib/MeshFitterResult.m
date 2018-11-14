classdef MeshFitterResult < CortidQCT.lib.ObjectBase
  
  properties (SetAccess = private)
    mesh
    referenceMesh
  end

  properties (Dependent)
    displacementVector
    weights
    vertexNormals
    volumeSamplingPositions
    volumeSamples
    minimumDisplacementNorm
    logLikelihood
    iteration
    converged
    success
    nonDecreasingCount
  end
  
  methods
    
    function obj = MeshFitterResult(handle)
      
      import CortidQCT.lib.ObjectBase;
      import CortidQCT.lib.Mesh;
      
      obj@CortidQCT.lib.ObjectBase(handle);
      
      meshHandle = ObjectBase.call('meshFitterResultMesh', obj.handle);
      obj.mesh = Mesh(meshHandle);
      obj.mesh.retain;

      refMeshHandle = ObjectBase.call('meshFitterResultReferenceMesh', obj.handle);
      obj.referenceMesh = Mesh(refMeshHandle);
      obj.referenceMesh.retain;
      
    end

    function displacementVector = get.displacementVector(obj)
      import CortidQCT.lib.ObjectBase;

      buffer = libpointer('singlePtr', zeros(obj.mesh.vertexCount, 1, 'single'));
      result = ObjectBase.call('meshFitterResultCopyDisplacementVector', obj.handle, buffer);
      assert(result == 4 * obj.mesh.vertexCount, "Size mismatch");
      displacementVector = buffer.Value;
    end

    function weights = get.weights(obj)
      import CortidQCT.lib.ObjectBase;

      buffer = libpointer('singlePtr', zeros(obj.mesh.vertexCount, 1, 'single'));
      result = ObjectBase.call('meshFitterResultCopyWeights', obj.handle, buffer);
      assert(result == 4 * obj.mesh.vertexCount, "Size mismatch");
      weights = buffer.Value;
    end

    function vertexNormals = get.vertexNormals(obj)
      import CortidQCT.lib.ObjectBase;

      buffer = libpointer('singlePtr', zeros(3, obj.mesh.vertexCount, 'single'));
      result = ObjectBase.call('meshFitterResultCopyVertexNormals', obj.handle, buffer);
      assert(result == 3 * 4 * obj.mesh.vertexCount, "Size mismatch");
      vertexNormals = buffer.Value';
    end

    function volumeSamplingPositions = get.volumeSamplingPositions(obj)
      import CortidQCT.lib.ObjectBase;

      posCount = ObjectBase.call('meshFitterResultVolumeSamplingPositionsCount', obj.handle);
      buffer = libpointer('singlePtr', zeros(3, posCount, 'single'));
      result = ObjectBase.call('meshFitterResultCopyVolumeSamplingPositions', obj.handle, buffer);
      assert(result == 3 * 4 * posCount, "Size mismatch");
      volumeSamplingPositions = buffer.Value';
    end

    function volumeSamples = get.volumeSamples(obj)
      import CortidQCT.lib.ObjectBase;

      posCount = ObjectBase.call('meshFitterResultVolumeSamplingPositionsCount', obj.handle);
      buffer = libpointer('singlePtr', zeros(posCount, 1, 'single'));
      result = ObjectBase.call('meshFitterResultCopyVolumeSamples', obj.handle, buffer);
      assert(result == 4 * posCount, "Size mismatch");
      volumeSamples = buffer.Value;
    end

    function minimumDisplacementNorm = get.minimumDisplacementNorm(obj)
      import CortidQCT.lib.ObjectBase;
      minimumDisplacementNorm = ObjectBase.call('meshFitterResultMinimumDisplacementNorm', obj.handle);
    end

    function logLikelihood = get.logLikelihood(obj)
      import CortidQCT.lib.ObjectBase;
      logLikelihood = ObjectBase.call('meshFitterResultLogLikelihood', obj.handle);
    end

    function iteration = get.iteration(obj)
      import CortidQCT.lib.ObjectBase;
      iteration = ObjectBase.call('meshFitterResultIterationCount', obj.handle);
    end

    function converged = get.converged(obj)
      import CortidQCT.lib.ObjectBase;
      converged = ObjectBase.call('meshFitterResultHasConverged', obj.handle) == 1;
    end

    function success = get.success(obj)
      import CortidQCT.lib.ObjectBase;
      success = ObjectBase.call('meshFitterResultSuccess', obj.handle) == 1;
    end

    function nonDecreasingCount = get.nonDecreasingCount(obj)
      import CortidQCT.lib.ObjectBase;
      nonDecreasingCount = ObjectBase.call('meshFitterResultNonDecreasingCount', obj.handle);
    end
    
  end
end

