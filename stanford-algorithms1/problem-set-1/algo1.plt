set xrange [0:10000]

f(x) = 2 * x * x * log(2 * x * x)
g(x) = x * x * log (x * x)

plot f(x), g(x), g(x)/ f(x)
#plot g(x)/ f(x)

