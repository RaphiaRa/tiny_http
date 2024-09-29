## Benchmark

To reproduce the benchmark results, first create the testfile for 
the static file test (Only if you want to actually run that test):
    
```bash
mkdir -p testfiles
dd if=/dev/urandom of=testfiles/test_50000 bs=50000 count=1
 ```

Compile respective benchmarks (Compile commands are 
in the file header comments) and run them.
Running the static file test:
```bash
wrk -c 512 -d 1s http://localhost:8080/test_50000
```
Running the hello world test:
```bash
wrk -c 512 -d 1s http://localhost:8080/
```

