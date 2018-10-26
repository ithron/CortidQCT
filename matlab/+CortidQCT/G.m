function y = G(t, theta, GTable)
% G Evaluates the primitive function of the angle dependend PSF g using a
% pre-computed lookup table.
%   t - evaluation points
%   theta - angle(s) with z axis in rad
%   GTable - output og genGTable

yLin = interp2(GTable.x, GTable.y, GTable.values, t(:), theta(:));

y = reshape(yLin, size(t));

clear yLin

y(t < GTable.minX) = 0;
y(t > GTable.maxX) = 1;

end

