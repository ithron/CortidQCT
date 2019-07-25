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
  
  properties (Access = private)
    GTable
    autoCorrTable
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
      
      obj = obj.genTables();
    end
    
    function Psi = eval(obj, t, w, s, theta)
      % EVAL evaluates the base matrix at the given positions, width,
      % shift and angle.
      %  Psi = eval(obj, t, w, s, theta)
      %  t - positions at which to evaluate the base matrix [mm], 1xNxK
      %   real array
      %  w - half cortex width [mm], eiehter a scalar or an 1x1xK array
      %  s - shift parameter [mm], either a scalar or an 1x1xK array
      %  theta - angle(s) with the z-axis [rad], 1x1xK array
      % Returns a Nx3xK matrix, where N = length(t) and K = length(theta)
      
      N = size(t, 2);
      
      t = permute(t, [2, 1, 3]);
      
      if isrow(theta)
        theta = theta.';
      end
      
      tp = t + w - s;
      tn = t - w - s;
      
      theta = repmat(theta, N, 1, 1);
      
      [X, Y] = meshgrid(obj.t, obj.theta);
      
      Gp = interp2(X, Y, obj.GTable, tp(:), theta(:));
      Gn = interp2(X, Y, obj.GTable, tn(:), theta(:));
      
      Gp(tp(:) < min(obj.t)) = 0;
      Gp(tp(:) > max(obj.t)) = 1;
      
      Gn(tn(:) < min(obj.t)) = 0;
      Gn(tn(:) > max(obj.t)) = 1;
      
      Gp = permute(reshape(Gp, N, []), [1, 3, 2]);
      Gn = permute(reshape(Gn, N, []), [1, 3, 2]);
      
      Psi = [1 - Gp, Gp - Gn, Gn];
      
    end
    
    function dsPsi = ds(obj, t, w, s, theta)
      % DSPSI Computes the derivatives of the base matrix wrt. the shift
      % parameter s.
      % dsPsi = ds(obj, t, w, s, theta)
      %  t - positions at which to evaluate the base matrix [mm]
      %  w - half cortex width [mm]
      %  s - shift parameter [mm]
      %  theta - angle(s) with the z-axis [rad]
      %  Returns a Nx3xK matrix, where N = length(t) and K = length(theta)

      t = permute(t, [2, 1, 3]);
      
      tp = t + w - s;
      tn = t - w - s;
      
      gp = CortidQCT.PSF(tp, theta, obj.sigmaG, obj.sliceThickness);   
      gn = CortidQCT.PSF(tn, theta, obj.sigmaG, obj.sliceThickness);
      
      dsPsi = [gp, gn - gp, -gn];
    end
    
    function PsiPlus = pinv(obj, t, w, s, theta)
      % PINV Computes the pseudo inverse of the base matrix
      %  %  t - positions at which to evaluate the base matrix [mm]
      %  w - half cortex width [mm]
      %  s - shift parameter [mm]
      %  theta - angle(s) with the z-axis [rad]
      %  Returns a 3xNxK matrix, where N = length(t) and K = length(theta)
      
      Psi = obj.eval(t, w, s, theta);
      
      if ismatrix(Psi)
        PsiPlus = (Psi.' * Psi) \ Psi.';
      else
        PsiPlus = zeros(3, size(Psi, 1), size(Psi, 3), class(Psi));
        for ii = 1 : size(Psi, 3)
          PsiPlus(:, :, ii) = (Psi(:, :, ii).' * Psi(:, :, ii)) \ Psi(:, :, ii).';
        end
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
      
      Psi = obj.eval(t, w, s, theta);
      PsiDs = obj.ds(t, w, s, theta);
      
      if ismatrix(Psi)
        PP = Psi.' * Psi;
        PPds = PsiDs.' * Psi + Psi.' * PsiDs;
        
        PsiPlusDs = -(PP \ PPds) * (PP \ Psi.') + PP \ PsiDs.';
        
      else
        
        PsiPlusDs = zeros(3, size(Psi, 1), size(Psi, 3), class(Psi));
        for ii = 1 : size(Psi, 3)
          PP = Psi(:, :, ii).' * Psi(:, :, ii);
          PPds = PsiDs(:, :, ii).' * Psi(:, :, ii) + Psi(:, :, ii).' * PsiDs(:, :, ii);
        
          PsiPlusDs(:, :, ii) = -(PP \ PPds) * (PP \ Psi(:, :, ii).') + PP \ PsiDs(:, :, ii).';
        end
        
      end
      
    end
    
    function GG = autoCorrPSF(obj, t, theta)
      % AUTOCORRPSF Computes the autocorrelation matrox of the PSF
      %  t - positions at which to evaluate the base matrix [mm]
      %  theta - angle(s) with the z-axis [rad]
      %  Returns a NxNxK matrix, where N = length(t) and K = length(theta)
      
      N = size(t, 2);
      
      t = t - permute(t, [2, 1, 3]);
          
      [X, Y] = meshgrid(obj.t, obj.theta);
      
      [Xq, Yq] = meshgrid(t(:), theta);
      
      gg = interp2(X, Y, obj.autoCorrTable, Xq(:), Yq(:));
      
      gg(Xq(:) < min(obj.t) | Xq(:) > max(obj.t)) = 0;
      
      GG = permute(reshape(gg, [], N, N), [2, 3, 1]);
      
    end

  end
  
  methods (Access = private)
    
    function obj = genTables(obj)
      % GENTABLES Generates lookup tables for the primitive function of the
      % PSF and the autocorrelation function of the PSF
      
      % Evaluate PSF, densely sampled
      g = CortidQCT.PSF(obj.t, obj.theta, obj.sigmaG, obj.sliceThickness);
      
      % Approximate primitive function by using trapezioid method
      obj.GTable = permute(cumtrapz(obj.t, g, 2), [3, 2, 1]);
      
      % Compute autocorrelation function: for each theta convolve the PSF
      % with itself
      obj.autoCorrTable = zeros(size(obj.GTable), class(g));
      for ii = 1 : length(obj.theta)
        obj.autoCorrTable(ii, :) = conv(g(:, :, ii), g(:, :, ii), 'same');
      end
      
      obj.autoCorrTable = obj.autoCorrTable * obj.dt;
      
    end
    
  end
end

