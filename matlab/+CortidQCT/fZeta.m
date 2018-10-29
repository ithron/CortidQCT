function f = fZeta(z, Gp, Gn, mu1, mu2, mu3, sigma1, sigma2, sigma3)
%FZETA probability density functino of the gaussian random process zeta.
% f = fZeta(z, Gp, Gn, mu1, mu2, mu3, sigma1, sigma2, sigma2).
% z - value to evaluate
% Gp - G(t + w) pre-evaluated
% Gn - G(t - w) pre-evaluated
% mu1 - mean tissue density
% mu2 - mean cortex density
% mu3 - mean trabecular density
% sigma1 - tissue dentiy std
% sigma2 - cortex density std
% sigma3 - trabecular density std

% This file is part of the 'CortidQCT' project.
% Author: Stefan Reinhold
% Copyright: Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.¬
%            You may use, distribute and modify this code under the terms of¬
%            the AFL 3.0 license; see LICENSE for full license details.

mu = CortidQCT.muZeta(Gp, Gn, mu1, mu2, mu3);
sigmaSq = CortidQCT.varZeta(Gp, Gn, sigma1, sigma2, sigma3);

f = 1./sqrt(2*pi.*sigmaSq) .* exp(-0.5 * (z - mu).^2 ./ sigmaSq);

end

