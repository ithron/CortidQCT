function y = OTF(f, theta, sigmaG, sliceThickness)
%OTF Evaluates the OTF (optical transfer function) of the measurement system
%   y = OTF(w, thetam sigmaG, sliceThickness) - evaluates the OTF at frequencies
%   f (lp/mm). sigmaG is the width parameter of the in-plane PSF and sliceThickness
%   is the collimination width. theta is the angle (in rad) with the z-axis

% This file is part of the 'CortidQCT' project.
% Author: Stefan Reinhold
% Copyright: Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.¬
%            You may use, distribute and modify this code under the terms of¬
%            the AFL 3.0 license; see LICENSE for full license details.

st = sin(theta);
ct = cos(theta);

h = sliceThickness;

  gi = exp(-2 * pi^2 * sigmaG^2 * st.^2 .* f.^2);
go = sinc(h * ct .* f) .* sinc(0.5 * h * ct .* f).^2;
y = ...
  gi .* go;

end

