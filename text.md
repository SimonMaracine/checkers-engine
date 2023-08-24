# Bibliotecă de inteligență artificială pentru jocul dame

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
este pentru un jucător, dacă s-ar afla în poziția respectivă. Se presupune că jucătorul alb vrea un scor
cât mai mare, iar jucătorul negru vrea un scor cât mai mic. Astfel, jucătorul alb, dintre toate mișcările
legale, va efectua mișcarea care va duce la poziția cu scorul cel mai mare. La fel, jucătorul negru
va alege mișcarea cea mai nefavorabilă pentru alb, adică cu scorul cel mai mic.

<!-- TODO game tree explanation, pseudocode -->

### Implementări existente de minimax pentru jocul dame

...

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
