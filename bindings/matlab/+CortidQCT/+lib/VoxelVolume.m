classdef VoxelVolume < CortidQCT.lib.ObjectBase
  %VOXELVOLUME VoxelVolume class
  
  properties (Dependent)
    size
    voxelSize
    Voxels
    pointer
  end
  
  methods
    function obj = VoxelVolume()
      %VOXELVOLUME Construct an empty voxel volume
      
      import CortidQCT.lib.ObjectBase;
      
      handle = ObjectBase.call('createVoxelVolume');
      obj@CortidQCT.lib.ObjectBase(handle);
    end
    
    function success = loadFromFile(obj, filename)
      %LOADFROMFILE load the voxel volume from file
      
      import CortidQCT.lib.ObjectBase;
      import CortidQCT.lib.Error;
      
      err = Error;
      res = ObjectBase.call('voxelVolumeLoadFromFile', obj.handle, filename, err.pointer);
      if res ~= 1
        error('Failed loading voxel volume: %s', err.message);
      end
      
      success = res == 1;
    end

    function obj = calibrate(obj, slop, intercept)
      
      import CortidQCT.lib.ObjectBase;
      
      ObjectBase.call('voxelVolumeCalibrate', obj.handle, slope, intercept);
    end
    
    function size = get.size(obj)
      
      import CortidQCT.lib.ObjectBase;
      
      w = ObjectBase.call('voxelVolumeWidth', obj.handle);
      h = ObjectBase.call('voxelVolumeHeight', obj.handle);
      d = ObjectBase.call('voxelVolumeDepth', obj.handle);
      
      size = [w h d];
    end
    
    function voxelSize = get.voxelSize(obj)
      
      import CortidQCT.lib.ObjectBase;
      
      w = ObjectBase.call('voxelVolumeVoxelWidth', obj.handle);
      h = ObjectBase.call('voxelVolumeVoxelHeight', obj.handle);
      d = ObjectBase.call('voxelVolumeVoxelDepth', obj.handle);
      
      voxelSize = [w h d];
    end
    
    function Voxels = get.Voxels(obj)
      
      import CortidQCT.lib.ObjectBase;
      
      s = obj.size;
      vBuffer = libpointer('singlePtr', zeros(s(1), s(2), s(3), 'single'));
      res = ObjectBase.call('voxelVolumeCopyVoxels', obj.handle, vBuffer);
      assert(res == prod(s) * 4);
      
      Voxels = permute(reshape(vBuffer.Value, s(1), s(2), s(3)), [2, 1, 3]);
      
    end
    
    function s = plot(obj)

      import CortidQCT.lib.image3.*;
      
      vs = obj.voxelSize;
      T = [0 vs(1) 0 -vs(2); vs(2), 0, 0, -vs(1); 0, 0, vs(3), -vs(3)];
      centers = floor(obj.size() / 2);

      Vox = obj.Voxels;
      
      h1 = slice3(Vox, T, 1, centers(1));
      h2 = slice3(Vox, T, 2, centers(2));
      h3 = slice3(Vox, T, 3, centers(3));
      s = [h1, h2, h3];
      axis equal;
      view(3);
      
    end
    
    function pointer = get.pointer(obj)
      pointer = obj.handle;
    end
    
  end
  
  methods (Static)
    
    function obj = fromFile(filename)
      
      import CortidQCT.lib.VoxelVolume;
      
      obj = VoxelVolume;
      obj.loadFromFile(filename);
      
    end
    
  end
end

