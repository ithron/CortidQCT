function [resultMesh, volume] = CortidQCT_cli(configFilename, volumeFilename, outputMeshFilename, varargin)
%CORTIDQCT-CLI Identifies the cortical shape of the given volume and writes
%the output to the given file

% Load volume
volume = VoxelVolume.fromFile(volumeFilename);

% Create MeshFitter using config file
fitter = MeshFitter(configFilename);

% fit
result = fitter.fit(volume);

% write output
result.mesh.writeToFile(outputMeshFilename, varargin{:});

% plot results
volume.plot;
h = result.mesh.plot;
axis equal
colormap gray

h.FaceColor = 'g';
h.FaceAlpha = 0.3;

resultMesh = result.mesh;


end

