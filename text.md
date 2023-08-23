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
Pentru a computeriza varianta americană a jocului dame, este esențial să se cunoască regulile exacte ale jocului [1]:

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

### Computerizarea jocului dame

Implementarea regulilor jocului dame într-un program software, într-un joc video este simplă. Însă tematica
acestei lucrări este crearea unui adversar cu care să jucăm dame. În esență, trebuie instrucționat calculatorul
cu regulile acestui joc și cu ce trebuie să facă ca să câștige. În continuare, voi prezenta trei diferite abordări
pentru crearea unui oponent la calculator.

O primă abordare și una naivă ar fi ca, dintre toate mișcările posibile la un moment dat, să se aleagă
una bazat pe o anumită logică, de exemplu, verificându-se toate piesele de pe tablă și constatând anumite
mișcări bune sau rele. Problema acetei abordări este că e foarte dificil de implementat, nu garantează
că ia în considerare toții factorii care pot determina câștigul sau pierderea, și nu poate să vadă
în ansamblu în ce direcție o ia jocul.

<!-- more, more research for machine learning -->
O altă abordare, una relativ recentă și complet opusă primei abordări, este să fie lăsat calculatorul
să își descopere proprii lui algoritmi sau propriile lui tactici de a juca dame. Acest proces se numește,
pe larg, în engleză, machine learning. Calculatoarele de astăzi au ajuns suficient de puternice pentru
a procesa enormele operații necesare pentru machine learning. Pentru aceasta se folosesc de obicei
procesoarele grafice (GPU), fiindcă algoritmii de machine learning sunt, în mare parte, algoritmi numerici
și pot fi paralelizați foarte ușor. Recent, în ultimii ani, jucătorul digital de șah AlphaZero a reușit
să-l învingă pe cel mai bun jucător digital la vremea aceea, Stockfish, într-un meci de 1000 de runde,
cu 155 de câștiguri, 6 pierderi și 839 de remize. [2]

A treia abordare, cea pe care o voi implementa și eu, este folosirea puterii de procesare a calculatorului,
pentru a computa toate posibilele mișcări pe care le pot face ambii jucători până la un punct anume
și alegerea mișcării celei mai favorabile pentru calculator. Acest algoritm se numește minimax.

### Algoritmul minimax

...

### Implemenații existente de minimax pentru jocul dame

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
