# Software design hw
------
### Task 2: fake shell upgrade
Adding grep command to the shell. 

The command should be of the form 

grep [FLAGS] PATTERN [FILE] 

Where [FLAGS] may contain any combination of the following flags:

* <b>-i</b> case insensitive search
* <b>-w</b> match whole words only
* <b>-A n</b> print n lines after each match

---
####Installation:

1. Clone repository
2. Run ```gradle deploy``` 
3. Now shell is installed into $HOME/sdshell/ directory. 

####Running the shell:

cd $HOME/sdshell/ && java -jar driver.jar

####Tracker for the project
https://www.pivotaltracker.com/n/projects/1881227