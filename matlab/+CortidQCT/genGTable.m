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

range = max(7.034487 * s, 2 * sliceThickness);
dx = 1e-3;
range = ceil(range / dx) * dx;
x = -range:dx:range;
th = linspace(0, pi/2, 91);

[X, Th] = meshgrid(x, th);

dirac = zeros(size(X));
dirac(X == 0) = 1 / dx;

ip = CortidQCT.gi(X, s .* sin(Th));
oop = CortidQCT.go(X ./ (sliceThickness * cos(Th))) ./ cos(Th);

ipSingIdx = any(~isfinite(ip), 2);
oopSingIdx = any(~isfinite(oop) | oop > 2/dx, 2);
ip(ipSingIdx, :) = dirac(ipSingIdx, :);
oop(oopSingIdx, :) = dirac(oopSingIdx, :) * sliceThickness;
oop = oop * dx / sliceThickness;
ip = ip * dx;

IP = fft(fftshift(ip, 2), [], 2);
OOP = fft(fftshift(oop, 2), [], 2);

Y = IP .* OOP ./ dx;

yValues = ifftshift(ifft(Y, [], 2), 2);

GTable = struct;
GTable.x = single(x);
GTable.y = single(th);
GTable.sigma = single(s);
GTable.sliceThickness = single(sliceThickness);
GTable.minX = single(min(x));
GTable.maxX = single(max(x));
GTable.values = single(yValues);
GTable.primitiveValues = single(cumsum(yValues, 2) * dx);

end

