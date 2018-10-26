function y = gi(x, sigma)


if ~isvector(x)
    delta = x(1, 2) - x(1, 1);
else
    delta = x(2) - x(1);
end

y = exp(-0.5 * x.^2 ./ sigma.^2);

if isvector(y)
    y = y ./ sum(y);
else
    y = bsxfun(@rdivide, y, sum(y, 2));
end

% y = y / delta;

end

