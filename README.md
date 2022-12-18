Multi-Threaded HTTP Server
---
How to Build/Run:
-run make to compile httpserver binary file and create object httpserver.o
-make clean removes binary file and httpserver.o from directory
---
---
Functionality : <br />
-type ./httpserver [-t threads] [-l logfile] <port><br />
-port is an integer that the server will listen into<br />
-threads is number of threads program will run (still in progress)<br />
-logfile is where requests are logs <br />
-the program runs a server that runs forever.<br />
-it listens and accepts connections from a client through the use of sockets<br />
-To communicate, it follows the HTTP 1.1 protocol<br />
-This program currently accepts three types of methods: GET, PUT, and HEAD <br />
-for GET, the server waits for a request and the client requests a file identified by <br />
a URI which is given by the client.  The GET gives a reponse to the client with the content
of the URI and also a status code that indicates if it's successful or not. <br />
-PUT is when a client requests to add/create new data to a URI to the server<br />
-HEAD is likely about the same procedure as GET, however it doesn't include the content
of the file but rather just shows its status.<br />
---
---
To run tests : <br />
<br />
For Single-Threaded tests:
-make sure test_files and test_script directories are all in the same directory as httpserver binary file<br />
-execute the test scripts from the same directory as httpserver.  For example, ./test_scripts/put_binary_large.sh<br />
-run "./test_scripts/run-all-test.sh" to run all Single-Threaded tests <br />
-tests returns 0 for sucess and a non zero for failure.<br />
<br />
For Multi-Threaded tests:<br />
Requirements:<br />
-python3.9 installed<br />
<br />
-run "./test_scripts/stress_test_runner.sh" to multithreaded tests<br />

Note: stress_test_runner.sh have parameters you can change which will be stated below<br />
Parameters:<br />
-threads: number of threads<br />
-seed: helps guarantees deterministic results.<br />
-num_requests: number of requests to send<br />
-num_uris: number of unique URI's to generate<br />
-num_files: number of unique files to generate<br />
<br />
Make sure to delete produced files and uris by the program for each tests.  Just run "rm file*" and "rm uri*" to delete most files<br />
---
