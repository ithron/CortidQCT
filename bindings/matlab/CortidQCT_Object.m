classdef (Abstract) CortidQCT_Object < handle
    
    properties (Access = public)
        handle
    end
    
    properties (Hidden)
        cleanup
    end
    
    methods
        function obj = CortidQCT_Object(handle)
           obj.handle = handle;
%            obj.cleanup = onCleanup(@() obj.release);
        end
        
        function cp = copy(obj)
            fprintf('Copy\n');
        end
        
        function delete(obj)
            obj.release;
        end
    end
    
    
    methods(Access = protected)
        
        function outputArg = retain(obj)
            prepareLibrary;
            outputArg = calllib('CortidQCT', 'CQCT_retain', obj.handle);
            fprintf('Retained\n');
        end
        
        function release(obj)
            prepareLibrary;
            calllib('CortidQCT', 'CQCT_release', obj.handle);
            fprintf('Released\n');
        end
        
        function autorelease(obj)
            prepareLibrary;
            calllib('CortidQCT', 'CQCT_autorelease', obj.handle);
        end

    end
end
