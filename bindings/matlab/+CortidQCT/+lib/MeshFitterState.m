classdef MeshFitterState < CortidQCT.lib.MeshFitterResult

  properties (Dependent)
    modelSamplingPositions
    pointer
  end

  methods

    function obj = MeshFitterState(fitterHandle, volumeHandle)

      import CortidQCT.lib.ObjectBase;

      handle = ObjectBase.call('createMeshFitterState', fitterHandle, volumeHandle);

      obj@CortidQCT.lib.MeshFitterResult(handle);

    end

    function modelSamplingPositions = get.modelSamplingPositions(obj)
      import CortidQCT.lib.ObjectBase;

      N = size(obj.volumeSamples, 1);
      buffer = libpointer('singlePtr', zeros(4, N, 'single'));
      res = ObjectBase.call('meshFitterStateCopyModelSamplingPositions', obj.handle, buffer);
      assert(res == 4 * 4 * N);

      modelSamplingPositions = buffer.Value';
    end

    function pointer = get.pointer(obj)
      pointer = obj.handle;
    end

  end

end
