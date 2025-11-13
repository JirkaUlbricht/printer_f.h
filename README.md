# printer_f.h
Have you ever wanted to waste your precious paper? Do you want to immortalize your mistakes when debugging? Did you want to make the print in printf(); live to it's name?

Then printer_f.h is the header for you!

## What does it do?

Simple, printer_f.h will print out any printf(); output on paper 

## What is the point of this?

I got bored

## But what about the environment?

🤓

## How to use printer_f.h?

Simply add the printer_f.h file to your project just like any other header file and you are good to go!

## Supported OS

- Windows (Print to PDF also supported)
- Linux (Not tested, should work, no Print to PDF in WSL tho)
- MacOS (Not tested, got no device to test it on)

## Usage

### Hello ~~world~~ wasted toner

```c
#include "printer_f.h"
#include <stdio.h>

int main() {
    printf("Hello wasted toner!");

    return 0;
}

```

### Simple example on how to lose your printing privileges anywhere (approx. 152 pages of A4 paper)

```c
#include "printer_f.h"
#include <stdio.h>

int main() {
    int i = 1;
    while (i <= 10000) {
        printf("Line %d", i);
        ++i;
    }

    return 0;
}

```

## Licence and terms of use
[GNU GPL v2.0](https://choosealicense.com/licenses/gpl-3.0/)

Use it to your heart's content, just don't claim it as your own :\)

---------------

# printer_f.h
Už jste někdy chtěli vyplýtvat Váš drahocenný papír? Chcete zvěčnit své chyby při debugování? Chtěli jste, aby slovo print v printf(); reálně tisklo?

Pak je printer_f.h header pro Vás!

## Co to má vlastně dělat?

Jednoduše, printer_f.h bude tisknou výstupy z printf(); přímo na papír

## Proč jsem to vůbec vytvořil?

Nudil jsem se.

## A co na to životní prostředí?

🤓

## Jak používat printer_f.h?

Jednoduše. Stačí přidat soubor printer_f.h do Vašeho projektu stejně jako jakýkoliv jiný header a jste ready!

## Podporované OS

- Windows (Print to PDF podpora)
- Linux (Netestováno, ale mělo by to snad fungovat, Print to PDF ve WSLku nefunguje)
- MacOS (Netestováno, nemám v čem)

## Příklad použití

### Ahoj ~~světe~~ vyplýtvaný tonere

```c
#include "printer_f.h"
#include <stdio.h>

int main() {
    printf("Ahoj vyplýtvaný tonere!");

    return 0;
}

```

### Jednoduchý příklad jak přijít privilegia k tisknutí kdekoli (cca. 152 ks A4)

```c
#include "printer_f.h"
#include <stdio.h>

int main() {
    int i = 1;
    while (i <= 10000) {
        printf("Řádek %d", i);
        ++i;
    }

    return 0;
}

```

## Licence a podmínky k používání
[GNU GPL v2.0](https://choosealicense.com/licenses/gpl-3.0/)

Klidně využívejte, kde chcete, jen ale printer_f.h nevydávejte za svůj :\) 
