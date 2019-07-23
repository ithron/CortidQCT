function y = PSF(t, theta, sigmaG, sliceThickness)
%PSF Evaluates the PSF (point spread function) of the measurement system
%   y = PSF(t, theta, sigmaG, sliceThickness) - evaluates the PSF at
%   positions t [mm]. sigmaG are the parameters for the in-plane PSF and sliceThickness
%   is the collimination width. theta is the angle (in rad) with the z-axis
%
%   If sigmaG is a scalar, the in-plane PSF is approcimated by a
%   gaussian of width sigmaG.
%   If sigmaG is a vector of length N*3, then the in-plane OTF will be
%   approximated by a sum of N gaussians of the form
%     Gi = a(1) * exp(-0.5*(f - a(2)).^2/a^3), The in-plane PSF the is the
%   corresponding inverse fourier transform.
%   Aboove a = [scale, mean, std] are the parameter of the i-th gaussian.

% This file is part of the 'CortidQCT' project.
% Author: Stefan Reinhold
% Copyright: Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.¬
%            You may use, distribute and modify this code under the terms of¬
%            the AFL 3.0 license; see LICENSE for full license details.

% TODO: Optimize code by reducing doubel computations

st = sin(theta);
ct = cos(theta);

h = sliceThickness;

if numel(sigmaG) == 1
  sigmaG = [1 0 sigmaG];
end

if iscolumn(sigmaG)
  sigmaG = sigmaG.';
end

transposeT = false;
if isrow(t)
  t = t.';
  transposeT = true;
end
  
if mod(length(sigmaG), 3) ~= 0
  error('sigmaG must either be a scalar or a vector of length multiple of 3');
end

Ni = length(sigmaG) / 3;
a = sigmaG(1:3:3*Ni);
b = sigmaG(2:3:3*Ni);
c = sigmaG(3:3:3*Ni);

% Apply strechting due to angle with z-axis
b = b ./ sin(theta);
c = c ./ sin(theta);

h = h .* cos(theta);
% TODO: Handle signular cases

Xi_0 = @(t) exp(- b.^2 ./ (2 * c.^2)) .* ( 1 + ...
  ( erfi(1i * (2 * pi * c.^2 .* t - 1i * b) ./ (sqrt(2) * c)) + ...
    erfi(1i * (2 * pi * c.^2 .* t + 1i * b) ./ (sqrt(2) * c)) ) ./ 2i );
  
Xi_1 = @(t) ...
  -1 ./ (sqrt(2 * pi) * c) .* ...
  ( exp( -2 * pi * t .* (pi * c.^2 .* t + 1i * b)) + ...
    exp( -2 * pi * t .* (pi * c.^2 .* t - 1i * b)) ) ./ (2 * pi) + ...
  b./(2 * c.^2) .* exp(- b.^2 ./ (2 * c.^2)) .* ...
  ( erfi((b - 2i * pi * c.^2 .* t) ./ (sqrt(2) * c)) + ...
    erfi((b + 2i * pi * c.^2 .* t) ./ (sqrt(2) * c)) ) ./ (2 * pi);
  
  
 Xi_2 = @(t) ...
  exp(-b.^2./(2*c.^2)) .* (...
    -t ./ (sqrt(2*pi) .* c) .* (...
      exp((b + 2i * pi * c.^2 .* t).^2 ./ (2 .* c.^2)) + ...
      exp((b - 2i * pi * c.^2 .* t).^2 ./ (2 .* c.^2)) ...
    ) / (2 * pi) - ...
    1i * b ./ (2 * pi * sqrt(2*pi) * c.^3) .* (...
      exp((b + 2i * pi * c.^2 .* t).^2 ./ (2 .* c.^2)) - ...
      exp((b - 2i * pi * c.^2 .* t).^2 ./ (2 .* c.^2)) ...
    ) ./ (2 * pi) - ...
    (b.^2 - c.^2) ./ (4 * pi .* c.^4) .* (...
      2 + ...
      1i * erfi((b - 2i * pi * c.^2 .* t) ./ (sqrt(2) .* c)) - ...
      1i * erfi((b + 2i * pi * c.^2 .* t) ./ (sqrt(2) .* c)) ...
   ) ./ (2 * pi));


Upsilon_0 = @(t, s)                                             Xi_0(t + s);
Upsilon_1 = @(t, s)                      -Xi_1(t + s) + t    .* Xi_0(t + s);
Upsilon_2 = @(t, s) Xi_2(t + s) - 2*t .*  Xi_1(t + s) + t.^2 .* Xi_0(t + s);

scale = 1 ./ (2 * sum(a .* exp(-b.^2 ./ (2 * c.^2))));

y = a .* (  1 ./ h    .* (     Upsilon_0(t, -h/2) -     Upsilon_0(t, h/2) - 2 * Upsilon_0(t, -h) + 2 * Upsilon_0(t, h) ) ...
          + 4 ./ h.^2 .* (    -Upsilon_1(t, -h/2) -     Upsilon_1(t, h/2) +     Upsilon_1(t, -h) +     Upsilon_1(t, h) ) ...
          + 2 ./ h.^3 .* ( 2 * Upsilon_2(t, -h/2) - 2 * Upsilon_2(t, h/2) -     Upsilon_2(t, -h) +     Upsilon_2(t, h) ));
 
y = sum(y, 2) .* scale;

if transposeT
  y = permute(y, [2, 1, 3, 4]);
end

end

