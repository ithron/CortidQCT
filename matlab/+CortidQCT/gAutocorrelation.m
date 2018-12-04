function c = gAutocorrelation(s, t, theta, GTable)
%GAUTOCORRELATION Computes the autocorrelation coefficients of g at s and t


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

