set xrange [1:4.1]

f(x) = x
g(x) = 2 ** (2 ** x)
h(x) = 2 ** (x ** 2)
i(x) = x ** (2 ** x)
j(x) = x ** 2 * (log(x) / log(2))

plot log(log(g(x))), log(log(h(x)))

pause -1

#plot f(x), g(x), h(x), i(x), j(x)
