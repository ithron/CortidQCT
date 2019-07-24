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
b = b ./ st;
c = c ./ st;

h = h .* ct;
% TODO: Handle signular cases

exp_bc = exp(-b.^2 ./ (2 .* c.^2));

exp_ph = exp(-2*pi^2*c.^2.*(t + h).^2);
exp_mh = exp(-2*pi^2*c.^2.*(t - h).^2);
exp_ph2 = exp(-2*pi^2*c.^2.*(t + h/2).^2);
exp_mh2 = exp(-2*pi^2*c.^2.*(t - h/2).^2);

cos_ph = cos(2*pi*b.*(t + h));
cos_mh = cos(2*pi*b.*(t - h));
cos_ph2 = cos(2*pi*b.*(t + h/2));
cos_mh2 = cos(2*pi*b.*(t - h/2));

sin_ph = sin(2*pi*b.*(t + h));
sin_mh = sin(2*pi*b.*(t - h));
sin_ph2 = sin(2*pi*b.*(t + h/2));
sin_mh2 = sin(2*pi*b.*(t - h/2));

erfi_ph = CortidQCT.Faddeeva.erfi((b + 2i * pi * c.^2 .* (t + h)) ./ (sqrt(2) * c));
erfi_mh = CortidQCT.Faddeeva.erfi((b + 2i * pi * c.^2 .* (t - h)) ./ (sqrt(2) * c));
erfi_ph2 = CortidQCT.Faddeeva.erfi((b + 2i * pi * c.^2 .* (t + h/2)) ./ (sqrt(2) * c));
erfi_mh2 = CortidQCT.Faddeeva.erfi((b + 2i * pi * c.^2 .* (t - h/2)) ./ (sqrt(2) * c));
  
Xi_0_ph = exp_bc .* ( 1 + ( erfi_ph - conj(erfi_ph) ) ./ 2i );
Xi_0_mh = exp_bc .* ( 1 + ( erfi_mh - conj(erfi_mh) ) ./ 2i );
Xi_0_ph2 = exp_bc .* ( 1 + ( erfi_ph2 - conj(erfi_ph2) ) ./ 2i );
Xi_0_mh2 = exp_bc .* ( 1 + ( erfi_mh2 - conj(erfi_mh2) ) ./ 2i );

Xi_1_ph = -1 ./ (sqrt(2 * pi) * c) .* exp_ph .* cos_ph ./ pi + ...
  b./(2 * c.^2) .* exp(- b.^2 ./ (2 * c.^2)) .* ...
  ( erfi_ph + conj(erfi_ph) ) ./ (2 * pi);

Xi_1_mh = -1 ./ (sqrt(2 * pi) * c) .* exp_mh .* cos_mh ./ pi + ...
  b./(2 * c.^2) .* exp(- b.^2 ./ (2 * c.^2)) .* ...
  ( erfi_mh + conj(erfi_mh) ) ./ (2 * pi);

Xi_1_ph2 = -1 ./ (sqrt(2 * pi) * c) .* exp_ph2 .* cos_ph2 ./ pi + ...
  b./(2 * c.^2) .* exp(- b.^2 ./ (2 * c.^2)) .* ...
  ( erfi_ph2 + conj(erfi_ph2) ) ./ (2 * pi);

Xi_1_mh2 = -1 ./ (sqrt(2 * pi) * c) .* exp_mh2 .* cos_mh2 ./ pi + ...
  b./(2 * c.^2) .* exp(- b.^2 ./ (2 * c.^2)) .* ...
  ( erfi_mh2 + conj(erfi_mh2) ) ./ (2 * pi);
  
 
Xi_2_ph = -(t + h) ./ (sqrt(2*pi) .* c) .* exp_ph .* cos_ph ./ pi + ...
    b ./ (2 * pi * sqrt(2*pi) * c.^3) .* exp_ph .* sin_ph ./ pi - ...
    exp_bc .* (b.^2 - c.^2) ./ (4 * pi .* c.^4) .* (...
      2 + 1i * conj(erfi_ph) - 1i * erfi_ph) ./ (2 * pi);
    
Xi_2_mh = -(t - h) ./ (sqrt(2*pi) .* c) .* exp_mh .* cos_mh ./ pi + ...
    b ./ (2 * pi * sqrt(2*pi) * c.^3) .* exp_mh .* sin_mh ./ pi - ...
    exp_bc .* (b.^2 - c.^2) ./ (4 * pi .* c.^4) .* (...
      2 + 1i * conj(erfi_mh) - 1i * erfi_mh) ./ (2 * pi);
    
Xi_2_ph2 = -(t + h/2) ./ (sqrt(2*pi) .* c) .* exp_ph2 .* cos_ph2 ./ pi + ...
    b ./ (2 * pi * sqrt(2*pi) * c.^3) .* exp_ph2 .* sin_ph2 ./ pi - ...
    exp_bc .* (b.^2 - c.^2) ./ (4 * pi .* c.^4) .* (...
      2 + 1i * conj(erfi_ph2) - 1i * erfi_ph2) ./ (2 * pi);
    
Xi_2_mh2 = -(t - h/2) ./ (sqrt(2*pi) .* c) .* exp_mh2 .* cos_mh2 ./ pi + ...
    b ./ (2 * pi * sqrt(2*pi) * c.^3) .* exp_mh2 .* sin_mh2 ./ pi - ...
    exp_bc .* (b.^2 - c.^2) ./ (4 * pi .* c.^4) .* (...
      2 + 1i * conj(erfi_mh2) - 1i * erfi_mh2) ./ (2 * pi);


Upsilon_1_ph = -Xi_1_ph + t .* Xi_0_ph;
Upsilon_1_mh = -Xi_1_mh + t .* Xi_0_mh;
Upsilon_1_ph2 = -Xi_1_ph2 + t .* Xi_0_ph2;
Upsilon_1_mh2 = -Xi_1_mh2 + t .* Xi_0_mh2;

Upsilon_2_ph = Xi_2_ph - 2*t .*  Xi_1_ph + t.^2 .* Xi_0_ph;
Upsilon_2_mh = Xi_2_mh - 2*t .*  Xi_1_mh + t.^2 .* Xi_0_mh;
Upsilon_2_ph2 = Xi_2_ph2 - 2*t .*  Xi_1_ph2 + t.^2 .* Xi_0_ph2;
Upsilon_2_mh2 = Xi_2_mh2 - 2*t .*  Xi_1_mh2 + t.^2 .* Xi_0_mh2;

scale = 1 ./ (2 * sum(a .* exp(-b.^2 ./ (2 * c.^2))));

y = a .* (  1 ./ h    .* (     Xi_0_mh2      -     Xi_0_ph2      - 2 * Xi_0_mh      + 2 * Xi_0_ph ) ...
          + 4 ./ h.^2 .* (    -Upsilon_1_mh2 -     Upsilon_1_ph2 +     Upsilon_1_mh +     Upsilon_1_ph ) ...
          + 2 ./ h.^3 .* ( 2 * Upsilon_2_mh2 - 2 * Upsilon_2_ph2 -     Upsilon_2_mh +     Upsilon_2_ph ));
 
y = sum(y, 2) .* scale;

if transposeT
  y = permute(y, [2, 1, 3, 4]);
end

end

