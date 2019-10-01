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
      obj.dt = 1e-3;
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
      %  joint - boolean indicating if a single joint base matrix
      %   for all angles should be returned or an independent one for each
      %   angle. Defaults to `true`.
      % Returns a N*Mx3xK matrix iff `joint == true` and a Mx3xNxK matrix
      % otherwise.
      
      M = size(t, 1);
      N = length(theta);
      K = size(w, 4);
      
      if not(isempty(varargin))
        joint = varargin{1} == true;
      else
        joint = true;
      end
      
      assert(size(w, 4) == K);
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
    
    function PsiPlus = pinv(obj, t, w, s, theta, varargin)
      % PINV Computes the pseudo inverse of the base matrix
      %  PsiPlus = pinv(obj, t, w, s, theta)
      %  PsiPlus = pinv(obj, t, w, s, theta, joint)
      %  t - positions at which to evaluate the base matrix [mm], Mx1xNxK
      %   real array
      %  w - half cortex width [mm], eiehter a scalar or an 1x1x1xK array
      %  s - shift parameter [mm], either a scalar or an 1x1xNxK array
      %  theta - angle(s) with the z-axis [rad], 1x1xN array
      %  joint - the joint parameter for `eval`. See eval.
      %  Returns a 3xN*MxK matrix iff `joint == true` and a 3xMxNxK
      %  otherwise.
      
      Psi = obj.eval(t, w, s, theta, varargin{:});
      
      sz = [3, size(Psi, 1), size(Psi, 3), size(Psi, 4)];
      
      if ismatrix(Psi)
        PsiPlus = (Psi.' * Psi) \ Psi.';
      elseif not(obj.useGPU)
        PsiPlus = zeros(sz(1), sz(2), sz(3)*sz(4), class(Psi));
        Psi = reshape(Psi, sz(2), sz(1), []);
        for ii = 1 : size(Psi, 3)
          PsiPlus(:, :, ii) = (Psi(:, :, ii).' * Psi(:, :, ii)) \ Psi(:, :, ii).';
        end
        PsiPlus = reshape(PsiPlus, sz);
      else
        sz = [3, size(Psi, 1), size(Psi, 3), size(Psi, 4)];
        Psi = reshape(Psi, sz(2), sz(1), sz(3) * sz(4));
        PsiPlus = pagefun(@mtimes, permute(Psi, [2, 1, 3]), Psi);
        PsiPlus = pagefun(@mldivide, PsiPlus, permute(Psi, [2, 1, 3]));
        PsiPlus = reshape(PsiPlus, sz);
      end
      
    end
    
    function PsiPlusDs = dsPinv(obj, t, w, s, theta, varargin)
      % DSPINV Computes the derivative of the pseudo inverse of the base
      % matrix wrt. the shift parameter s.
      % PsiPlusDs = dsPinv(obj, t, w, s, theta)
      % PsiPlusDs = dsPinv(obj, t, w, s, theta, joint)
      %  %  t - positions at which to evaluate the base matrix [mm]
      %  w - half cortex width [mm]
      %  s - shift parameter [mm]
      %  theta - angle(s) with the z-axis [rad]
      %  joint - the joint parameter for `eval`. See eval.
      %  Returns a 3xNxK matrix, where N = length(t) and K = length(theta)
      
      Psi = obj.eval(t, w, s, theta, varargin{:}); % M*Nx3xK or Mx3xNxK
      PsiDs = obj.ds(t, w, s, theta); % Mx3xNxK
      
      M = size(t, 1);
      N = length(theta);
      K = length(w);
      
      assert(size(Psi, 1) == N * M || (size(Psi, 1) == M && size(Psi, 3) == N && size(Psi, 4) == K));
      assert(size(Psi, 3) == K || (size(Psi, 3) == N && size(Psi, 4) == K));
      
      joint = not(size(Psi, 3) == N && size(Psi, 4) == K);
      
      sz = size(Psi);
      
      if ismatrix(PsiDs)
        PP = Psi.' * Psi;
        PPds = PsiDs.' * Psi + Psi.' * PsiDs;
        
        PsiPlusDs = real(-(PP \ PPds) * (PP \ Psi.') + PP \ PsiDs.');
        
      else
        
        if not(obj.useGPU)
          if joint
            PsiPlusDs = zeros(3, M * N, N, K, class(Psi));
          
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
            
            PsiPlusDs = zeros(3, M, N * K, class(Psi));
            Psi = reshape(Psi, M, 3, []);
            PsiDs = reshape(PsiDs, M, 3, []);
            
            for ii = 1 : size(Psi, 3)
              
              PPI = real(inv(Psi(:, :, ii).' * Psi(:, :, ii)));
              
              PPds = PsiDs(:, :, ii).' * Psi(:, :, ii);
              PPds = PPds + PPds.';
              
              PsiPlusDs(:, :, ii) = -PPI * PPds * PPI * Psi(:, :, ii).';
              PsiPlusDs(:, :, ii) = PsiPlusDs(:, :, ii) + PPI * PsiDs(:, :, ii).';
              
            end
            
            PsiPlusDs = reshape(PsiPlusDs, 3, M, N, K);
          end
        else
          
          
          if not(joint)
            Psi = reshape(Psi, sz(1), sz(2), []);
          end
          PPI = pagefun(@inv, pagefun(@mtimes, permute(Psi, [2, 1, 3]), Psi));
          if joint
            PsiSep = permute(reshape(Psi, M, N, 3, K), [1, 3, 2, 4]);
          else
            PsiSep = reshape(Psi, sz);
          end
          PPds = pagefun(@mtimes, permute(reshape(PsiSep, M, 3, N * K), [2, 1, 3]), ...
            reshape(PsiDs, M, 3, N*K));
          PPds = reshape(PPds, 3, 3, N, K);
          PPds = PPds + permute(PPds, [2, 1, 3, 4]);
          
          if joint
            PPI = permute(PPI, [1, 2, 4, 3]);
          else
            PPI = reshape(PPI, 3, 3, N, K);
            Psi = reshape(Psi, sz);
          end
          
          PsiPlusDs = CortidQCT.mtimesND(PPds, PPI);
          PsiPlusDs = -CortidQCT.mtimesND(PPI, PsiPlusDs);
          
          if joint
            PsiPlusDs = CortidQCT.mtimesND(PsiPlusDs, permute(Psi, [2, 1, 4, 3]));
            
            for ii = 1 : N
              PsiPlusDs(:, (ii - 1) * M + 1 : ii * M, ii, :, :) = ...
              PsiPlusDs(:, (ii - 1) * M + 1 : ii * M, ii, :, :) + ...
              CortidQCT.mtimesND(PPI, permute(PsiDs(:, :, ii, :), [2, 1, 3, 4]));
            end
            
          else
            PsiPlusDs = CortidQCT.mtimesND(PsiPlusDs, permute(Psi, [2, 1, 3, 4]));
            PsiPlusDs = PsiPlusDs + CortidQCT.mtimesND(PPI, permute(PsiDs, [2, 1, 3, 4]));
          end
          
        end
        
      end
      
    end
    
    function GG = autoCorrPSF(obj, t, theta)
      % AUTOCORRPSF Computes the autocorrelation matrix of the PSF
      %  t - positions at which to evaluate the base matrix [mm]
      %  theta - angle(s) with the z-axis [rad]
      %  Returns a NxNxK matrix, where N = length(t) and K = length(theta)
      
      M = size(t, 1);
      N = length(theta);
      K = size(t, 4);
      
      t = permute(t - t(1, :, :, :), [2, 1, 3, 4]);
      
      t = [-t(:, end:-1:2, :, :), t];
      
      if size(t, 3) == 1
        t = repmat(t, 1, 1, N);
      end
      
      theta = repmat(theta, 1, 2 * M - 1, 1, K);
      
      GG = interp2(obj.X, obj.Y, obj.gTable, t(:), theta(:));
      
      GG(t(:) < min(obj.t) | t(:) > max(obj.t)) = 0;
      
      GG = reshape(GG, 2 * M - 1, 1, N * K);
      GG = num2cell(GG, [1, 2]);
      
      GG = cellfun(@(x) conv(x, x, 'same'), GG, 'UniformOutput', false);
      GG = cellfun(@(x) x(M : end), GG, 'UniformOutput', false);
      
      GG = cellfun(@toeplitz, GG, 'UniformOutput', false);
      GG = cat(3, GG{:});
           
      GG = reshape(GG, M, M, N, K);
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

