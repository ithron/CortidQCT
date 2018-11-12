classdef Mesh < CortidQCT.lib.ObjectBase
  %MESH A triangular mesh

  properties (Dependent)
    vertexCount
    triangleCount
    Vertices
    Indices
    Labels
  end

  methods

    % Constructor
    function obj = Mesh(varargin)
      import CortidQCT.lib.ObjectBase;
      
      if nargin == 0
        handle = ObjectBase.call('createMesh');
      else
        handle = varargin{1};
      end
      
      obj@CortidQCT.lib.ObjectBase(handle);
    end

    %%%%%%%%%%%%%%%%%
    %% Getters
    %

    function vertexCount = get.vertexCount(obj)
      import CortidQCT.lib.ObjectBase;
      vertexCount = ObjectBase.call('meshVertexCount', obj.handle);
    end

    function triangleCount = get.triangleCount(obj)
      import CortidQCT.lib.ObjectBase;
      triangleCount = ObjectBase.call('meshTriangleCount', obj.handle);
    end

    function Vertices = get.Vertices(obj)
      import CortidQCT.lib.ObjectBase;
      vBuffer = libpointer('singlePtr', zeros(3, obj.vertexCount, 'single'));
      result = ObjectBase.call('meshCopyVertices', obj.handle, vBuffer);
      assert(result == 4 * length(vBuffer.Value(:)), "Size mismatch");
      Vertices = vBuffer.Value';
    end

    function Indices = get.Indices(obj)
      import CortidQCT.lib.ObjectBase;
      iBuffer = libpointer('longPtr', zeros(3, obj.triangleCount, 'int64'));
      result = ObjectBase.call('meshCopyTriangles', obj.handle, iBuffer);
      assert(result == 8 * length(iBuffer.Value(:)), "Size mismatch");
      Indices = iBuffer.Value' + 1;
    end

    function Labels = get.Labels(obj)
      import CortidQCT.lib.ObjectBase;
      lBuffer = libpointer('uint32Ptr', zeros(obj.vertexCount, 1, 'uint32'));
      result = ObjectBase.call('meshCopyLabels', obj.handle, lBuffer);
      assert(result == 4 * length(lBuffer.Value(:)), "Size mismatch");
      Labels = lBuffer.Value;
    end

    %%%%%%%%%%%%%%%%%
    %% Modifyers
    %

    function success = loadFromFile(obj, meshFilename, varargin)
      import CortidQCT.lib.ObjectBase;
      import CortidQCT.lib.Error;
      
      err = Error;
      
      if nargin == 2
        result = ObjectBase.call('loadMesh', obj.handle, meshFilename, err.pointer);
      elseif (nargin == 3 && isa(varargin{1}, 'ColorToLabelMap'))
        map = varargin{1};
        result = ObjectBase.call('loadMeshWithCustomMapping', obj.handle, meshFilename, map.handle, err.pointer);
      elseif nargin == 3
        labelFilename = varargin{1};
        result = ObjectBase.call('loadMeshAndLabels', obj.handle, meshFilename, labelFilename, err.pointer);
      else
        error('Invalid number of arguments');
      end
      if result == 0
        error('Failed to load mesh: %s', err.message);
      end
      success = result == 1;
    end
    
    function success = writeToFile(obj, meshFilename, varargin)
      import CortidQCT.lib.ObjectBase;
      import CortidQCT.lib.Error;
      
      if nargin == 2
        labelsFilename = '/dev/null';
      else
        labelsFilename = varargin{1};
      end
      
      err = Error;
      
      res = ObjectBase.call('meshAndLabelsWriteToFile', obj.handle, meshFilename, labelsFilename, err.pointer);
      
      if res == 0
        error('Failed writing the mesh and label: %s', err.message);
      end
      
      success = res == 1;
      
    end
    
    function p = plot(obj, varargin)
      if nargin == 1
        ax = gca;
      else
        ax = varargin{1};
      end
      
      p = patch(ax, ...
        'Faces', obj.Indices, ...
        'Vertices', obj.Vertices, ...
        'FaceColor', 'interp', ...
        'FaceAlpha', 0.8, ...
        'CData', obj.Labels);
      
    end

  end % methods
  
  methods (Static)
    
    function mesh = fromFile(filename, varargin)
      import CortidQCT.lib.Mesh;
      mesh = Mesh;
      mesh.loadFromFile(filename, varargin{:});
    end
    
  end % Static methods
end

