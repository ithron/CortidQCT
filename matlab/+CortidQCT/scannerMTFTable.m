function M = scannerMTFTable(varargin)
%SCANNERMTFMAP Returns a table that contains some measurements of the MTF
%of the given scanner + kernel
%   M = scannerMTFTable(scanner) - returns an Nx2 matrix where the first
%   column contains frequencies [lp/mm] and the second column contains the
%   corresponding MTF values.
%
%   C = scannerMTFTable() - returns a cell array of all supported scanner
%   configurations.
%
%   The data is based on Ohkubu et al., 2009, Determination of point
%   spread function in computed tomography accompanied with verification,
%   American Association of Physicists in Medicine, DOI: 10.1118/1.3123762.

% This file is part of the 'CortidQCT' project.
% Author: Stefan Reinhold
% Copyright: Copyright (C) 2018 Stefan Reinhold  -- All Rights Reserved.¬
%            You may use, distribute and modify this code under the terms of¬
%            the AFL 3.0 license; see LICENSE for full license details.


Values = {...
  ...% Philips Brilliance Kernel D
  [0     1
   0.2   0.9685
   0.3   0.8601
   0.4   0.6923
   0.5   0.5 
   0.6   0.3601
   0.8   0.1608 
   1.0   0.0524
   1.2   0.0140
   1.4   0], ...
  ... % Siemens Somatom B40
  [0     1
   0.2   0.97
   0.3   0.81
   0.4   0.58
   0.5   0.37
   0.6   0.21
   0.7   0.11
   0.8   0.046
   1.0   0
   1.2   0
   1.4   0
  ], ...
  ... % Siemens Somatom B70
  [0     1
   0.1   1.092
   0.2   1.246
   0.3   1.382
   0.4   1.415
   0.5   1.331
   0.6   1.07
   0.75  0.5
   0.8   0.338
   0.9   0.146
   1.0   0.038
   1.1   0.015
   1.2   0
   1.4   0]...
  };

Keys = {...
  'Philips Brilliance D', ...
  'Siemens Somatom B40', ...
  'Siemens Somatom B70' ...
  };

Map = containers.Map(Keys, Values);

if nargin == 1
  M = Map(varargin{1});
elseif nargin == 0
  M = Keys;
else
  error('Too many inputs');
end

end

