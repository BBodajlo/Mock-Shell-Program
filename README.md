**My Shell**

CS 214: Systems Programming

**Group Members:**

- Adam Katinsky (ark180)
- Blake Bodajlo (bjb262)


**Overview:**

MyShell is a custom implementation of a linux based shell. It takes input either from standard input within the program (interactive mode), or an argument when running the program which is a file containing commands (batch mode). It runs standard programs provided by the user/bin library. It also handles piping of multiple commands as well as file redirection. Proper syntax is determined by the shell and will only execute commands if the syntax is valid.


**Extensions:**

Escape Characters (\)
Multiple Pipes


**General Notes:**

In running the provided SyntaxTest.txt, it is a “special case” and will not behave like a normal batch file. This file is to show the syntax of the tokens provided across the whole file. This was done to avoid piping errors, command not found errors, and redirection creating new files when they shouldn’t for testing. Using the commands themselves will behave abnormally. If the syntax test commands are desired to be used further, put everything into the SyntaxTest.txt file. Commands: echoSyn, echoCommand, echoNext, and echoPrev. 

**Make File:**

The make file can be utilized by calling “make mysh”. It also can be called with “clean mysh” to get rid of what the making of mysh includes.


**Our Shell’s Functionality**

In all shells, there are tons of edgy cases for you to consider. This is how our shell handles some of the edge cases:

**Redirects:**

Redirects take priority over pipes. This means running something like “ls > foo | cat”, cat will receive no input as ls writes to the redirected file named foo. This is the same for “ls | cat < foo”. Ls will run and will send information through the pipe, however cat won’t read it and reads the information from foo.

Redirects also operate on a “last seen” type of rule. This being, if you redirect 10 times in a command, the 10th one will be the one our code listens to. For example “ls > foo > baz > bar > bat” will put the results of ls into the file bat, as it was the last redirection mentioned.

**Wildcards:**

Wildcards search and expand the tokens that we store. If no expansion is found, nothing changes. With this expansion, we expand the token linked list where the wildcard was placed, and insert all of the new arguments there. 

If a wildcard happens to be after a redirect, we treat the first file found as the “redirect file” and the rest as arguments to the command as mentioned in class. For example “cat > \*.txt” can expand to “cat > foo.txt bar.txt” which will use bar.txt as an argument, and foo.txt as the output file.

**Built-In Commands:**

Built in commands end up calling the “fork” function if pipes are used in the command, leaving them to be still ran by us, but in a subprocess. Because of this, the cd command only works if not ran with pipes. If cd is ran on the same line of a pipe, similarly to pwd, it will change/get that subprocess’s directory.

**Newlines:**

All of our errors output to the window with newline characters, and lots of the commands also have newline characters to add new lines after running the command. Because of this, we opted to not put the “mysh>” line on a new line, as many things already use newlines, so we did not want to have blank lines.

Because of this, running things like “cat foo” where foo does not have a new line at the end of the file prints “mysh>” on the same line as the last line in foo.


**Testing Plan:** 

1) Syntax/Token Testing
   1) The shell works off of user input so proper syntax is needed in order for commands to properly be executed. This is done through tokenizing what the user has input so that the execution portion of the shell is able to recognize and perform what the user has input. To achieve tokenization, a struct exists that contains the following structure: A list of all of the tokens where each token contains the string of the token itself, the command that the token is pointing to, a pointer to the next token, and a pointer to the previous token. The following checks will need to be had:
      1) Tokens
         1) Regular works separated by spaces
         1) Special characters ( |, <, >) being separated from other tokens
         1) Quotes (“ “) making everything in between a single token
      1) Commands - Commands are tokens that arguments point to so any time a pipe or redirection is seen, the next token will be a command
         1) Arguments should point to the their command
         1) Everytime a special character is seen, the next token becomes a command
      1) Next 
         1) Every token should point to the immediate next
      1) Previous
         1) Every token should point to the immediate previous
      1) Escape characters
         1) Escaping any regular, non special character 
         1) Escaping a space 
         1) Escaping a special character 
         1) Escaping an escape 
         1) Escaping a newline character \n
   1) To check the above, the following should be true
      1) Tokens
         1) Any stream of non-special strings should be separated into their own token
         1) Any time a special character is seen, it should be it’s own token
            1) Seeing the special character before another character
            1) Seeing a special character in the middle/end of a string
         1) Everything after a single quote should be together in a single token until another quote is seen; a token will not be valid unless two quotes are seen
      1) Commands
         1) Any non-special token after the first should point to the first token
         1) Any time a special character is seen, the next token becomes a new command and arguments after now point to the new command
      1) Next
         1) For every single token type, it should always have a next token with the last token having a NULL token.
      1) Previous
         1) For every single token type, it should point to the previous token with the first token pointing to NULL
      1) Escape characters
         1) Non special characters are unaffected by the escape and should be added normally
         1) Should nullify a space and add a space to a token like a quote
         1) Should nullify it’s special meaning and add it to the token
         1) Should add the escape to the token
         1) Should nullify the new line and **not** add it to the token


1) To test the above cases
   1) Tokens - Using the echoSyn command in the execution of mysh which is a custom version of echo to print out tokens basically the same as echo
      1) Calling “echoSyn foo bar” should result in the terminal printing: foo bar 

1) Special characters
   1) Calling “ecoSyn foo |bar” should result in: T: foo | bar
   1) Calling “echoSyn foo|bar” should result in T: foo | bar
1) Quotes
   1) Calling “echoSyn “foo < bar”” should result in: T: foo<bar
   1) Calling “echoSyn “foo \_bar|”” baz should result in: T: foo \_bar| baz 
   1) Calling “echoSyn “foo bar  ” Should result in no tokens since the quote wasn’t finished
   1) Calling “echoSyn foo bar “ “ should result in: foo bar with no quote since it is not a valid token
1) Commands - Using the echoCommand command in the execution of mysh will print out the commands that each token points to
   1) Calling “echoCommand foo bar” should result in: echoCommand echoCommand echoCommand - meaning they all point to the first token which is the command
   1) Calling “echoCommand foo bar | baz loo” should result in echoCommand echoCommand echoCommand echoCommand baz baz - the special character point to the command before it
1) Next - Using the echoNext command within mysh will print out the next token name of each token
   1) Calling “echoNext foo bar | baz “qux quux” > thud” should result in foo bar | baz (“qux quux”) > thud NULL - Note that it shows “qux quux” as that is how the command line prints out the quoted tokens; it doesn't actually contain quotes. 
1) Previous - Using the echoPrev command within mysh will print out the previous token that each token is pointing to
   1) Calling “echoPrev foo bar | baz < qux” will result in: NULL echoPrev foo bar | baz < 
1) Escape Characters - Using the echoSyn command within mysh will show individual tokens
   1) Calling “echoSyn fo\o” should result in T: foo
   1) Calling “echoSyn foo\ baz” should result in T: foo baz
   1) Calling “echoSyn foo\| baz” should result in T: foo| T:baz - where | is now just a regular character
      1) Calling “echoSyn "foo\| \" baz"” “ should result in T: foo| “ baz - where the quote is escaped
   1) Calling “echoSyn foo\\ bar” should result in T: foo\ T:bar
   1) Added the test case in the write where calling echoSyn foo bar\ baz\<qu\ux\\>spam results in T: foo T: bar baz<quux\ T: > T: spam
   1) Calling “echoSyn foo\” should result in T: foo - where the \n is not added and it completes the token (Not included in the batch file since 
   1) In batch mode, having the file look like: 
      <br>echoSyn hello\ 
      <br>cat text.txt
      
      Should result in  T:hellocat  T: text.txt - where the newline is nullified and the token is not pushed like its all on the same line







