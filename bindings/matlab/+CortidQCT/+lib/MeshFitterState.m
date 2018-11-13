classdef MeshFitterState < CortidQCT.lib.MeshFitterResult

  properties (Dependent)
    pointer
  end

  methods

    function obj = MeshFitterState(fitterHandle, volumeHandle)

      import CortidQCT.lib.ObjectBase;

      handle = ObjectBase.call('createMeshFitterState', fitterHandle, volumeHandle);

      obj@CortidQCT.lib.MeshFitterResult(handle);

    end

    function pointer = get.pointer(obj)
      pointer = obj.handle;
    end

  end

end
