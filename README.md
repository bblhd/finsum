# finsum
Program to sum a series of repeated and/or one-time transactions stored in a file.
Intended for simple budgeting in the modern era.

## Usage
Run using `finsum file [day [month [year]]]`.
Will proceed to add up the list of transactions stored in the given file,
all the way up to, and including, the given date. Any part of the date
that isn't given is substituted with the current date.

Uses a human readable plaintext file format, consisting of a series of
entries seperated by line breaks. Each entry is formatted according to this
EBNF ([W3 XML dialect](https://www.w3.org/TR/xml/#sec-notation)):

```EBNF
entry ::= amount when ( comment )?
amount ::= ( '+' | '-' ) '$' decimal
when ::= 'every' timing 'from' date ( 'to' date )? | 'on' date
comment ::= '#' .*
timing ::= 'day' | 'month' | 'year' | integer ('days' | 'months' | 'years')
date ::= integer month integer
decimal ::= [0-9]* ( '.' ( [0-9] [0-9]? )? )?
integer ::= [0-9]*
month ::= jan | feb | mar | apr | may | jun | jul | aug | sep | oct | nov | dec
```

Which is correct save for a few details regarding restrictions on whitespace
that are difficult to communicate.

### Entry Examples:
- `-$50.30 every 2 weeks from 9 aug 2020`
- `+$50000 every year from 1 jan 2014 to 1 jan 2020`
- `-$12 on 17 sep 2019 #lunch`

## Installing
Compile using `make` or `make compile`, and then install
it to /usr/local/bin by running `make install` as root.
