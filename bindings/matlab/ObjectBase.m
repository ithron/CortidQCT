classdef (Abstract) ObjectBase < handle
  
  properties (Access = protected)
    handle
  end
  
  methods
    function obj = ObjectBase(handle)
      obj.handle = handle;
    end
    
    function delete(obj)
      obj.release;
    end
  end
  
  methods(Access = private, Static)
    
    function prepareLib()
      if not(libisloaded('CortidQCT'))
        
        hfile = sprintf('%s%s', getenv('CortidQCT_ROOT'), '/include/CortidQCT/Matlab/CortidQCT.h');
        libfile = sprintf('%s%s',getenv('CortidQCT_ROOT'), '/lib/libCortidQCT-Matlab.so');
        
        loadlibrary(libfile, hfile, 'alias', 'CortidQCT');
      end
    end
    
  end
  
  methods(Access = protected, Static)
    
    function varargout = call(funName, varargin)
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
      ObjectBase.call('retain', obj.handle);
      fprintf('Retained\n');
    end
    
    function release(obj)
      ObjectBase.call('release', obj.handle);
      fprintf('Released\n');
    end
    
    function obj = autorelease(obj)
      ObjectBase.call('autorelease', obj.handle);
      fprintf('Autoreleased\n');
    end
    
  end
end
