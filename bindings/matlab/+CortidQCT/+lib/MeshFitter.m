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
    
  end
end

