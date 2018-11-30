function y = gi(x, s)
% GI evaluates the in-plane PSF
%     y = gi(x, sigma) = evaluate the in-plane PSF at x given the scale
%     parameter sigma

% This file is part of the 'CortidQCT' project.
% Author: Stefan Reinhold
% Copyright: Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.¬
%            You may use, distribute and modify this code under the terms of¬
%            the AFL 3.0 license; see LICENSE for full license details.

% 
% if ~isvector(x)
%     delta = x(1, 2) - x(1, 1);
% else
%     delta = x(2) - x(1);
% end

sSq = s.^2;

y = 1./sqrt(2*pi*sSq) .* exp(-0.5 * x.^2 ./ sSq);

% if isvector(y)
%     y = y ./ sum(y);
% else
%     y = bsxfun(@rdivide, y, sum(y, 2));
% end

% y = y / delta;

end

