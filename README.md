# mywebbench
refer to WebBench

# myhttpd
refer to Tinyhttpd           
reactor + thread_pool       

## myhttpd bench result
cmd:           
ab -n 10000 -r -c 1000 127.0.0.1:4000/          

thread num:          
1       

environment:      
cpu name  : Intel(R) Core(TM) i7-8750H CPU @ 2.20GHz        
cpu core num    : 8      
memory size : 8GB           

result:        
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
Document Length:        187 bytes     

Concurrency Level:      1000      
Time taken for tests:   0.485 seconds     
Complete requests:      10000     
Failed requests:        0     
Total transferred:      2460000 bytes     
HTML transferred:       1870000 bytes     
Requests per second:    20607.00 [#/sec] (mean)     
Time per request:       48.527 [ms] (mean)      
Time per request:       0.049 [ms] (mean, across all concurrent requests)  
Transfer rate:          4950.51 [Kbytes/sec] received     

Connection Times (ms)     
              min  mean[+/-sd] median   max     
Connect:        0    0   1.5      0      13     
Processing:     0    1   1.9      1      13     
Waiting:        0    1   1.9      1      10     
Total:          0    2   2.7      1      16       

Percentage of the requests served within a certain time (ms)      
  50%      1      
  66%      1      
  75%      1      
  80%      1      
  90%      4          
  95%      8      
  98%     13      
  99%     14      
 100%     16 (longest request)      




