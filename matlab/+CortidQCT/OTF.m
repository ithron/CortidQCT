function y = OTF(w, theta, sigmaG, sliceThickness)
%OTF Evaluates the OTF (optical transfer function) of the measurement system
%   y = OTF(w, thetam sigmaG, sliceThickness) - evaluates the OTF at frequencies
%   w. sigmaG is the width parameter of the in-plane PSF and sliceThickness
%   is the collimination width. theta is the angle (in rad) with the z-axis

% This file is part of the 'CortidQCT' project.
% Author: Stefan Reinhold
% Copyright: Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.¬
%            You may use, distribute and modify this code under the terms of¬
%            the AFL 3.0 license; see LICENSE for full license details.

st = sin(theta);
ct = cos(theta);

h = sliceThickness;

y = ...
  sigmaG * h^2 .* ...
  exp(-2 * pi^2 * sigmaG^2 * st.^2 .* w.^2) .* ...
  sinc(h * ct .* w) .* sinc(0.5 * h * ct .* w).^2;

end

