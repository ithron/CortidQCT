function mu = muZeta(Gp, Gn, mu1, mu2, mu3)
%MUZETA The expected value function of the random process zeta

% This file is part of the 'CortidQCT' project.
% Author: Stefan Reinhold
% Copyright: Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.¬
%            You may use, distribute and modify this code under the terms of¬
%            the AFL 3.0 license; see LICENSE for full license details.

mu = mu1 + Gp * (mu2 - mu1) + Gn * (mu3 - mu2);

end

