# mywebbench
refer to WebBench

# myhttpd
refer to Tinnyhttpd
reactor + thread_pool

# myhttpd bench result
This is ApacheBench, Version 2.3 <$Revision: 1843412 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 127.0.0.1 (be patient)


Server Software:        zhttp
Server Hostname:        127.0.0.1
Server Port:            4000

Document Path:          /
Document Length:        187 bytes

Concurrency Level:      1000
Time taken for tests:   0.932 seconds
Complete requests:      10000
Failed requests:        1
   (Connect: 0, Receive: 0, Length: 1, Exceptions: 0)
Keep-Alive requests:    0
Total transferred:      2460004 bytes
HTML transferred:       1870004 bytes
Requests per second:    10726.53 [#/sec] (mean)
Time per request:       93.227 [ms] (mean)
Time per request:       0.093 [ms] (mean, across all concurrent requests)
Transfer rate:          2576.89 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0    0   1.3      0      12
Processing:     0    2   6.9      1      52
Waiting:        0    2   6.9      1      52
Total:          1    2   8.1      1      55

Percentage of the requests served within a certain time (ms)
  50%      1
  66%      1
  75%      1
  80%      1
  90%      1
  95%      1
  98%     50
  99%     52
 100%     55 (longest request)