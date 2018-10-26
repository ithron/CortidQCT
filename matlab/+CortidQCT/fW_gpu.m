function f = fW_gpu(w, muW, sigmaW)
%FW_CPU GPU compatible version of fW.

f = 1./(w .* sigmaW * sqrt(2*pi)) .* exp(-0.5 .* (log(abs(w)) - muW).^2 ./ sigmaW^2);


if f <= 0 || isnan(f)
    f = single(0);
end


end

