# Connect Four

This is a connect-four application for academic porpuses. The focus of this application is in the use of IPC mechanisms such as pipes and shared memory in a POSIX environment with C. Feel free to study, fork and reuse the code present in this repository.

## Running the software

The package comes with a Makefile to ease the building process, so to build the binary just call the `make` command:

    $ make

This will create the `connect-four` binary in the current directory. It will also show you any error during the building process. Then you can run it like this:

    $ ./connect-four

## FAQ

#### Do I need anything else to run this software?

This software doesn't have any dependencies besides a C compiler such as `gcc` or `clang`. Make sure to run it on a POSIX compliant environment, like GNU/Linux or Mac OS X, I haven't tested it on Windows.

#### Why are you using IPC methods in such a simple game?!

Don't look at me, this was a requirement put by the assignment I made this for, so I figured a creative way of using pipes and shared memory to fulfill the task. If you want to read a simplier code, go through the commit history of `src/main.c`, I made the whole game before I started to worry about IPC. Some bugs of the older version are fixed in the IPC version though, so be careful.

#### I found a bug! can I fix it?

Sure you can, and after you do that go ahead and make a pull request for everyone's benefit!

