README
This program 'myshell' emulates the basic functionalities of a c shell.
compilation : use the accompanying makefile

This program supports:

	processing input from terminal (run via "./myshell", then input as normal into a terminal)
	processing input from file (run via "./myshell < input_file")
		myshell will exit after processing input_file's contents

	cmd calls
		ex) "ls", "ls -l", "ls -l -t -h"
	cmd calls seperated by ';' they will execute sequentially
		ex) "ls; date"
	cmd calls seperated by '&', they will execute simultaniously
		ex) "sleep 100 & ls"
	input/output redirection
		ex)
		"cmd < input_file" uses input_file as the input for cmd
		"cmd > output_file" or "cmd 1> output_file" redirects the outout for the cmd to output_file
		"cmd 2> output_file" redirects stderr of cmd to output_file
		"cmd &> output_file" redirects output and stderr of cmd to output_file
	piping
		ex) "cmd1 | cmd2 | cmd3" pipes the output of cmd1 to be used as the input for
				cmd2, and the output of cmd2 to be used as the input for cmd3

	all inputs should have space to seperate every two entities
		this program does not properly parse inputs without spacing
		ex) program will parse "ls -l" correctly but will not understand "ls-l"

	ctrl+c to kill all processes started via myshell (will not kill myshell itself)
	ctrl+d to exit myshell
