function y = OTF(f, theta, sigmaG, sliceThickness)
%OTF Evaluates the OTF (optical transfer function) of the measurement system
%   y = OTF(w, thetam sigmaG, sliceThickness) - evaluates the OTF at frequencies
%   f (lp/mm). sigmaG is the width parameter of the in-plane PSF and sliceThickness
%   is the collimination width. theta is the angle (in rad) with the z-axis
%
%   If sigmaG is a signel scalar, the in-plane PSF is approcimated by a
%   gaussian of width sigmaG. The OTF is the corresponding fourier
%   transform.
%   If sigmaG is a vector of length N*3, then the in-plane OTF will be
%   approximated by a sum of N gaussians of the form
%     gi = a(1) * exp(-0.5*(f - a(2)).^2/a^3),
%   where a = [scale, mean, std] are the parameter of the i-th gaussian.

% This file is part of the 'CortidQCT' project.
% Author: Stefan Reinhold
% Copyright: Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.¬
%            You may use, distribute and modify this code under the terms of¬
%            the AFL 3.0 license; see LICENSE for full license details.

st = sin(theta);
ct = cos(theta);

h = sliceThickness;

if numel(sigmaG) > 1
  
  if mod(length(sigmaG), 3) ~= 0
    error('sigmaG must either be a scalar or a vector of length multiple of 3');
  end
  
  N = length(sigmaG) / 3;
  
  fexp = @(x, a) a(1) .* exp(-0.5*(x - a(2)).^2/a(3).^2);
  
  gi = zeros(size(f));
  scale = 0;
  for ii=1:N
    params = sigmaG((ii - 1) * 3 + 1 : ii * 3);
    gi = gi + fexp(f .* st.^2, params);
    scale = scale + fexp(0, params);
  end
  gi = gi / scale;
  
else
  
  gi = exp(-2 * pi^2 * sigmaG^2 * st.^2 .* f.^2);
  
end

go = sinc(h * ct .* f) .* sinc(0.5 * h * ct .* f).^2;

y = ...
  gi .* go;

end

