# Schedule compiler#

##Thunk scheduling ##

This schedule compiler transforms schedule from textual representation
to a binary one.

Textual for has following format. It consists of N lines. Each line
consists of three columns:

```
ThreadId ThunkId CPU
```

ThreadId and ThunkId fields identify particular thunk. CPU field
specifies a CPU which should run a thunk.

An example of a schedule:

```
1 1 1
1 2 1
1 3 1
1 4 1
1 5 1
2 1 2
3 1 3
4 1 0
5 1 1
1 6 1
```

To compile the schedule simply call compiler with textual schedule as
an input parameter. The result produced in file with '.bin' extension.
