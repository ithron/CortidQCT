classdef MeshFitterResult < ObjectBase
  
  properties (SetAccess = private)
    mesh
  end
  
  methods
    
    function obj = MeshFitterResult(handle)
      obj@ObjectBase(handle);
      
      meshHandle = ObjectBase.call('meshFitterResultMesh', obj.handle);
      obj.mesh = Mesh(meshHandle);
      obj.mesh.retain;
      
    end
    
  end
end

