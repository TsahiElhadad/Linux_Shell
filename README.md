# Linux_Shell
C program that  implement Shell in linux. <br />
The program displays a prompt on the screen and allows the user to type commands in Linux like `ls`, `cat`, `sleep` and more. <br />
After pressing ENTER, the typed command will be executed. <br />

## Non built-in commands ##
For `non built-in commands` (commands that run by creation of son process) there is 2 options for use:
* Forground - The shell process which is a parent process will read the typed command by the user and create a son process that will execute it. The parent process will wait for the son process to finish before continuing to read additional commands from the user.
* Background - in this case, too, a son process will be created, but here the son process will run in the background, without the father process will wait for him.<br />
for Background command press & . for example: `ls &` <br/>
## Built-in commands ##
As mentioned, built-in commands are commands that are implemented independently by the shell program and are not create a son process.
The program implement 4 commands:
* jobs - displays list of commands that currently running in the background, in chronological order of their entry by user (from early to late).
* history - list of all the commands that the user entered during the run of the program, also order chronological from early to late, including failed commands.
* cd - changes the working directory (current) of the process.
* exit - will exit the program, with a return value of 0.
