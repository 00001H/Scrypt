# ABOUT COMMENTS IN THE CODE

## 1. Function calls

The stack is depicted this way: `#i` where i is the index(starting at 1) of the current function in the stack, with 1 being the top-most(e.g. the `main()` function)

Important value(s) follows, seperated with a comma.

(Optional) the function name may be put right after the stack info.

Example of a frame:
`#1/1 parseStr string: exam|ple, pos: 4`

Sometimes, the whole stack is shown, like this:
`{main}/{parse file: blah.txt}/{parse_str string: exam|ple}`

Calls may be shown this way:

```scrypt-comment/stacktrace
#1 main
  -> #2 askUserForFilename
  -> #2 parse file: blah.txt
       -> #3 parseStr string: exam|ple
```

Note: the arrow is 2 spaces to the right of the `#`.
Note 2: the arrow may be replaced with unicode characters `U+2514 └` `U+2500 ─` for an "extra fancy" diagram.
e.g.

```scrypt-comment/stacktrace
#1 main
  └─ #2 parse file: blah.txt
```
