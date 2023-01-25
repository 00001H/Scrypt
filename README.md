# Scrypt: the embeddable interpreter

## 1. Language

NOTE: Comment processing is NOT part of the intepreter at the moment, and the user is responsible for removing comments.

In this example code, comments are present in the form `#comment` in the code for convenience, and comments are to be removed before the code can be parsed.

NOTE2: The syntax is designed to be close to Python 3, therefore for syntax highlighting purposes code fences will be tagged \`\`\`py instead.

```py
#Literals
#NOTE: Numbers are finite, but at least signed 64-bit.
#Bigint and infinite-precision floats are planned.
number = 3
big_number = 30000
decimal = 30.04
big_decimal = 3e40
print(number)#print() takes only one argument, #varargs not implemented yet.
print(big_number)
print(decimal)
print(big_decimal)

parens = (((((((((((3)))+3))))))))
string = "blahblahblah"
print(parens)
print(string)

concat0 = "Hello"
space = " "
concat1 = "World"
print(concat0+(space+concat1)+space+((concat0+space)+concat1))

exit()#does not take a return code

```
