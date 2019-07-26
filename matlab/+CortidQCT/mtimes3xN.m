function C = mtimes3xN(A, B)
% MTIMES3XN Computes the matrix product of the (up to) 4D arrays A and B
%  C = mtimes2xN(A, B)
% A - 3xMx...x... array
% B - MxNx...x... array
% C - 3xNx...x... array representing the matrix product of A and B using
% broadcasting.

% This file is part of the 'CortidQCT' project.
% Author: Stefan Reinhold
% Copyright: Copyright (C) 2019 Stefan Reinhold  -- All Rights Reserved.¬
%            You may use, distribute and modify this code under the terms of¬
%            the AFL 3.0 license; see LICENSE for full license details.


assert(size(A, 1) == 3)
assert(size(A, 2) == size(B, 1))

dummy = A(:, :, 1, 1) * B(:, :, 1, 1);

sizeA = size(A);
sizeB = size(B);
sizeC = [size(dummy), max(sizeA(3:end), sizeB(3:end))];
classC = class(dummy);

C = zeros(sizeC, classC);

A = permute(A, [2, 1, 3, 4]);

C(1, :, :, :) = sum(A(:, 1, :, :) .* B, 1);
C(2, :, :, :) = sum(A(:, 2, :, :) .* B, 1);
C(3, :, :, :) = sum(A(:, 3, :, :) .* B, 1);

end

