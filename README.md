# Hot Ticket

An issue tracker built for CPSC3720 in C++, by Team Jalapeno!

## Basic Usage

To run the program, do the following:

### Server

1. Run `make server` to create the server executable, named `issueServer`

2. `./hotTicket-server` will begin running the server. This should output the following message right away:

   ```
   Server listening on http://127.0.0.1:8080...
   ```

   We can now issue requests to our server! 🎉

Note: You can run `./hotTicket &` to kick off the server in the background and free up your terminal. This is also accomplished by running `make runServer`

### Client

There are a couple ways to interact with the server. The simplest would be to run the client program:

1. In a new terminal window, run `make client` to build the client executable, named `hotTicket`

3. Run `./hotTicket` to begin the client program!

Additionally, you could run `cURL` commands with the server running to query the information:

1. (with the server running on localhost:8080) run `curl -GET "http://localhost:8080/users"`, which should output 

  ```json
  {"users":[{"id":"9ip70005tt","name":"Everett","role":"Developer"},{"id":"fss4fdbq58","name":"Sully","role":"Developer"},{"id":"o5ohxwysq2" ,"name":"Mike","role":"Developer"}]}
  ```

to the terminal
2. run `curl -GET "http://localhost:8080/users/9ip70005tt"` to retrieve a specific user:
  ```json
  {"id":"9ip70005tt","name":"Everett","role":"Developer"}
  ```
3. run `curl --request POST "http://localhost:8080/users" --data "{\"name\":\"Michael\",\"role\":\"Developer\"}" --header "Content-Length: 37"` to create a new user, which should return a newly generated user
 ```json
 {"id":"ma7j3audgl","name":"Michael","role":"Developer"}
 ```
 Note: for this command to work, `role` must be specified on the user object

## Advanced Usage

Both the client and server programs accept command line arguments, thanks to the [cxxopts](https://github.com/jarro2783/cxxopts/) library. 

Assuming you have the programs built (see [Basic Usage](#basic-usage)), run `./hotTicket-server --help` or `./hotTicket --help` to see the available command line arguments.

### Server command line arguments:

- `-d` or ` --debug`: If this argument is provided, [restbed Logging](https://github.com/Corvusoft/restbed/blob/master/documentation/example/LOGGING.md) will be enabled
- `-p` or  `--port` (integer):  The port to run the server on (default is `8080`)
- `-h` or `--help`: Prints out the help message

**Example**

```bash
./hotTicket-server -p 6969
Server listening on http://127.0.0.1:6969...
```

```bash
./hotTicket-server -p 6969 -d
Server listening on http://127.0.0.1:6969...
Wed Dec 02 01:02:20 2020: [INFO] Service accepting HTTP connections at 'http://127.0.0.1:6969'.
Wed Dec 02 01:02:20 2020: [INFO] Resource published on route '/alive'.
Wed Dec 02 01:02:20 2020: [INFO] Resource published on route '/comments'.
Wed Dec 02 01:02:20 2020: [INFO] Resource published on route '/comments/{id:[a-z0-9]*}'.
Wed Dec 02 01:02:20 2020: [INFO] Resource published on route '/issues/{issueId:[a-z0-9]*}/comments'.
Wed Dec 02 01:02:20 2020: [INFO] Resource published on route '/issues/{issueId:[a-z0-9]*}/comments/{id:[a-z0-9]*}'.
Wed Dec 02 01:02:20 2020: [INFO] Resource published on route '/issues/{issueId:[a-z0-9]*}/votes'.
Wed Dec 02 01:02:20 2020: [INFO] Resource published on route '/issues/{issueId:[a-z0-9]*}/votes/{id:[a-z0-9]*}'.
Wed Dec 02 01:02:20 2020: [INFO] Resource published on route '/users'.
Wed Dec 02 01:02:20 2020: [INFO] Resource published on route '/users/{id:[a-z0-9]*}'.
Wed Dec 02 01:02:20 2020: [INFO] Resource published on route '/votes'.
Wed Dec 02 01:02:20 2020: [INFO] Resource published on route '/votes/{id:[a-z0-9]*}'.
```

### Client command line arguments

- `-H` or `--host` (string): The IP address where the server is running (default is `localhost`)

- `-p` or `--port` (integer): The port where the server is running (default is `8080`)

- `-u` or `--url`(string): The full URL to where the server is running

  > Note: If `-u` or `--url` are specified, any host and port arguments are ignored.

- `-h` or `--help`: Prints help text

**Examples**

```bash
./hotTicket
Connecting the to server at 'http://localhost:8080'...
```

```bash
./hotTicket -p 1234
Connecting to the server at 'http://localhost:1234'...
```

```bash
./hotTicket -p 1234 -H 172.16.0.9
Connecting to the server at 'http://172.16.0.9:1234'...
```

If the server was running on an AWS EC2 instance, where `52.204.122.132` is the public IP address, then you could run

```bash
./hotTicket -u https://ec2-52-204-122-132.compute-1.amazonaws.com
Connecting to the server at 'https://ec2-52-204-122-132.compute-1.amazonaws.com'...
```

