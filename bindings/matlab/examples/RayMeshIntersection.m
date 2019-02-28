%% Setup
import CortidQCT.lib.Mesh

% mesh = Mesh.fromFile('../../../data/Vertebra.off');
mesh = Mesh.fromFile('/data1/sreinhold/src/CortidQCT/data/Vertebra.off');

radius = 30;
N = 300;

Origins = randn(N, 3);
Origins = radius * Origins ./ sqrt(sum(Origins.^2, 2));

Directions = randn(N, 3);
% Make directions point inwards
Directions = Directions .* sign(sum(-Directions .* Origins, 2));
% Normalize
Directions = Directions ./ sqrt(sum(Origins.^2, 2));


%% Compute ray-mesh-intersections

[UV, Idx, ~, Valid] = mesh.rayIntersections(Origins, Directions);

%% Convert results ot cartesian points

ValidPoints = mesh.barycentricToCartesian(UV(Valid, :), Idx(Valid));
Points = Inf(N, 3);
Points(Valid, :) = ValidPoints;

%% Plot
clf;
mesh.plot
axis equal off
hold on

quiver3(Origins(:, 1), Origins(:, 2), Origins(:, 3), ...
  5 * Directions(:, 1), 5 * Directions(:, 2), 5 * Directions(:, 3), 'r');

plot3([Origins(:, 1), Points(:, 1)]', [Origins(:, 2), Points(:, 2)]', ...
  [Origins(:, 3), Points(:, 3)]', 'gx-');

hold off

% Example outptu can be found in
% images/RayMeshIntersection-Example-Plot.png

