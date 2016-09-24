# Software design hw
------
### Task 1: fake shell
Fake shell should support following commands:
* exit
* pwd
* cat [FILE]
* echo 
* wc [FILE] 

Also pipe redirection should work for these tools.
Solution lives in task1 branch of this repo.

---
####Installation:

1. Clone repository
2. Run ```gradle deploy``` 
3. Now shell is installed into $HOME/sdshell/ directory. 

####Running the shell:

cd $HOME/sdshell/ && java -jar driver.jar
    