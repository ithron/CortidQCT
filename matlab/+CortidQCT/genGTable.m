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
dirac(X == 0) = 1;

ip = CortidQCT.gi(X, s .* sin(Th));
ip = ip ./ sum(ip, 2);
oop = CortidQCT.go(X ./ (sliceThickness * cos(Th))) ./ cos(Th);
oop = oop ./ sum(oop, 2);

ipSingIdx = any(~isfinite(ip), 2);
oopSingIdx = any(~isfinite(oop) | oop > 2/dx, 2);
ip(ipSingIdx, :) = dirac(ipSingIdx, :);
oop(oopSingIdx, :) = dirac(oopSingIdx, :);

% zero pad signals
% 2 * K + 1 = size of signal before padding
K = floor(size(ip, 2) / 2);
padding = zeros(size(ip, 1), K);

ip = [padding, ip, padding];
oop = [padding, oop, padding];

% Compute convolution using FFT

IP = fft(ifftshift(ip, 2), [], 2);
OOP = fft(ifftshift(oop, 2), [], 2);

Y = IP .* OOP;
Ysq = Y .* conj(Y);

yValues = fftshift(ifft(Y, [], 2), 2) / dx;
autoCorrValues = fftshift(ifft(Ysq, [], 2), 2);

% Remove padding
yValues = yValues(:, K + 1 : end - K);
autoCorrValues = autoCorrValues(:, K + 1 : end - K);

% Ensure the PSF is non negative. This is done by exploiting the fact that
% the PSF is monotonically decreasing for x>0. If there exists a x' so that
% the PSF(x') > PSF(x) for all x > 0, then the numeric precision is not
% sufficient at all points x >= x'. So it is safe to set all values
% PSF(abs(x) > x') to zero.
% The same applies for the autocorrelation.

yDiff = diff(yValues(:, K+1:end), [], 2);
acDiff = diff(autoCorrValues(:, K+1:end), [], 2);
for ii=1:size(yValues, 1)
  xZero = x(K+find(yDiff(ii, :) > 0, 1));
  xZeroAC = x(K+find(acDiff(ii, :) > 0, 1));
  
  if not(isempty(xZero))
    yValues(ii, abs(x) >= xZero) = 0;
  end
  if not(isempty(xZeroAC))
    autoCorrValues(ii, abs(x) >= xZeroAC) = 0;
  end
end

% Ensure symmetry
yValues(:, 1:K) = yValues(:, end:-1:K+2);
autoCorrValues(:, 1:K) = autoCorrValues(:, end:-1:K+2);

% Construct table
GTable = struct;
GTable.x = single(x);
GTable.y = single(th);
GTable.sigma = single(s);
GTable.sliceThickness = single(sliceThickness);
GTable.minX = single(min(x));
GTable.maxX = single(max(x));
GTable.values = single(yValues);
GTable.primitiveValues = single(cumsum(yValues, 2) * dx);
GTable.autocorrelationValues = single(autoCorrValues);

end

