function mu = muZeta(Gp, Gn, mu1, mu2, mu3)
%MUZETA The expected value function of the random process zeta

mu = mu1 + Gp * (mu2 - mu1) + Gn * (mu3 - mu2);

end

