function f = fW_gpu(w, muW, sigmaW)
%FW_CPU GPU compatible version of fW.

% This file is part of the 'CortidQCT' project.
% Author: Stefan Reinhold
% Copyright: Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.¬
%            You may use, distribute and modify this code under the terms of¬
%            the AFL 3.0 license; see LICENSE for full license details.

f = 1./(w .* sigmaW * sqrt(2*pi)) .* exp(-0.5 .* (log(abs(w)) - muW).^2 ./ sigmaW^2);


if f <= 0 || isnan(f)
    f = single(0);
end


end

