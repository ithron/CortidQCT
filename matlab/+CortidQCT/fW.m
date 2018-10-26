function f = fW(w, muW, sigmaW)
%FW Probability density function of log-normal distributed random variable
%W with mean muW and standard deviation sigmaW.

f = 1./(w .* sigmaW * sqrt(2*pi)) .* exp(-0.5 .* (log(abs(w)) - muW).^2 ./ sigmaW^2);

f(f <= 0 | isnan(f)) = 0;

end

