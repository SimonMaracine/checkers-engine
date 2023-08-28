# Bibliotecă despre inteligență artificială pentru jocul dame

## Introducere

...

## Aspecte teoretice

### Jocuri de masă și de strategie

Jocurile de masă au existat de când e lumea. Încă din antichitate, pentru divertisment, oamenii
au inventat diverse jocuri de masă și mai ales de strategie, în doi jucători, la multe observându-se chiar un șablon:
o tablă pe care se desfășoară jocul și doi oponenți, fiecare cu un set de piese de o anumită culoare,
de obicei alb și negru. Există prea multe jocuri de acest fel și prea multe variații ale acestora pentru a
fi scrise toate într-un singur capitol. Însă, printre cele mai cunoscute se enumeră: șah, dame, moară și go.
Toate aceste jocuri au în comun aspectul important că se joacă în ture, odată un jucător, iar apoi celălalt și tot
așa. Datorită acestui model simplu - adică doi jucători care joacă pe rând pe o tablă - este destul de ușor
de a computeriza astfel de jocuri. Voi discuta despre acest aspect într-o altă secțiune din acest capitol.

### Jocul de strategie dame

În particular, ce ne interesează pe noi este jocul dame (engl. "checkers"). Este un joc abstract, de strategie,
de masă, în doi jucători. Jocul dame are foarte multe variante, în diferite țări și culturi, însă aici,
când spun "dame", mă refer mai exact la varianta americană a jocului (engl. "american ckeckers" sau "english checkers").
Pentru a computeriza varianta americană a jocului dame, este esențial să se cunoască regulile exacte ale jocului: [1]

- Tabla este constituită din opt rânduri și opt coloane, adică șaizeci și patru de pătrățele
  în carouri, ca și la tabla de șah.
- Un jucător deține piesele albe sau roșii, iar celălalt deține piesele negre.
- La început, piesele fiecărui jucător sunt așezate pe tablă, pe pătrățelele negre, în părți opuse,
  ca în figura de mai jos.
- Pe rând, jucătorii vor muta piesele pe diagonală, doar în față, pe orice pătrățel liber. Dacă pe un
  pătrățel adiacent din față se află o piesă a oponentului, iar imediat după aceasta este un pătrățel liber,
  jucătorul poate muta piesa acestuia peste piesa adversarului, capturând în același timp (eliminând din joc)
  piesa adersarului. În acest mod, mai multe piese de ale oponentului pot fi capturate într-o singură
  tură, cu condiția ca mutările piesei și capturile să fie succesive. Jucătorul negru începe primul.
- Dacă o piesă ajunge până la ultimul rând, pe partea opusă a tablei, atunci ea devine "rege" până
  la sfârșitul jocului sau până e capturată și are abilitatea în plus de a se deplasa și de a captura
  și în spate.
- Jocul se sfârșește atunci când fie un jucător nu mai are piese pe tablă, fie nu mai poate face nicio
  mișcare legală. În acest caz jucătorul respectiv pierde.

<!-- draw by threefold repetition, proposed draws, resignments -->

![Poziție inițială](https://upload.wikimedia.org/wikipedia/commons/thumb/f/f1/CheckersStandard.jpg/250px-CheckersStandard.jpg)

Există câțiva termeni din câmpul lexical al astfel de jocuri în general, care trebuie bine definiți:

- Tură și semitură (engl. "turn" și "ply"): O tură sau o tură completă reprezintă o mișcare a jucătorului
  negru urmată de o mișcare a jucătorului alb. O semitură reprezintă o mișcare a unui jucător.
- Mișcare (engl. "move"): Reprezintă acțiunea sau acțiunile necesare, efectuate de un jucător, pentru a-și
  termina semitura.
- Poziție (engl. "position"): Reprezintă starea momentană a unui joc dintre semiture. Toate elementele
  care pot face diferența dintre o poziție de alta, împreună reprezintă o poziție a jocului. Aici include
  și jucătorul la rând.
- Remiză (engl. "draw"): Este situația în care jocul se termină fără niciun câștigător.

### Computerizarea jocului dame

Implementarea regulilor jocului dame într-un program software, într-un joc video este simplă. Însă tematica
acestei lucrări este crearea unui adversar cu care se poate juca dame. În esență, trebuie instrucționat calculatorul
cu regulile acestui joc și cu ce trebuie să facă ca să câștige. În continuare, voi prezenta trei diferite abordări
pentru crearea unui oponent la calculator.

O primă abordare și una naivă ar fi să se analizeze tabla cu toate piesele și, printr-o anumită logică,
să se aleagă o anumită mișcare. Cu alte cuvinte, să se scrie un oarecare algoritm. Problema acestei abordări
este că e foarte dificil de implementat, nu garantează că ia în considerare toții factorii care pot determina
câștigul sau pierderea, și nu poate să vadă în ansamblu în ce direcție o ia jocul, să "vadă" în viitor.

<!-- more, more research for machine learning -->
O altă abordare, una relativ recentă și complet opusă primei abordări, este să fie lăsat calculatorul
să își descopere proprii lui algoritmi sau propriile lui tactici de a juca dame. Acest proces se numește,
pe larg, în engleză, machine learning. Calculatoarele de astăzi au ajuns suficient de puternice pentru
a procesa enormele operații matematice necesare pentru machine learning. Pentru aceasta se folosesc de obicei
procesoarele grafice (GPU), fiindcă algoritmii de machine learning sunt, în mare parte, algoritmi numerici
și pot fi paralelizați foarte ușor. Recent, în ultimii ani, jucătorul digital de șah AlphaZero a reușit
să-l învingă pe cel mai bun jucător digital la vremea aceea, Stockfish, într-un meci de 1000 de runde,
cu 155 de câștiguri, 6 pierderi și 839 de remize. [2]

A treia abordare, cea pe care o voi implementa și eu, este folosirea puterii de procesare a calculatorului,
pentru a calcula toate posibilele mișcări din partea ambilor jucători până la un punct anume din joc și
toate posibilele rezultate, iar apoi alegerea mișcării celei mai favorabile pentru jucătorul calculator.
Acest algoritm poartă denumirea de minimax.

### Algoritmul minimax

"Minimax este o regulă de decizie utilizată în teoria jocurilor, statistică și filosofie și care constă
în minimizarea pierderii maxime posibile. Alternativ, abordarea poate fi și cea a maximizării câștigului
minim." [3]

Minimax este, de asemenea, un algoritm și este deseori folosit în inteligență artificială pentru jocuri
de tip "turn-based", de doi jucători, cu mișcări alternative, potrivindu-se astfel perfect pentru jocul dame.
Se mai poate privi și ca un algoritm de căutare, fiindcă ideea de bază al acestui algoritm este să "caute" cea mai avantajoasă
mișcare pentru o anumită poziție a unui joc, pentru un anumit jucător. Minimax este un algoritm generic
și trebuie adaptat pentru un anumit joc, de aceea nu există "algoritmul minimax de șah" sau "algoritmul minimax de dame".
Iar în realitate, deseori sunt implementate extensii peste modelul de bază al algoritmului. Sunt comune
extensii precum (engl.) alpha-beta pruning, sau tabele de transpoziție (engl. "transposition table"),
sau adaptări ale algoritmului pe mai multe fire de execuție. Datorită multitudinilor de moduri în care
poate fi implementat algoritmul minimax, voi explica în alt capitol, în detaliu algoritmul adaptat pentru jocul dame.
În continuare, însă, voi explica algoritmul minimax de bază pe un model simplu, arătând mai bine ideea algoritmului.

Fiecărei poziții a unui joc îi este asociată o valoare sau un scor. Valoarea aceasta este calculată folosind
o funcție de evaluare a poziției (engl. "static evaluation function") și reprezintă cât de bine sau rău
este pentru un anume jucător, dacă s-ar afla în poziția respectivă. Se presupune că jucătorul alb vrea un scor
cât mai mare, iar jucătorul negru vrea un scor cât mai mic. Astfel, jucătorul alb, dintre toate mișcările
legale, va efectua mișcarea care va duce la poziția cu scorul cel mai mare. La fel, jucătorul negru
va alege mișcarea cea mai nefavorabilă pentru alb, adică cu scorul cel mai mic. Algoritmul se poate
vizualiza sub forma unui arbore, numit arborele jocului.

<!-- more and better explanation needed -->
<!-- better image and pseudocode -->
În imaginea de mai jos [4] se află arborele unui joc în care fiecare jucător are maxim două posibile mișcări
la dispoziție în fiecare tură. Nodurile cerc reprezintă pozițiile în care la rând este jucătorul care
maximizează scorul, iar nodurile pătrat reprezintă pozițiile jucătorului care minimizează scorul.
De regulă, arborele este limitat în înălțime, datorită numărului imens de poziții ce trebuie calculate.
Astfel, nodurile terminale dintr-un astfel de arbore, fie că sunt nodurile finale ale jocului sau nu,
reprezintă pozițiile care trebuie analizate static cu ajutorul unei funcții euristice. În imagine,
odată ce se cunosc valorile a două noduri copil, algoritmul atribuie nodului părinte maximul sau minimul,
care depinde de jucător. Spre exemplu, primele două noduri terminale din stânga au valorile 10,
respectiv +inf. Jucătorul minimizant va alege, valoarea minimă, adică 10. La finalul algoritmului,
primului nod, adică poziția de pornire, îi este atribuită o valoare (-7) care reprezintă scorul cel mai
favorabil, scorul maxim pentru pe care îl poate obține jucătorul maximizant, presupunând că ambii
jucători joacă perfect.

![Arbore algoritm](https://upload.wikimedia.org/wikipedia/commons/thumb/6/6f/Minimax.svg/600px-Minimax.svg.png)

Pseudocodul algoritmului minimax arată cam așa: [4]

```text
function minimax(node, depth, maximizingPlayer) is
    if depth = 0 or node is a terminal node then
        return the heuristic value of node
    if maximizingPlayer then
        value := −∞
        for each child of node do
            value := max(value, minimax(child, depth − 1, FALSE))
        return value
    else (* minimizing player *)
        value := +∞
        for each child of node do
            value := min(value, minimax(child, depth − 1, TRUE))
        return value
```

### Implementări existente de minimax pentru jocul dame

<!-- write some more in this chapter -->
Căutând în zilele de astăzi pe internet (în anul 2023), se observă că mulți au scris programe care,
pe lângă jocul dame, implementează și un oarecare AI, majoritatea folosind algoritmul minimax.
Foarte puțini au încercat să-și creeze propriul algoritm pentru AI și la fel de puțini au folosit
tehnici de machine learning.

De asemenea, de observat este că aproape toți au implementat algoritmul minimax pentru jocul dame
în limbaje de programare destul de nepotrivite precum Python, JavaScript și Java. Spun că acestea
sunt nepotrivite, fiindcă nu sunt limbaje de programare performante în viteză de execuție. Pentru a
scrie un adversar performant pentru jocul dame, două lucruri sunt esențiale: funcția de evaluare
statică trebuie să fie bună, iar adâncimea de căutare trebuie să fie cât mai mare. Însă, mărirea
adâncimii crește exponențial timpul de căutare, ceea ce este o problemă. De aceea, este foarte
important ca algoritmul să fie rapid în execuție și eficient în memorie.

Eu voi alege o altă abordare pentru scrirea adversarului pentru jocul dame și mă voi focaliza pe alte
obiective, nu doar pe o implementare superficială. În capitolul următor voi vorbi despre ideile pe
care vreau să le aplic, lucrurile pe care vreau să le implementez și țintele pe care vreau să
le ating.

### Implementarea și ideile mele

<!-- here I present what I want to do and hopefully accomplish -->
<!-- I create the AI as a separate, usable library in C++, I use alpha-beta pruning, multithreading and much more -->
<!-- I create testing and developing tools for the AI, I improve the AI by comparing with himself -->
<!-- unit testing -->
...

## Proiectarea și dezvoltarea bibliotecii

<!-- I present how I do the things I proposed earlier -->
<!-- I also present the failures -->
...

## Experimente și rezultate

...

## Concluzii

...

## Bibliografie

[1] <https://en.wikipedia.org/wiki/Checkers>  
[2] <https://en.wikipedia.org/wiki/AlphaZero>  
[3] <https://ro.wikipedia.org/wiki/Minimax>  
[4] <https://en.wikipedia.org/wiki/Minimax>
