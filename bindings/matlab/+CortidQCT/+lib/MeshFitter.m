classdef MeshFitter < CortidQCT.lib.ObjectBase
  %MESHFITTER MeshFitter class
  
  methods
    function obj = MeshFitter(configFile)
      %MESHFITTER Constructs a MeshFitter object usign the given
      %configuration file
      
      import CortidQCT.lib.ObjectBase;
      import CortidQCT.lib.Error;
      
      err = Error;
      handle = ObjectBase.call('createMeshFitter', configFile, err.pointer);
      
      if handle.isNull
        error('Failed to create MeshFitter: %s', err.message);
      end
      
      obj@CortidQCT.lib.ObjectBase(handle);
      
    end
    
    function result = fit(obj, volume)
      
      import CortidQCT.lib.ObjectBase;
      import CortidQCT.lib.MeshFitterResult;
      
      if not(isa(volume, 'CortidQCT.lib.VoxelVolume'))
        error('volume must be a VoxelVolume object');
      end
      
      resultHandle = ObjectBase.call('meshFitterFit', obj.handle, volume.handle);
      
      result = MeshFitterResult(resultHandle).retain;
      
    end

    function state = init(obj, volume)
      import CortidQCT.lib.MeshFitterState;
      
      if not(isa(volume, 'CortidQCT.lib.VoxelVolume'))
        error('volume must be an instance of VoxelVolume')
      end

      state = MeshFitterState(obj.handle, volume.pointer);
    end

    function success = fitOneIteration(obj, state)
      if not(isa(state, 'CortidQCT.lib.MeshFitterState'))
        error('State must be an instance of MeshFitterState. Call init() first!');
      end

      import CortidQCT.lib.ObjectBase;
      import CortidQCT.lib.Error;

      err = Error;

      res = ObjectBase.call('meshFitterFitOneIteration', obj.handle, state.pointer);

      if res == 0
        error('fitOneIteration failed: %s', err.message);
        success = false;
      else
        success = true;
      end
    end
    
  end
end

