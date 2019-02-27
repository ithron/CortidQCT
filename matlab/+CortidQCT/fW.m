function f = fW(w, muW, sigmaW)
%FW Probability density function of log-normal distributed random variable
%W with mean muW and standard deviation sigmaW.

% This file is part of the 'CortidQCT' project.
% Author: Stefan Reinhold
% Copyright: Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.¬
%            You may use, distribute and modify this code under the terms of¬
%            the AFL 3.0 license; see LICENSE for full license details.

f = 1./(w .* sigmaW * sqrt(2*pi)) .* exp(-0.5 .* (log(abs(w)) - muW).^2 ./ sigmaW^2);

f(f <= 0 | isnan(f)) = 0;

end

