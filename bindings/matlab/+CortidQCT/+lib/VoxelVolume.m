classdef VoxelVolume < CortidQCT.lib.ObjectBase
  %VOXELVOLUME VoxelVolume class
  
  properties (Dependent)
    size
    voxelSize
    Voxels
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
    
    function s = plot(obj, varargin)
      
      if nargin == 1
        ax = gca;
      else
        ax = varargin{1};
      end
      
      [X, Y, Z] = meshgrid(...
        (0:obj.size(1)-1) * obj.voxelSize(1), ...
        (0:obj.size(2)-1) * obj.voxelSize(2), ...
        (0:obj.size(3)-1) * obj.voxelSize(3));
      
      centers = ((obj.size - 1) .* obj.voxelSize) / 2;
      
      xslice = centers(1);
      yslice = centers(2);
      zslice = centers(3);
      
      s = slice(ax, X, Y, Z, obj.Voxels, xslice, yslice, zslice);
      set(s, 'edgecolor', 'interp');
      
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

