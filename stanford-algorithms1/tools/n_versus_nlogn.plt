f(x) = x
g(x) = log(x) / log(2)
h(x) = x * (log(x) / log(2))

set xrange [1:10000]
set logscale x 2

plot f(x), g(x), h(x)
