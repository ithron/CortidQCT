function GTable = genGTable(s, sliceThickness)
% GENGTABLE Generate a lookup table that is required for fast evaluation of
% the primitive function of the angle dependent PSF g.
%    s - in-plane scale parameter
%    sliceThickness - slice thickness in mm

% This file is part of the 'CortidQCT' project.
% Author: Stefan Reinhold
% Copyright: Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.¬
%            You may use, distribute and modify this code under the terms of¬
%            the AFL 3.0 license; see LICENSE for full license details.

N = 1e4;

% range = max(norminv(1 - 1e-12) * s, sliceThickness);
range = max(7.034487 * s, 2 * sliceThickness);

x = linspace(-range, range, N + 1);
y = linspace(0, pi/2, 91);

delta = x(2) - x(1);

[X, Y] = meshgrid(x, y);

evalG = CortidQCT.g(X, Y, s, 2 * sliceThickness);

% evalG = bsxfun(@(a, b) g(a, b, s, sliceThickness), x', y);

values = cumsum(evalG, 2) * delta;

GTable = struct();
GTable.values = single(values);
GTable.x = single(x);
GTable.y = single(y);
GTable.sigma = single(s);
GTable.sliceThickness = single(sliceThickness);
GTable.minX = single(-range);
GTable.maxX = single(range);

end

