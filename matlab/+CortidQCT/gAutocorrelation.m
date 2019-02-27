function c = gAutocorrelation(s, t, theta, GTable)
%GAUTOCORRELATION Computes the autocorrelation coefficients of g at s and t

% This file is part of the 'CortidQCT' project.
% Author: Stefan Reinhold
% Copyright: Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.¬
%            You may use, distribute and modify this code under the terms of¬
%            the AFL 3.0 license; see LICENSE for full license details.

if isvector(s) && isvector(t)
  if isrow(s)
    s = s';
  end
  
  if iscolumn(t)
    t = t';
  end
end

delta = s - t;

cLin = interp2(GTable.x, GTable.y, GTable.autocorrelationValues, delta(:), theta(:), 'linear', 0);

c = reshape(cLin, size(delta));
cT = c';

c(delta < 0) = cT(delta < 0);

end

