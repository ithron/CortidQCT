classdef (Abstract) Autoreleasepool < ObjectBase
  
  methods (Static)
    
    function push()
      ObjectBase.call('autoreleasePoolPush');
    end
    
    function pop()
      ObjectBase.call('autoreleasePoolPop');
    end
    
  end
end

