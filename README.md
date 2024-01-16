# 1brc
One Billion Row Challenge Jan 2024

https://github.com/gunnarmorling/1brc

### Usage

```bash
$ make

$ ./generate 1000000 > measurements.txt
0.049 seconds

$ time ./1brc measurements.txt > /dev/null
Stations count (413): 413
Lines count: 1000000 (too few)

real	0m0.026s
user	0m0.024s
sys	0m0.002s
```
