classdef MeshFitterResult < CortidQCT.lib.ObjectBase
  
  properties (SetAccess = private)
    mesh
  end
  
  methods
    
    function obj = MeshFitterResult(handle)
      
      import CortidQCT.lib.ObjectBase;
      import CortidQCT.lib.Mesh;
      
      obj@CortidQCT.lib.ObjectBase(handle);
      
      meshHandle = ObjectBase.call('meshFitterResultMesh', obj.handle);
      obj.mesh = Mesh(meshHandle);
      obj.mesh.retain;
      
    end
    
  end
end

