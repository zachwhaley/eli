# eli /ɛli/

Short for Eliana and pronounced Ellie

eli is a simply made text editor written in C

## What You Need

[GNU ncurses](https://www.gnu.org/software/ncurses/ncurses.html)

```bash
$ sudo dnf install ncurses ncurses-libs ncurses-static
```

[GNU GCC](http://gcc.gnu.org/)

```bash
$ sudo dnf install gcc
```

## What it Does

Displays a file on the screen.

Move around with <kbd>h</kbd> <kbd>j</kbd> <kbd>k</kbd> <kbd>l</kbd>.

Enter insert mode with <kbd>i</kbd> and type some things.

Exit insert mode with <kbd>Ctrl-c</kbd>

Use <kbd>Ctrl-s</kbd> to save and <kbd>Ctrl-q</kbd> to quit.

See `eli.c` for more

## How to Use it

```bash
$ make
$ ./eli somefile
```

## Credit

This project was influenced by the [sandy](http://tools.suckless.org/sandy) text editor
