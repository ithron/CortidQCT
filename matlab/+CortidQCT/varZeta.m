function sigmaSq = varZeta(Gp, Gn, sigma1, sigma2, sigma3)
%VARZETA The variance function of the random process zeta

% This file is part of the 'CortidQCT' project.
% Author: Stefan Reinhold
% Copyright: Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.¬
%            You may use, distribute and modify this code under the terms of¬
%            the AFL 3.0 license; see LICENSE for full license details.

sigmaSq = (1 - Gp).^2 * sigma1.^2 + (Gp - Gn).^2 * sigma2.^2 + Gn.^2 * sigma3.^2;

end

