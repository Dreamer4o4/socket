# mywebbench
refer to WebBench

# myhttpd
refer to Tinnyhttpd
reactor + thread_pool

# myhttpd bench result
ab -n 10000 -k -r -c 1000 127.0.0.1:4000/

This is ApacheBench, Version 2.3 <$Revision: 1843412 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 127.0.0.1 (be patient)
Completed 1000 requests
Completed 2000 requests
Completed 3000 requests
Completed 4000 requests
Completed 5000 requests
Completed 6000 requests
Completed 7000 requests
Completed 8000 requests
Completed 9000 requests
Completed 10000 requests
Finished 10000 requests


Server Software:        zhttp
Server Hostname:        127.0.0.1
Server Port:            4000

Document Path:          /
Document Length:        0 bytes

Concurrency Level:      1000
Time taken for tests:   0.703 seconds
Complete requests:      10000
Failed requests:        0
Non-2xx responses:      10000
Keep-Alive requests:    0
Total transferred:      680000 bytes
HTML transferred:       0 bytes
Requests per second:    14223.48 [#/sec] (mean)
Time per request:       70.306 [ms] (mean)
Time per request:       0.070 [ms] (mean, across all concurrent requests)
Transfer rate:          944.53 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   1.6      0      14
Processing:     0    1   1.4      1      30
Waiting:        0    1   1.2      1      30
Total:          0    2   2.7      1      41

Percentage of the requests served within a certain time (ms)
  50%      1
  66%      1
  75%      1
  80%      1
  90%      1
  95%      7
  98%     14
  99%     16
 100%     41 (longest request)

