function sigmaSq = varZeta(Gp, Gn, sigma1, sigma2, sigma3)
%VARZETA The variance function of the random process zeta

sigmaSq = (1 - Gp).^2 * sigma1.^2 + (Gp - Gn).^2 * sigma2.^2 + Gn.^2 * sigma3.^2;

end

