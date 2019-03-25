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
    perVertexLogLikelihood
    effectiveSigmaS
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

    function obj = set.displacementVector(obj, vec)
      import CortidQCT.lib.ObjectBase;


      if not(isvector(vec))
        error('vec must be a single vector');
      end
      
      if isrow(vec)
        vec = vec';
      end

      if not(isa(vec, 'single'))
        vec = single(vec);
      end

      ObjectBase.call('meshFitterResultSetDisplacementVector', obj.handle, vec);
    end

    function weights = get.weights(obj)
      import CortidQCT.lib.ObjectBase;

      buffer = libpointer('singlePtr', zeros(obj.mesh.vertexCount, 1, 'single'));
      result = ObjectBase.call('meshFitterResultCopyWeights', obj.handle, buffer);
      assert(result == 4 * obj.mesh.vertexCount, "Size mismatch");
      weights = buffer.Value;
    end

    function obj = set.weights(obj, vec)
      import CortidQCT.lib.ObjectBase;

      if not(isvector(vec))
        error('vec must be a single vector');
      end
      
      if isrow(vec)
        vec = vec';
      end

      if not(isa(vec, 'single'))
        vec = single(vec);
      end

      ObjectBase.call('meshFitterResultSetWeights', obj.handle, vec);
    end

    function vertexNormals = get.vertexNormals(obj)
      % VERTEXNORMALS Returns the vertex normals of the deformed mesh
      %   vertexNormals = get.vertexNormals(obj)
      % DEPRECATED: Will be removed in version 2.0. Use `mesh.Normals`
      % instead.
      import CortidQCT.lib.ObjectBase;

      warning('MeshFitterResult.vertexNormals is deprecated and will be removed in version 2.0. Use mesh.Normals instead.');

      vertexNormals = mesh.Normals;
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

    function perVertexLogLikelihood = get.perVertexLogLikelihood(obj)
      import CortidQCT.lib.ObjectBase;
      buffer = libpointer('singlePtr', zeros(obj.mesh.vertexCount, 1, 'single'));
      res = ObjectBase.call('meshFitterResultCopyPerVertexLogLikelihood', obj.handle, buffer);
      assert(res == 4 * obj.mesh.vertexCount, 'Size mismatch');

      perVertexLogLikelihood = buffer.Value;
    end

    function obj = set.logLikelihood(obj, ll)
      import CortidQCT.lib.ObjectBase;
      if not(isa(ll, 'single'))
        ll = single(ll);
      end
      if length(ll) ~= 1
        error('ll must be a number');
      end
      ObjectBase.call('meshFitterResultSetLogLikelihood', obj.handle, ll);
    end

    function effectiveSigmaS = get.effectiveSigmaS(obj)
      import CortidQCT.lib.ObjectBase;
      effectiveSigmaS = ObjectBase.call('meshFitterResultEffectiveSigmaS', obj.handle);
    end

    function iteration = get.iteration(obj)
      import CortidQCT.lib.ObjectBase;
      iteration = ObjectBase.call('meshFitterResultIterationCount', obj.handle);
    end

    function obj = set.iteration(obj, iter)
      import CortidQCT.lib.ObjectBase;
      if length(iter) ~= 1
        error('iter must be a number');
      end
      ObjectBase.call('meshFitterResultSetIterationCount', obj.handle, iter);
    end

    function converged = get.converged(obj)
      import CortidQCT.lib.ObjectBase;
      converged = ObjectBase.call('meshFitterResultHasConverged', obj.handle) == 1;
    end

    function obj = set.converged(obj, count)
      import CortidQCT.lib.ObjectBase;
      if length(count) ~= 1
        error('count must be a number');
      end
      count = logical(count);

      ObjectBase.call('meshFitterResultSetHasConverged', obj.handle, count);
    end

    function success = get.success(obj)
      import CortidQCT.lib.ObjectBase;
      success = ObjectBase.call('meshFitterResultSuccess', obj.handle) == 1;
    end

    function obj = set.success(obj, success)
      import CortidQCT.lib.ObjectBase;
      if length(success) ~= 1
        error('success must be a number');
      end
      success = logical(success);
      ObjectBase.call('meshFitterResultSetSuccess', obj.handle, success);
    end

    function nonDecreasingCount = get.nonDecreasingCount(obj)
      import CortidQCT.lib.ObjectBase;
      nonDecreasingCount = ObjectBase.call('meshFitterResultNonDecreasingCount', obj.handle);
    end

    function obj = set.nonDecreasingCount(obj, count)
      import CortidQCT.lib.ObjectBase;
      if length(count) ~= 1
        error('count must be a number');
      end
      ObjectBase.call('meshFitterResultSetNonDecreasingCount', obj.handle, count);
    end
    
  end
end

