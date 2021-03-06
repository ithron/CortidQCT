classdef Mesh < CortidQCT.lib.ObjectBase
  %MESH A triangular mesh
  
  properties (Dependent)
    vertexCount
    triangleCount
    Vertices
    Normals
    Indices
    Labels
  end
  
  methods
    
    % Constructor
    function obj = Mesh(varargin)
      import CortidQCT.lib.ObjectBase;
      import CortidQCT.lib.Error;
      
      if nargin == 0
        handle = ObjectBase.call('createMesh');
      elseif nargin == 1
        handle = varargin{1};
      elseif nargin == 2 || nargin == 3 || nargin == 4
        V = varargin{1};
        F = varargin{2};
        if nargin >= 3
          L = varargin{3};
        else
          L = zeros(size(V, 1), 1, 'uint32');
        end
        if nargin == 4
          N = varargin{4};
        else
          N = [];
        end
        
        err = Error;
        handle = ObjectBase.call('createMeshAndAllocateMemory', size(V, 1), size(F, 1), err.pointer);
        if handle == 0
          error('Failed to create mesh: %s', err.message);
        end
      end
      
      obj@CortidQCT.lib.ObjectBase(handle);
      
      if nargin == 2 || nargin == 3 || nargin == 4
        obj.Vertices = V;
        obj.Indices = F;
        obj.Labels = L;
        if not(isempty(N))
          obj.Normals = N;
        else
          obj.updateNormals();
        end
      end
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

    function obj = set.Vertices(obj, V)
      import CortidQCT.lib.ObjectBase;
      
      if size(V, 2) ~= 3 || not(isa(V, 'single'))
        error('V must be a Nx3 single matrix')
      end
      
      if size(V, 1) ~= obj.vertexCount
        error('Number of rows of V must match vertexCount')
      end
      
      ObjectBase.call('meshSetVertices', obj.handle, V');
    end

    function Normals = get.Normals(obj)
      import CortidQCT.lib.ObjectBase;
      vnBuffer = libpointer('singlePtr', zeros(3, obj.vertexCount, 'single'));
      result = ObjectBase.call('meshCopyVertexNormals', obj.handle, vnBuffer);
      assert(result == 4 * length(vnBuffer.Value(:)), "Size mismatch");
      Normals = vnBuffer.Value';
    end

    function obj = set.Normals(obj, N)
      import CortidQCT.lib.ObjectBase;
      
      if size(N, 2) ~= 3 || not(isa(N, 'single'))
        error('N must be a Nx3 single matrix')
      end
      
      if size(N, 1) ~= obj.vertexCount
        error('Number of rows of N must match vertexCount')
      end
      
      ObjectBase.call('meshSetVertexNormals', obj.handle, N');
    end
    
    function Indices = get.Indices(obj)
      import CortidQCT.lib.ObjectBase;
      iBuffer = libpointer('longPtr', zeros(3, obj.triangleCount, 'int64'));
      result = ObjectBase.call('meshCopyTriangles', obj.handle, iBuffer);
      assert(result == 8 * length(iBuffer.Value(:)), "Size mismatch");
      Indices = iBuffer.Value' + 1;
    end
    
    function obj = set.Indices(obj, F)
      import CortidQCT.lib.ObjectBase;
      
      if size(F, 2) ~= 3
        error('F must be a Mx3 single matrix')
      end
      
      if ~isa(F, 'int64')
        F = int64(F);
      end
      
      if size(F, 1) ~= obj.triangleCount
        error('Number of rows of F must match triangleCount')
      end
      
      % Convert from matlab's 1 bases indexing to C's 0 based indexing
      F = F - 1;
      
      ObjectBase.call('meshSetTriangles', obj.handle, F');
    end
    
    function Labels = get.Labels(obj)
      import CortidQCT.lib.ObjectBase;
      lBuffer = libpointer('uint32Ptr', zeros(obj.vertexCount, 1, 'uint32'));
      result = ObjectBase.call('meshCopyLabels', obj.handle, lBuffer);
      assert(result == 4 * length(lBuffer.Value(:)), "Size mismatch");
      Labels = lBuffer.Value;
    end
    
    function obj = set.Labels(obj, L)
      import CortidQCT.lib.ObjectBase;
      
      if not(isvector(L))
        error('L must be a vector');
      end
      
      if isrow(L)
        L = L';
      end
      
      if ~isa(L, 'uint32')
        F = uint32(L);
      end
      
      if numel(L) ~= obj.vertexCount
        error('Number of rows of L must match vertexCount')
      end
      
      ObjectBase.call('meshSetLabels', obj.handle, L);
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
        'VertexNormals', obj.Normals, ...
        'CData', obj.Labels);
      
    end
    
    function obj = transform(obj, T)
      % obj = transform(obj, T) - transforms the vertices of the mesh
      % according to the 4x4 affine transformation matrix T.
      VT = [obj.Vertices, ones(obj.vertexCount, 1)] * T';
      obj.Vertices = VT(:, 1:3);
    end

    function obj = upsample(obj, nTimes)
      % UPSAMPLE Upsamples the mesh using loop subdivision but without touching
      % the orginal vertices.
      %   obj = upsample(obj, nTimes)
      % Upsamples the mesh `nTimes`.

      import CortidQCT.lib.ObjectBase;

      ObjectBase.call('meshUpsample', obj.handle, nTimes);

    end

    function obj = updateNormals(obj)
      % UPDATENORMALS re-compute the per-vertex normals.
      %
      %   obj = updateNormals(obj)

      import CortidQCT.lib.ObjectBase;

      ObjectBase.call('meshUpdatePerVertexNormals', obj.handle);

    end
    
    %%%%%%%%%%%%%%%%%
    %% Queries
    %

    function meshCopy = copy(obj)
      % COPY(obj) Returns a copy of the mesh

      import CortidQCT.lib.Mesh

      meshCopy = Mesh(obj.Vertices, obj.Indices, obj.Labels, obj.Normals);

    end
    
    function points = barycentricToCartesian(obj, UV, Idx)
      % BARYCENTRICTOCARTESIAN Converts points given in barycentric coordinates
      % into cartesian coordinates.
      %   points = barycentricToCartesian(obj, UV, Idx)
      %     UV is an Nx2 matrix of barycentric coordinates, each row represents
      %       the coordinates of one point.
      %     Idx is an N-vector of triangle indices.
      %   points is an Nx3 matrix of cartesian coordinates
      
      % Use barycentricInterpolation to simplify things
      points = obj.barycentricInterpolation(UV, Idx, obj.Vertices);
      
    end
    
    function points = barycentricInterpolation(obj, UV, Idx, Attributes)
      % BARYCENTRICINTERPOLATION Interpolates per-vertex attributes for
      % arbirary points on the surface of the mesh.
      %   points = barycentricInterpolation(obj, UV, Idx, Attributes)
      %     UV is an Nx2 matrix of barycentric coordinates, each row represents
      %       the coordinates of one point.
      %     Idx is an N-vector of triangle indices.
      %   Attributes is an KxD matrix of attribute values. Each row contains a
      %     D-dimensional attribute vector. K mist equal the number of vertices
      %     in the mesh.
      %   points is an Nx3 matrix of cartesian coordinates
      
      import CortidQCT.lib.ObjectBase;
      import CortidQCT.lib.Error;
      
      if size(UV, 2) ~= 2
        error('UV must be a Nx2 matrix')
      end
      if size(UV, 1) ~= length(Idx)
        error('UV and Idx must have the same number of rows')
      end
      if size(Attributes, 1) ~= obj.vertexCount
        error('The number of rows of Attributes must match the VertexCount')
      end
      if isrow(Idx)
        Idx = Idx';
      end
      
      nDims = size(Attributes, 2);
      
      % Convert data fir API call
      Attributes = single(Attributes);
      Idx = Idx - 1;
      baryPts = arrayfun(@(u, v, i) ...
        struct('u', u, 'v', v, 'triangleIndex', i), UV(:, 1), UV(:, 2), Idx);
      outBuffer = libpointer('singlePtr', zeros(length(Idx) * nDims, 1, 'single'));
      
      err = Error;
      
      res = ObjectBase.call('meshBarycentricInterpolation', ...
        obj.handle, ...
        baryPts, length(Idx), ...
        Attributes', nDims, ...
        outBuffer, ...
        err.pointer);
      
      if res ~= 0
        error('Interpolation failed: %s', err.message);
      end
      
      points = reshape(outBuffer.Value, nDims, [])';
    end
    
    function [UV, Idx, t, varargout] = rayIntersections(obj, Origins, Directions)
      % RAYINTERSECTIONS Compute the intersections points between a set of rays
      % and the mesh.
      %
      %   [UV, Idx, t] = rayIntersecions(obj, Origins, Directions)
      %     Origins is a Nx3 matrix of ray origins
      %     Directions is a Nx3 matrix of ray directions
      %     UV is a Nx2 matrix of the barycentric coordinates of the
      %       intersections positions.
      %     Idx is a Nx1 vector containinf the triangle indices corresponding
      %       the the rows of UV
      %     t is a Nx1 vector containing the signed distances from the ray
      %       origins to the intersection points or Inf of no intersection
      %       exists
      %   [UV, Idx, t, Valid] = rayIntersecions(obj, Origins, Directions)
      %     Valid is a Nx1 logic vector containing True for all valid
      %       intersections abd False for every invalid intersection.
      
      import CortidQCT.lib.ObjectBase;
      
      % Validate input
      if size(Origins, 2) ~= 3
        error('Origins must be a Nx3 matrix');
      end
      if size(Directions, 2) ~= 3
        error('Directions must be a Nx3 matrix');
      end
      if size(Origins, 1) ~= size(Directions, 1)
        error('Origins and Directions must have the same number of rows');
      end
      
      Origins = single(Origins);
      Directions = single(Directions);
      N = size(Origins, 1);
      
      % Create input struct array
      rays = arrayfun(@(x0, y0, z0, dx, dy, dz) ...
        struct('x0', x0, 'y0', y0, 'z0', z0, 'dx', dx, 'dy', dy, 'dz', dz), ...
        Origins(:, 1), Origins(:, 2), Origins(:, 3), ...
        Directions(:, 1), Directions(:, 2), Directions(:, 3));
      
      uvPtr = libpointer('singlePtr', zeros(2 * N, 1, 'single'));
      idxPtr = libpointer('longPtr', zeros(N, 1, 'int64'));
      tPtr = libpointer('singlePtr', zeros(N, 1, 'single'));
      
      res = ObjectBase.call('meshRayIntersectionsBuffers', ...
        obj.handle, ...
        rays, N, ...
        uvPtr, idxPtr, tPtr);

      assert(res == N * (4 + 4 + 8 + 4), 'Size mismatch');
      
      % Convert output
      UV = reshape(uvPtr.Value, N, 2);
      Idx = idxPtr.Value;
      t = tPtr.Value;
      
      if nargout > 3
        varargout{1} = isfinite(t);
      end

    end

    function [vIdx, tIdx] = nRing(obj, seedVertex, n)
      % NRING returns all vertex and triagnle indices that are within the
      % n-ring neighbourhood of the seed vertex
      %
      %   [vIdx, tIdx] = nRing(obj, seedVertex, n)
      %     seedVertex - index of the seed vertex (must be a scalar)
      %     n - scalar ring number >= 0
      %     vIdx - Nx1 vector of vertex indices that are inside the n-ring
      %     tIdx = Mx1 vector of triangle indices thar are incident to the
      %     (n-1)-ring vertex set (empty for n = 0)

      toVisit = seedVertex;

      if n == 0
        tIdx = [];
        vIdx = seedVertex;
        return;
      end

      F = obj.Indices;

      for ii=1:n

        % Find all facets indicent to vertices in toVisit
        facets = find(any(F(:, 1) == toVisit, 2));
        facets = [facets; find(any(F(:, 2) == toVisit, 2));];
        facets = [facets; find(any(F(:, 3) == toVisit, 2));];
        facets = unique(facets);

        % All vertices of facets
        verts = F(facets, :);
        verts = unique(verts(:));

        vIdx = verts;
        tIdx = facets;
        toVisit = vIdx';

      end
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

