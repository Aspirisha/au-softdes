## Shell Architecture Description
Shell consists of two parts: 
* driver, which implements shell itself without any commands
* tools - set of utilities like pwd, wc and so on and so forth

Driver is responsible for processing user input and performing one of three actions depending on it:

1. Exiting 
2. Calling utility with passed args
3. Setting environment variable

Driver tracks information about set environment variables and current directory. 

Each of the tools is a separate standalone application which can be called independently of driver. When user calls specific utility, it's just called synchronously as a child process from driver. Communication is performed via java streams api. 
