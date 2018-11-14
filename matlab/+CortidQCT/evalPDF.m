function f = evalPDF(X, mu, s, muW, sigmaW, GTable)
%EVALPDF Evaluate the 3D probability density function for the given data
% points in X.
% X is an Nx3 matrix where each row corresponds to an observation.
% First column represents the signed distance from cortex center, second column 
% angle with z-axis (in rad) and third column density.

% This file is part of the 'CortidQCT' project.
% Author: Stefan Reinhold
% Copyright: Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.¬
%            You may use, distribute and modify this code under the terms of¬
%            the AFL 3.0 license; see LICENSE for full license details.

N = size(X, 1);

if size(X, 2) ~= 3
    error('X is ought to be of size Nx3 but it is %dx%d', N, size(X, 2));
end

X_gpu = gpuArray(single(X));

    function val = toBeIntegrated(wi)
        Gp = CortidQCT.gPrimitive(X_gpu(:, 1) + wi, X_gpu(:, 2), GTable);
        Gn = CortidQCT.gPrimitive(X_gpu(:, 1) - wi, X_gpu(:, 2), GTable);
        
        val = double(gather(...
            CortidQCT.fZeta(X_gpu(:, 3), Gp, Gn, mu(1), mu(2), mu(3), s(1), s(2), s(3)))) .* ...
            double(CortidQCT.fW(wi, muW, sigmaW));
        
        clear Gp Gn
    end

f = integral(@toBeIntegrated, 0, Inf, 'ArrayValued', true, 'RelTol', 1e-1);

end

