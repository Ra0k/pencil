# Pencil
Self-implemented programming language. Right now, it only supports 64 bit systems, and automatic builds are only avaiable for Mac.


## Requirements 
G++, Flex, GNU Bison

## Running the example

### Build the compiler

  Compile using the `Makefile`: `$ make`

### Compile & Run

1.  First compile the source code using the `Makefile` in `/example`
2.  Run the program `./program`

## Example Program
```
program {
  a, i, divider ~ integer
  divisible ~ boolean

  read(a) 
  
  i <- 2
  divisible <- false
  while not divisible {
    if a mod i = 0 {
      divisible <- true
      divider <- i
    }
    i <- i + 1
  }
  if divisible {
    write(divisible)
    write(divider)
  } else {
    write(divisible)
  }
}
```
