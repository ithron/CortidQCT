function [f, Z_, T_] = evalFZ(z, t, theta, mu, s, muW, sigmaW, GTable)

delta = 1e-3;

maxW = exp(muW + sigmaW * norminv(1 - 1e-5));
w = linspace(0, maxW, maxW / delta + 1);

[T_, Z_, W] = meshgrid(single(t), single(z), single(w));

T = gpuArray(T_);
Z = gpuArray(Z_);
W = gpuArray(W);

[TT, WW] = meshgrid(single(t), single(w));
TT_ = gpuArray(TT);
WW_ = gpuArray(WW);

TpW = TT_ + WW_;
TmW = TT_ - WW_;

One_ = ones(size(TpW), 'gpuArray');

clear TT_ WW_;

f = zeros(length(z), length(t), length(theta));

textprogressbar('Evaluating PDF: ');

for ii=1:length(theta)
    thetai = single(theta(ii));
    
    % Compute integrals of kernel function
    Gp = G(TpW, thetai * One_, GTable);
    Gn = G(TmW, thetai * One_, GTable);
    
    Gp = reshape(Gp', 1, size(T, 2), size(T, 3));
    Gn = reshape(Gn', 1, size(T, 2), size(T, 3));

    F = arrayfun(...
        @(t, z, w, theta, muW, sigmaW, mu1, mu2, mu3, s1, s2, s3, Gp, Gn) ...
        fW_gpu(w, muW, sigmaW) .* ...
        fZeta(z, Gp, Gn, mu1, mu2, mu3, s1, s2, s3), ...
        T, Z, W, theta(ii), muW, sigmaW, mu(1), mu(2), mu(3), s(1), s(2), s(3), Gp, Gn);

    clear Gp Gn

    f(:, :, ii) = double(gather(sum(F, 3))) .* delta;

    clear F
    
    textprogressbar(100 * ii / length(theta));
end
textprogressbar(' done.');

clear T Z W TpW TmW

T_ = T_(:, :, 1);
Z_ = Z_(:, :, 1);

end

