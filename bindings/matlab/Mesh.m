classdef Mesh < CortidQCT_Object
    %MESH A triangular mesh
    
    properties (Dependent)
        vertexCount
        triangleCount
        Vertices
        Indices
    end
    
    methods
        
        function obj = Mesh()
            prepareLibrary;
            handle = calllib('CortidQCT', 'CQCT_createMesh');
            obj@CortidQCT_Object(handle);
            
        end
        
        function vertexCount = get.vertexCount(obj)
            prepareLibrary;
            
            vertexCount = calllib('CortidQCT', 'CQCT_meshVertexCount', obj.handle);
        end
        
        function triangleCount = get.triangleCount(obj)
            prepareLibrary;
            
            triangleCount = calllib('CortidQCT', 'CQCT_meshTriangleCount', obj.handle);
        end
        
        
    end
end

