# track

## Introduction

A little program for tracking how much time I spend doing certain activities.
Very simple, small productivity tool for personal use. You can modify it for
your own purposes by editing the source code.

## Dependencies

You will need a C++20 compiler. This program has not been tested under Windows.

## Management

* To build the program, run `make`
* To install the program to the system, run `make install` as root
* To remove the program from the system, run `make uninstall` as root

## Usage

After installing, run the program from the command line.

To create a time data file or reset statistics (must be run at least once after
initial program installation):

```
$ track -r
```

To start recording that you're programming:

```
$ track program
```

To switch activities to reading:

```
$ track read
```

To stop tracking for the day:

```
$ track stop
```

To print out your statistics:

```
$ track -l # broad-strokes statistics.
$ track -lp # millisecond precision.
```

## Contributing

Don't bother. This is a strictly personal project. You can clone the repository
and use it for your own purposes, if you want.
