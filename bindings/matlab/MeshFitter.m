classdef MeshFitter < ObjectBase
  %MESHFITTER MeshFitter class
  
  methods
    function obj = MeshFitter(configFile)
      %MESHFITTER Constructs a MeshFitter object usign the given
      %configuration file
      
      err = Error;
      handle = ObjectBase.call('createMeshFitter', configFile, err.pointer);
      
      if handle.isNull
        error('Failed to create MeshFitter: %s', err.message);
      end
      
      obj@ObjectBase(handle);
      
    end
    
    function result = fit(obj, volume)
      
      if not(isa(volume, 'VoxelVolume'))
        error('volume must be a VoxelVolume object');
      end
      
      resultHandle = ObjectBase.call('meshFitterFit', obj.handle, volume.handle);
      
      result = MeshFitterResult(resultHandle).retain;
      
    end
    
  end
end

