classdef Error < ObjectBase
  %ERROR Error class
  
  properties (Dependent)
    message
    pointer
  end
  
  methods
    function obj = Error(varargin)
      %ERROR Construct an error instance given the handle
      
      ObjectBase.prepareLib;
      
      if nargin == 0
        handle = libpointer('CQCT_Error_tPtr', []);
      else
        handle = varargin{1};
      end
      
      obj@ObjectBase(handle);
      
    end
    
    function message = get.message(obj)
      if obj.handle.isNull
        message = [];
      else
        message = ObjectBase.call('errorMessage', obj.handle);
      end
    end
    
    function pointer = get.pointer(obj)
      pointer = obj.handle;
    end
    
  end
end

