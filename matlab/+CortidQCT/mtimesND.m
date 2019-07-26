function C = mtimesND(A, B)
% MTIMESND Computes the matrix product of the (up to) 5D arrays A and B
%  C = mtimesND(A, B)
% A - LxMx...x... array
% B - MxNx...x... array
% C - LxNx...x... array representing the matrix product of A and B using
% broadcasting.

% This file is part of the 'CortidQCT' project.
% Author: Stefan Reinhold
% Copyright: Copyright (C) 2019 Stefan Reinhold  -- All Rights Reserved.¬
%            You may use, distribute and modify this code under the terms of¬
%            the AFL 3.0 license; see LICENSE for full license details.


L = size(A, 1);
assert(size(A, 2) == size(B, 1))

dummy = A(:, :, 1, 1, 1) * B(:, :, 1, 1, 1);

sizeA = size(A);
sizeB = size(B);
sizeC = [size(dummy), max(sizeA(3:end), sizeB(3:end))];
classC = class(dummy);

C = zeros(sizeC, classC);

A = permute(A, [2, 1, 3, 4, 5]);

for ii = 1 : L
  C(ii, :, :, :, :) = sum(A(:, ii, :, :, :) .* B, 1);
end

end

