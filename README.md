
Hello everyone! SigFLUP from the Uber Leet Hacker Force here. 
Hope you like this one- it's a command-line unixporn screen-shot
uploader. I had a lot of fun looking over pcaps to write it. I
hope this is a lesson about how you really shouldn't write parsers.

INSTALLATION:

after extracting in the directory that yesplz extracted to type these commands: 

./configure
make
make install 

ought to do the trick. You'll need libjpeg, X11 and pthreads-- since 
these things are pretty much standard on any distribution dependencies 
shouldn't be a problem. You will also need flex/lex to compile it. 

RUNNING:

$ yesplz --help

will give you a list of options. Here's what they do:

** if ~/.yesplz.cfg doesn't exists at least -album -user and -title must
** be specified. All arguments on the command-line overwrite what's in
** the configuration file

 -t <timeout> 	 this is the timeout in seconds for each operation,
                 the default is 20 seconds 

 -up <timeout>   this is the timeout is seconds for the uploading part,
                 the default is 100 seconds

 -album          this is the album you wish to upload to, this must 
                 be present

 -cat <name>     This is the category, if you have alternate categories
                 on your account. The default is "no category"

 -user <name>    this is the username of your account-- you need to 
                 set-up an account on www.unixporn.ml first.

 -title <text>   this is the title of your screen-shot

 -desc  <text>   this is a description of your screen-shot (optional)

 -tag <text>     these are any tags you want to use (optional)
 
 -disp <name>    this is an alternate X display you want to take a
                 shot from, the default is what's in the environment
                 variable DISPLAY

  -qual <0-100>  This specifies the jpeg quality (upload size) default is 100

  -save          This saves any arguments you specify with this argument
                 as a configuration file in ~/.yesplz.cfg The next time
		 yesplz runs and if it finds that file it uses the arguments
		 you "-save"ed with as the defaults. For example, if you 
		 type:
                 
		 $ yesplz -user sigflup -album "nice album" -save

		 from then on all you have to do is type:

                 $ yesplz -title "I love pants"

                 to upload a screen-shot

  -wait <time>   Wait a specified number of seconds before doing anything
                 this will give you a chance to make your desktop purty

  -dump <file>   dumps sessions to a file for use in debugging


and that's it. Here's an example of how to use it:

$yesplz -user sigflup -title "this is a screen-shot" -album screenshots

it will then ask you for your password...
 
sigflup's password:

and then show you the status of what it's doing..

shot..
delicious front page cookie
delicious login cookie
login
forum token and timestamp
upload
logout

If you have any questions or maybe it doesn't work for you please email 
me at thea@theadesilva.com and we can work out your problems.

yours,
 SigFLUP
            
