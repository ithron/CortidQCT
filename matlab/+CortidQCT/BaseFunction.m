classdef BaseFunction
  %BASEFUNCTION Implementation of the base function matrix using
  %precomputed lookup tables to speed up computations.
  
  % This file is part of the 'CortidQCT' project.
  % Author: Stefan Reinhold
  % Copyright: Copyright (C) 2019 Stefan Reinhold  -- All Rights Reserved.¬
  %            You may use, distribute and modify this code under the terms of¬
  %            the AFL 3.0 license; see LICENSE for full license details.
  
  properties (SetAccess = private)
    t
    dt
    theta
    dTheta
    sigmaG
    sliceThickness
  end
  
  properties
    useGPU
  end
  
  properties (Access = private)
    gTable
    GTable
    autoCorrTable
    X, Y
  end
  
  
  methods
    function obj = BaseFunction(sigmaG, sliceThickness)
      %BASEFUNCTION Construct an instance of this class
      %   Precomputes lookup tables for the primitive function of the PSF
      %   and the autocorrelation functin of the PSF
      %  obj = BaseFunction(sigmaG, sliceThickness)
      %  sigmaG - parameters of the PSF, see CortidQCT.PSF
      %  sliceThickness - collimation width
      
      minT = -5;
      maxT =  5;
      obj.dt = 1e-4;
      dtheta = pi/180;
      obj.dTheta = dtheta;
      obj.sigmaG = sigmaG;
      obj.sliceThickness = sliceThickness;
      obj.t = linspace(minT, maxT, (maxT - minT) / obj.dt);
      obj.theta = linspace(0, pi/2, pi/2 / dtheta);
      
      try
        obj.useGPU = parallel.gpu.GPUDevice.isAvailable;
      catch
        obj.useGPU = false;
      end
      
      obj = obj.genTables();
    end
    
    function Psi = eval(obj, t, w, s, theta, varargin)
      % EVAL evaluates the base matrix at the given positions, width,
      % shift and angle.
      %  Psi = eval(obj, t, w, s, theta)
      %  Psi = eval(obj, t, w, s, theta, joint)
      %  t - positions at which to evaluate the base matrix [mm], Mx1xNxK
      %   real array
      %  w - half cortex width [mm], eiehter a scalar or an 1x1x1xK array
      %  s - shift parameter [mm], either a scalar or an 1x1xNxK array
      %  theta - angle(s) with the z-axis [rad], 1x1xN array
      % independent - boolean indicating if a single joint base matrix
      %   for all angles should be returned or an independent one for each
      %   angle. Defaults to `true`.
      % Returns a N*Mx3xK matrix iff `joint == true` and a Mx3xNxK matrix
      % otherwise.
      
      M = size(t, 1);
      N = size(t, 3);
      K = length(w);
      
      if not(isempty(varargin))
        joint = varargin{1} == true;
      else
        joint = true;
      end
      
      assert(size(w, 4) == K);
      assert(numel(w) == K);
      assert(size(s, 3) == N);
      assert(numel(s) == N);
      assert(size(theta, 3) == N);
      assert(numel(theta) == N);
      
      tp = t + w - s; % Mx1xNxK
      tn = t - w - s;
      
      theta = repmat(theta, M, 1, 1, K);
      
      Gp = interp2(obj.X, obj.Y, obj.GTable, tp(:), theta(:));
      Gn = interp2(obj.X, obj.Y, obj.GTable, tn(:), theta(:));
      
      Gp(tp(:) < min(obj.t)) = 0;
      Gp(tp(:) > max(obj.t)) = 1;
      
      Gn(tn(:) < min(obj.t)) = 0;
      Gn(tn(:) > max(obj.t)) = 1;
      
      Gp = reshape(Gp, M, 1, N, K);
      Gn = reshape(Gn, M, 1, N, K);
      
      Psi = [1 - Gp, Gp - Gn, Gn];
      
      if joint
        Psi = permute(Psi, [1, 3, 2, 4]);
        Psi = reshape(Psi, M*N, 3, K);
      end
      
    end
    
    function dsPsi = ds(obj, t, w, s, theta)
      % DSPSI Computes the derivatives of the base matrix wrt. the shift
      % parameter s.
      % dsPsi = ds(obj, t, w, s, theta)
      %  t - positions at which to evaluate the base matrix [mm], Mx1xNxK
      %   real array
      %  w - half cortex width [mm], eiehter a scalar or an 1x1x1xK array
      %  s - shift parameter [mm], either a scalar or an 1x1xNxK array
      %  theta - angle(s) with the z-axis [rad], 1x1xN array
      % Returns a Mx3xNxK matrix containgin the non-zero parital
      % derivitives of the base function
      %
      % Note that the derivatives are the same for both, joint and
      % indpendent base matrices, since only the non-zero sub matrices are
      % returned in the joint case.
      
      M = size(t, 1);
      N = size(t, 3);
      K = length(w);
      
      assert(size(w, 4) == K);
      assert(numel(w) == K);
      assert(size(s, 3) == N);
      assert(numel(s) == N);
      assert(size(theta, 3) == N);
      assert(numel(theta) == N);
      
      tp = t + w - s;
      tn = t - w - s;
      
      theta = repmat(theta, M, 1, 1, K);

      gp = interp2(obj.X, obj.Y, obj.gTable, tp(:), theta(:), 'cubic');
      gn = interp2(obj.X, obj.Y, obj.gTable, tn(:), theta(:), 'cubic');
      
      gp(tp(:) < min(obj.t) | tp(:) > max(obj.t)) = 0;
      gn(tn(:) < min(obj.t) | tn(:) > max(obj.t)) = 0;
      
      gp = reshape(gp, M, 1, N, K);
      gn = reshape(gn, M, 1, N, K);
      
      dsPsi = [gp, gn - gp, -gn];
    end
    
    function PsiPlus = pinv(obj, t, w, s, theta)
      % PINV Computes the pseudo inverse of the base matrix
      %  t - positions at which to evaluate the base matrix [mm], Mx1xNxK
      %   real array
      %  w - half cortex width [mm], eiehter a scalar or an 1x1x1xK array
      %  s - shift parameter [mm], either a scalar or an 1x1xNxK array
      %  theta - angle(s) with the z-axis [rad], 1x1xN array
      %  Returns a 3xN*MxK matrix
      
      Psi = obj.eval(t, w, s, theta);
      
      if ismatrix(Psi)
        PsiPlus = (Psi.' * Psi) \ Psi.';
      elseif not(obj.useGPU)
        PsiPlus = zeros(3, size(Psi, 1), size(Psi, 3), class(Psi));
        for ii = 1 : size(Psi, 3)
          PsiPlus(:, :, ii) = (Psi(:, :, ii).' * Psi(:, :, ii)) \ Psi(:, :, ii).';
        end
      else
        PsiPlus = pagefun(@mtimes, permute(Psi, [2, 1, 3]), Psi);
        PsiPlus = pagefun(@mldivide, PsiPlus, permute(Psi, [2, 1, 3]));
      end
      
    end
    
    function PsiPlusDs = dsPinv(obj, t, w, s, theta)
      % DSPINV Computes the derivative of the pseudo inverse of the base
      % matrix rt. the shift parameter s.
      %  %  t - positions at which to evaluate the base matrix [mm]
      %  w - half cortex width [mm]
      %  s - shift parameter [mm]
      %  theta - angle(s) with the z-axis [rad]
      %  Returns a 3xNxK matrix, where N = length(t) and K = length(theta)
      
      Psi = obj.eval(t, w, s, theta); % Mx3xK
      PsiDs = obj.ds(t, w, s, theta); % Mx3xNxK
      
      M = size(t, 1);
      N = length(theta);
      K = length(w);
      
      assert(size(Psi, 1) == N * M);
      assert(size(Psi, 3) == K);
      
      if ismatrix(PsiDs)
        PP = Psi.' * Psi;
        PPds = PsiDs.' * Psi + Psi.' * PsiDs;
        
        PsiPlusDs = real(-(PP \ PPds) * (PP \ Psi.') + PP \ PsiDs.');
        
      else
        
        if not(obj.useGPU)
          PsiPlusDs = zeros(3, M * N, N, K);
          PsiSep = permute(reshape(Psi, M, N, 3, K), [1, 3, 2, 4]);
          for ii = 1 : K
            
            PPI = real(inv(Psi(:, :, ii).' * Psi(:, :, ii)));
            
            for jj = 1 : N
              PPds = ...
                PsiDs(:, :, jj, ii).' * PsiSep(:, :, jj, ii);
              PPds = PPds + PPds.';
              
              PsiPlusDs(:, :, jj, ii) = ...
                -PPI * PPds * PPI * Psi(:, :, ii).';
              PsiPlusDs(:, (jj - 1) * M + 1 : jj * M, jj, ii) = ...
                PsiPlusDs(:, (jj - 1) * M + 1 : jj * M, jj, ii) + ...
                PPI * PsiDs(:, :, jj, ii).';
              
            end
          end
        else
          
          PPI = pagefun(@inv, pagefun(@mtimes, permute(Psi, [2, 1, 3]), Psi));
          PsiSep = permute(reshape(Psi, M, N, 3, K), [1, 3, 2, 4]);
          PPds = pagefun(@mtimes, permute(reshape(PsiSep, M, 3, N * K), [2, 1, 3]), ...
            reshape(PsiDs, M, 3, N*K));
          PPds = reshape(PPds, 3, 3, N, K);
          PPds = PPds + permute(PPds, [2, 1, 3, 4]);
          
          PPI = permute(PPI, [1, 2, 4, 3]);
          
          PsiPlusDs = CortidQCT.mtimesND(PPds, PPI);
          PsiPlusDs = -CortidQCT.mtimesND(PPI, PsiPlusDs);
          
          PsiPlusDs = CortidQCT.mtimesND(PsiPlusDs, permute(Psi, [2, 1, 4, 3]));
          
          for ii = 1 : N
            PsiPlusDs(:, (ii - 1) * M + 1 : ii * M, ii, :, :) = ...
              PsiPlusDs(:, (ii - 1) * M + 1 : ii * M, ii, :, :) + ...
              CortidQCT.mtimesND(PPI, permute(PsiDs(:, :, ii, :), [2, 1, 3, 4]));
          end
          
        end
        
      end
      
    end
    
    function GG = autoCorrPSF(obj, t, theta)
      % AUTOCORRPSF Computes the autocorrelation matrox of the PSF
      %  t - positions at which to evaluate the base matrix [mm]
      %  theta - angle(s) with the z-axis [rad]
      %  Returns a NxNxK matrix, where N = length(t) and K = length(theta)
      
      M = size(t, 1);
      N = size(t, 3);
      K = size(t, 4);
      
      t = t - permute(t, [2, 1, 3, 4]);
      
      theta = repmat(theta, M, M, 1, K);
      
      gg = interp2(obj.X, obj.Y, obj.autoCorrTable, t(:), theta(:));
      
      gg(t(:) < min(obj.t) | t(:) > max(obj.t)) = 0;
      
      GG = reshape(gg, M, M, N, K);
    end
    
    function obj = set.useGPU(obj, val)
      oldVal = obj.useGPU;
      obj.useGPU = val;
      
      if oldVal ~= val
        obj = obj.switchDevice(val);
      end
    end
    
  end
  
  methods (Access = private)
    
    function obj = genTables(obj)
      % GENTABLES Generates lookup tables for the primitive function of the
      % PSF and the autocorrelation function of the PSF
      
      % Evaluate PSF, densely sampled
      g = CortidQCT.PSF(obj.t, obj.theta, obj.sigmaG, obj.sliceThickness);
      
      obj.gTable = permute(g, [3, 2, 1]);
      
      % Approximate primitive function by using trapezioid method
      obj.GTable = permute(cumtrapz(obj.t, g, 2), [3, 2, 1]);
      
      % Compute autocorrelation function: for each theta convolve the PSF
      % with itself
      obj.autoCorrTable = zeros(size(obj.GTable), class(g));
      for ii = 1 : length(obj.theta)
        obj.autoCorrTable(ii, :) = conv(g(:, :, ii), g(:, :, ii), 'same');
      end
      
      obj.autoCorrTable = obj.autoCorrTable * obj.dt;
      
      [obj.X, obj.Y] = meshgrid(obj.t, obj.theta);
      
      if obj.useGPU
        obj = obj.switchDevice(true);
      end
      
    end
    
    function obj = switchDevice(obj, toGPU)
      if toGPU
        obj.gTable = gpuArray(obj.gTable);
        obj.GTable = gpuArray(obj.GTable);
        obj.autoCorrTable = gpuArray(obj.autoCorrTable);
        obj.X = gpuArray(obj.X);
        obj.Y = gpuArray(obj.Y);
      else
        obj.gTable = gather(obj.gTable);
        obj.GTable = gather(obj.GTable);
        obj.autoCorrTable = gather(obj.autoCorrTable);
        obj.X = gather(obj.X);
        obj.Y = gather(obj.Y);
      end
    end
    
  end
  
  
end

