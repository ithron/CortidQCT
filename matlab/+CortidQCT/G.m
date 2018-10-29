function y = G(t, theta, GTable)
% G Evaluates the primitive function of the angle dependend PSF g using a
% pre-computed lookup table.
%   t - evaluation points
%   theta - angle(s) with z axis in rad
%   GTable - output og genGTable

% This file is part of the 'CortidQCT' project.
% Author: Stefan Reinhold
% Copyright: Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.¬
%            You may use, distribute and modify this code under the terms of¬
%            the AFL 3.0 license; see LICENSE for full license details.

yLin = interp2(GTable.x, GTable.y, GTable.values, t(:), theta(:));

y = reshape(yLin, size(t));

clear yLin

y(t < GTable.minX) = 0;
y(t > GTable.maxX) = 1;

end

