function y = go(x)
% GO evaluate the out-of-plane PSF.
%    y = go(x) - evaluates the out-of-plane PSF at x.

% This file is part of the 'CortidQCT' project.
% Author: Stefan Reinhold
% Copyright: Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.¬
%            You may use, distribute and modify this code under the terms of¬
%            the AFL 3.0 license; see LICENSE for full license details.

y0 = 2 * ( x + 1).^2;
y1 = 1 - 2 * x.^2;
y2 = 2 * (x - 1).^2;

% if ~isvector(x)
%     delta = x(1, 2) - x(1, 1);
% else
%     delta = x(2) - x(1);
% end

y = ...
    (y0 .* ( -1 < x & x <= -0.5) + ...
     y1 .* (-0.5 < x & x < 0.5) + ...
     y2 .* (0.5 <= x & x < 1));
 
% if isvector(y)
%     y = y ./ sum(y);
% else
%     y = bsxfun(@rdivide, y, sum(y, 2));
% end

% y = y / delta;

end

