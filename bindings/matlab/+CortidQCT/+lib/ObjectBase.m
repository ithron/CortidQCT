classdef (Abstract) ObjectBase < handle
  
  properties (Access = protected)
    handle
    autoreleased = false
  end
  
  methods
    function obj = ObjectBase(handle)
      obj.handle = handle;
      obj.autoreleased = handle.isNull;
    end
    
    function delete(obj)
      if not(obj.autoreleased)
        obj.release;
      end
    end
  end
  
  methods(Access = protected, Static)
    
    function prepareLib()
      if not(libisloaded('CortidQCT'))
        
        envRoot = getenv('CortidQCT_ROOT');
        if envRoot
          rootDir = envRoot;
        else
          [rootDir, ~, ~] = fileparts(mfilename('fullpath'));
        end

        hfile = sprintf('%s/include/CortidQCT/Matlab/CortidQCT.h', rootDir);

        if ismac
          libfile = sprintf('%s/lib/libCortidQCT-Matlab.dylib', rootDir);
        elseif isunix
          libfile = sprintf('%s/lib/libCortidQCT-Matlab.so', rootDir);
        elseif ispc
          libfile = sprintf('%s/lib/CortidQCT-Matlab.dll', rootDir);
        else
          error("Unknown platform");
        end
        
        loadlibrary(libfile, hfile, 'alias', 'CortidQCT');
      end
    end
    
  end
  
  methods(Access = protected, Static)
    
    function varargout = call(funName, varargin)
      import CortidQCT.lib.ObjectBase;
      
      ObjectBase.prepareLib;
      fullFunName = sprintf('CQCT_%s', funName);
      
      if nargout > 0
        varargout{1} = calllib('CortidQCT', fullFunName, varargin{:});
      else
        calllib('CortidQCT', fullFunName, varargin{:});
      end
    end
    
    
  end
  
  methods(Access = protected)
    
    function obj = retain(obj)
      import CortidQCT.lib.ObjectBase;
      ObjectBase.call('retain', obj.handle);
    end
    
    function release(obj)
      import CortidQCT.lib.ObjectBase;
      ObjectBase.call('release', obj.handle);
    end
    
    function obj = autorelease(obj)
      import CortidQCT.lib.ObjectBase;
      ObjectBase.call('autorelease', obj.handle);
    end
    
  end
end
