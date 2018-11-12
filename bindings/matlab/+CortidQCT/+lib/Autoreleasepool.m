classdef (Abstract) Autoreleasepool < CortidQCT.lib.ObjectBase
  
  methods (Static)
    
    function push()
      import CortidQCT.lib.ObjectBase;
      ObjectBase.call('autoreleasePoolPush');
    end
    
    function pop()
      import CortidQCT.lib.ObjectBase;
      ObjectBase.call('autoreleasePoolPop');
    end
    
  end
end

