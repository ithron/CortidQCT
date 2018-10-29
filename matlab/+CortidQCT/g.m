function y = g(t, theta, sigma, sliceThickness)
% G evaluates the angle dependent point spread function.
% y = g(t, theta, sigma, sliceThickness).
%   t - evaluation points
%   theta - angle(s) with z-axis in rad
%   sigma - scale parameter of in-plane PSF
%   sliceThickness - slice thickness in mm

% This file is part of the 'CortidQCT' project.
% Author: Stefan Reinhold
% Copyright: Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.¬
%            You may use, distribute and modify this code under the terms of¬
%            the AFL 3.0 license; see LICENSE for full license details.

if isvector(t)
    delta = t(2) - t(1);
else
    delta = t(1, 2) - t(1, 1);
end

theta = mod(abs(theta), pi);

theta(theta > pi/2) = pi - theta(theta > pi/2);

inPlane = CortidQCT.gi(t, sigma .* sin(theta));
outOfPlane = CortidQCT.go(t ./ (sliceThickness * cos(theta))) ./ cos(theta);

if isvector(outOfPlane)
    outOfPlane = outOfPlane / (sum(outOfPlane));
else
    outOfPlane = bsxfun(@rdivide, outOfPlane, sum(outOfPlane, 2));
end
  

dirac = zeros(1, size(t, 2));
dirac((length(dirac) - 1) / 2 + 1) = 1;

inPlane(any(~isfinite(inPlane), 2), :) = repmat(dirac, sum(any(~isfinite(inPlane), 2)), 1);
outOfPlane(any(~isfinite(outOfPlane), 2), :) = repmat(dirac, sum(any(~isfinite(outOfPlane), 2)), 1);

Ip = fft(inPlane, [], 2);
Oop = fft(outOfPlane, [], 2);

Y = Ip .* Oop;

y = ifftshift(ifft(Y, [], 2), 2);

y = bsxfun(@rdivide, y, sum(y, 2)) / delta;

end

