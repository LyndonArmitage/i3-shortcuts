# i3-shorcuts

This is a simple project to parse the i3 config for keyboard shortcuts and
return them in some sensible way.

The idea is that you can use the output to create a cheat sheet for your lesser
used shortcuts.

## Progress

So far the program can:

- Find a valid config file
- Parse out all the simple `bindsym` lines
- Convert the `bindsym` lines into a struct for programmatic use.

I am currently considering what kind of format the program should output.  
It makes sense to output text, perhaps 1 line per command. Although I could
also create a simple demo using X11 to show the output.  
Other things include generating a simple conky config.
