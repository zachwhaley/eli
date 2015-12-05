# eli (short for Eliana)

A bare bones text editor written in C

## Libraries

[ncurses](https://www.gnu.org/software/ncurses/ncurses.html)

```bash
$ sudo dnf install ncurses ncurses-libs ncurses-static
```

[GCC](http://gcc.gnu.org/)

```bash
$ sudo dnf install gcc
```

## What it Does

Displays a files content on the screen.

Move around with the arrow keys.

Type some things, and watch them appear.

Use <kbd>Ctrl-w</kbd> to save and <kbd>Ctrl-q</kbd> to quit.

## How to Use it

```bash
$ make
$ ./eli somefile
```

## Credit

This project was influenced by the [sandy](http://tools.suckless.org/sandy) text editor
