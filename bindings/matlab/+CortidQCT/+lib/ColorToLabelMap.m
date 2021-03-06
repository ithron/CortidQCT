classdef ColorToLabelMap < CortidQCT.lib.ObjectBase
  %COLORTOLABELMAP Custom color to label mapping
  
  properties (Dependent)
    Map
    entryCount
  end
  
  methods
    function obj = ColorToLabelMap()
      %COLORTOLABELMAP Construct an color to label mapping
      import CortidQCT.lib.ObjectBase;
      handle = ObjectBase.call('createColorToLabelMap');
      obj@CortidQCT.lib.ObjectBase(handle);
    end
    
    function success = loadFromFile(obj,filename)
      %LOADFROMFILE load the color to label mapping from a YAML file
      import CortidQCT.lib.ObjectBase;
      import CortidQCT.lib.Error;
      
      err = Error;
      res = ObjectBase.call('loadColorToLabelMapFromFile', obj.handle, filename, err.pointer);
      
      if res ~= 0
        error('Failed to load mapping: %s', err.message);
      end
      
      success = res == 1;
    end
    
    function entryCount = get.entryCount(obj)
      import CortidQCT.lib.ObjectBase;
      entryCount = ObjectBase.call('colorToLabelMapEntryCount', obj.handle);
    end
    
    function Map = get.Map(obj)
      
      import CortidQCT.lib.ObjectBase;
      
      mapBuffer = libpointer('uint32Ptr', zeros(4, obj.entryCount, 'uint32'));
      res = ObjectBase.call('colorToLabelMapCopyEntries', obj.handle, mapBuffer);
      assert(res == 4 * 4 * obj.entryCount);
      
      Map = mapBuffer.Value';
      
    end
    
    function obj = set.Map(obj, newMap)
      
      import CortidQCT.lib.ObjectBase;
      
      if size(newMap, 2) ~= 4
        error('newMap muste be an Nx4 matrx');
      end
      
      colors = newMap(:, 1:3);
      if any(colors(:) > 255) || any(colors(:) < 0)
        error('All color values must be between 0 and 255.');
      end
      
      count = size(newMap, 1);
      newMapT = newMap';
      linMap = uint32(newMapT(:));
      
      ObjectBase.call('colorToLabelMapSetEntries', obj.handle, count, linMap);
      
    end
  end
end

