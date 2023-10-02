# Bibliotecă despre inteligență artificială pentru jocul dame

<!-- Bibliotecă *cu* inteligență artificială pentru jocul dame -->

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
de a computeriza astfel de jocuri. Vom discuta despre acest aspect într-o altă secțiune din acest capitol.

### Jocul de strategie dame

În particular, ce ne interesează pe noi este jocul dame (engl. "checkers"). [1] Este un joc abstract, de strategie,
de masă, în doi jucători. Jocul dame are foarte multe variante, în diferite țări și culturi, însă aici,
când spun "dame", mă refer mai exact la varianta americană a jocului (engl. "american checkers" sau "english checkers").
Pentru a computeriza varianta americană a jocului dame, este esențial să cunoaștem regulile exacte ale jocului,
cel puțin cele care ne interesează pe noi: [2]

- Tabla este constituită din opt rânduri și opt coloane, adică șaizeci și patru de pătrățele în carouri.
- Un jucător deține piesele albe sau roșii, iar celălalt deține piesele negre.
- La început, piesele fiecărui jucător sunt așezate pe tablă, pe pătrățelele negre, în părți opuse,
  ca în figura de mai jos.
- Pe rând, jucătorii își vor efectua mutările. Jucătorul negru începe primul.
- Există două mari categorii de mutări: o mutare normală și o mutare de captură. O piesă se poate
  mișca doar în față, pe diagonală, pe un pătrățel liber. Dacă pe un pătrățel adiacent din față se
  află o piesă a oponentului, iar imediat după aceasta este un pătrățel liber, jucătorul trebuie să mute
  piesa acestuia peste piesa adversarului, capturând în același timp (eliminând din joc)
  piesa respectivă. Astfel, dacă  un jucător are posibilitatea de a captura o piesă a adversarului,
  atunci are obligația să o facă. Dacă imediat după o captură, în aceeași tură, jucătorul la
  rând mai poate captura o piesă a adversarului, sărind peste aceasta, este obligat să o facă. Dacă
  un jucător are posibilitatea de a captura piese din sărituri diferite, atunci poate alege care captură
  să o facă. O piesă poate fi sărită o singura dată.
- Dacă o piesă ajunge până la ultimul rând, pe partea opusă a tablei, atunci ea devine "rege" și are
  abilitatea în plus de a se deplasa și de a captura în spate. Ea devine rege doar la finalul turei sale.
- Jocul se sfârșește atunci când un jucător nu mai poate face nicio mutare legală, fie că piesele
  sale sunt blocate, fie că nu mai are piese. În acest caz jucătorul respectiv pierde.
- Jocul se mai poate sfârși și prin remiză atunci când o poziție se repetă pentru a treia oară, sau
  atunci când optzeci de semiture trec fără ca vreun jucător să mute o piesă non-rege sau să captureze
  o piesă a adversarului.

![Poziție inițială](https://upload.wikimedia.org/wikipedia/commons/thumb/f/f1/CheckersStandard.jpg/250px-CheckersStandard.jpg)

Am omis foarte multe reguli care pe noi nu ne interesează, fiindcă ele țin de desfășurarea jocurilor
în competiții. Spre exemplu, nu vom implementa jocul dame cu cronometru sau nu vom implementa remize
propuse.

Există câțiva termeni din câmpul lexical al astfel de jocuri, care trebuie bine definiți:

- Tură și semitură (engl. "turn" și "ply"): O tură sau o tură completă reprezintă o mutare a jucătorului
  negru urmată de o mutare a jucătorului alb. O semitură reprezintă o mutare a unui jucător.
- Mutare (engl. "move"): Reprezintă acțiunea sau acțiunile necesare, efectuate de un jucător, pentru a-și
  termina semitura.
- Poziție (engl. "position"): Reprezintă starea momentană a unui joc dintre semiture. Toate elementele
  care pot face diferența dintre o poziție de alta, împreună reprezintă o poziție a jocului. Aici include
  și jucătorul la rând.
- Remiză (engl. "draw"): Este situația în care jocul se termină fără niciun câștigător.

### Computerizarea jocului dame

Implementarea regulilor jocului dame într-un program software sau într-un joc video este simplă. Însă tematica
acestei lucrări este crearea unui adversar cu care se poate juca dame. În esență, trebuie instrucționat calculatorul
cu regulile acestui joc și cu ce trebuie să facă ca să câștige. În continuare, vom prezenta trei diferite abordări
pentru crearea unui oponent la calculator.

O primă abordare și una naivă ar fi să se analizeze tabla cu toate piesele și, printr-o anumită logică,
să se aleagă o anumită mutare. Cu alte cuvinte, să se scrie un oarecare algoritm. Problema acestei abordări
este că e foarte dificil astfel de implementat un oponent bun, nu garantează că ia în considerare toții
factorii care pot determina câștigul sau pierderea, și nu poate să vadă în ansamblu în ce direcție
o ia jocul, să "vadă" în viitor.

<!-- more, more research for deep learning and references -->
O altă abordare, una relativ recentă și complet opusă primei abordări, este să fie lăsat calculatorul
să își descopere proprii lui algoritmi sau propriile lui tactici de a juca dame. Acest proces se numește
în engleză, pe larg, machine learning sau, mai exact, deep learning. Calculatoarele de astăzi
au ajuns suficient de puternice pentru a procesa enormele operații matematice necesare pentru
machine learning. Pentru aceasta se folosesc de obicei procesoarele grafice (GPU), fiindcă algoritmii
de machine learning sunt, în mare parte, algoritmi numerici și pot fi paralelizați foarte ușor.
Recent, în ultimii ani, jucătorul digital de șah AlphaZero, care este bazat pe machine learning,
a reușit să-l învingă pe cel mai bun jucător digital la vremea aceea,
Stockfish (care nu este bazat pe machine learning), într-un meci de 1000 de runde, cu 155 de câștiguri,
6 pierderi și 839 de remize. [3]

A treia abordare, cea pe care o vom implementa și noi, este folosirea puterii de procesare a calculatorului,
pentru a calcula toate posibilele mutări din partea ambilor jucători până la un punct anume din joc și
toate posibilele rezultate, iar apoi alegerea mișcării celei mai favorabile pentru jucătorul calculator.
Un astfel de algoritm poartă denumirea de minimax.

### Algoritmul minimax

"Minimax este o regulă de decizie utilizată în teoria jocurilor, statistică și filosofie și care constă
în minimizarea pierderii maxime posibile. Alternativ, abordarea poate fi și cea a maximizării câștigului
minim." [4]

Minimax este, de asemenea, un algoritm și este deseori folosit în inteligență artificială pentru jocuri
de tip "turn-based", de doi jucători, cu mutări alternative, potrivindu-se astfel perfect pentru jocul dame.
Se mai poate privi și ca un algoritm de căutare, fiindcă ideea de bază al acestui algoritm este să "caute" cea mai avantajoasă
mutare pentru o anumită poziție a unui joc, pentru un anumit jucător. Minimax este un algoritm generic
și trebuie adaptat pentru un anumit joc, de aceea nu există "algoritmul minimax de șah" sau "algoritmul minimax de dame".
Iar în realitate, deseori sunt implementate extensii peste modelul de bază al algoritmului. Sunt comune
extensii precum (engl.) alpha-beta pruning, sau tabele de transpoziție (engl. "transposition table"),
sau adaptări ale algoritmului pe mai multe fire de execuție. Datorită multitudinilor de moduri în care
poate fi implementat algoritmul minimax, vom explica în alt capitol, în detaliu algoritmul adaptat pentru jocul dame.
În continuare, însă, vom explica algoritmul minimax de bază pe un model simplu, arătând mai bine ideea algoritmului.

Fiecărei poziții a unui joc îi este asociată o valoare sau un scor. Valoarea aceasta este calculată folosind
o funcție de evaluare a poziției (engl. "static evaluation function") și reprezintă cât de bine sau rău
ar fi pentru un anume jucător, dacă s-ar afla în poziția respectivă. Se presupune că jucătorul alb vrea un scor
cât mai mare, iar jucătorul negru vrea un scor cât mai mic. Astfel, jucătorul alb, dintre toate mutările
legale, va efectua mutarea care maximizează șansele de a ajunge la poziția cea mai avantajoasă, cu
scorul cel mai mare. La fel, jucătorul negru va alege mutarea cea mai nefavorabilă pentru alb, care
maximizează șansele de a ajunge la poziția cu scorul cel mai mic. Deci un scor cu valoarea 0 înseamnă
că niciun jucător nu este în avantaj, un scor pozitiv înseamnă că jucătorul alb este în avantaj, iar
un scor negativ semnifică avatanj pentru negru. Algoritmul se poate vizualiza sub forma unui arbore,
numit arborele jocului.

<!-- more and better explanation needed -->
<!-- better image and pseudocode -->
În imaginea de mai jos [4] se află arborele unui joc în care fiecare jucător are maxim două posibile mutări
la dispoziție în fiecare tură. Nodurile cerc reprezintă pozițiile în care la rând este jucătorul care
maximizează scorul, iar nodurile pătrat reprezintă pozițiile jucătorului care minimizează scorul.
De regulă, arborele este limitat în înălțime, datorită numărului imens de poziții ce trebuie calculate.
Astfel, nodurile terminale dintr-un astfel de arbore, fie că sunt nodurile finale ale jocului sau nu,
reprezintă pozițiile care trebuie analizate static cu ajutorul unei funcției euristice de evaluare. În imagine,
odată ce se cunosc valorile a două noduri copil, algoritmul atribuie nodului părinte maximul sau minimul,
care depinde de jucător. Spre exemplu, primele două noduri terminale din stânga au valorile 10,
respectiv +inf. Jucătorul minimizant va alege valoarea minimă, adică 10. La finalul algoritmului,
primului nod, adică poziției de pornire, îi este atribuită o valoare (-7) care reprezintă scorul cel mai
favorabil, scorul maxim pe care îl poate obține jucătorul maximizant, presupunând că ambii
jucători joacă perfect.

![Arbore algoritm](https://upload.wikimedia.org/wikipedia/commons/thumb/6/6f/Minimax.svg/600px-Minimax.svg.png)

Pseudocodul algoritmului minimax arată cam așa: [5]

```javascript
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

În capitolele următoare, pentru concizie și simplitate, vom numi adversarul calculator, (engl.) "AI",
adică inteligență artificială.

### Implementări existente de minimax pentru jocul dame

<!-- FIXME show research; 20 most popular -->
<!-- write some more in this chapter -->
În prezent, pe internet, se observă că mulți au scris programe care, pe lângă jocul dame,
implementează și un oarecare AI, majoritatea folosind algoritmul minimax. Foarte puțini au încercat
să-și creeze propriul algoritm pentru AI și la fel de puțini au folosit tehnici de machine learning.

De asemenea, de observat este că aproape toți au implementat algoritmul minimax pentru jocul dame
în limbaje de programare destul de nepotrivite precum Python, JavaScript și Java. Spun că acestea
sunt nepotrivite, fiindcă nu sunt limbaje de programare performante în viteză de execuție. Pentru a
scrie un adversar performant pentru jocul dame, două lucruri sunt esențiale: funcția de evaluare
statică trebuie să fie bună, iar adâncimea de căutare trebuie să fie mare. Însă, mărirea
adâncimii crește exponențial timpul de căutare, ceea ce este o problemă. De aceea, este foarte
important ca algoritmul să fie rapid în execuție și eficient în memorie.

Noi vom alege o altă abordare pentru scrirea adversarului pentru jocul dame și ne vom focaliza pe câteva
obiective, nu doar pe o implementare superficială. În secțiunea următoare vom vorbi despre ideile pe
care vrem să le aplicăm, lucrurile pe care vrem să le implementăm și țintele la care vrem să ajungem.

### Implementarea algoritmului minimax în această lucrare

<!-- here I present what I want to do and hopefully accomplish -->
<!-- I create the AI as a separate, usable library in C++, I use alpha-beta pruning, multithreading and much more -->
<!-- I create testing and developing tools for the AI, I improve the AI by comparing it with himself -->
<!-- unit testing -->
În această lucrare, în loc să dezvoltăm AI-ul legat de vreo anumită aplicație de interfață grafică pe
Linux, vrem să îl creăm sub formă de bibliotecă statică sau dinamică în limbajul C++. Avantajul
acestei abordări este că oricine dorește să scrie un joc de dame, fie pe desktop, fie pe telefon mobil
sau orice altă platformă, poate cu ușurință să integreze acest AI în aplicația lor.

Un alt avantaj este că în acest fel e simplu să creăm aplicații de interfață grafică auxiliare
cu scopul de a testa AI-ul în toate felurile și de al compara cu alte versiuni de ale sale.
Vom creea AI-ul sub formă de bibliotecă dinamică, care va fi încărcată la rulare de către aplicația de testare.
Astfel, păstrând bibiotecile dinamice compilate, vom putea testa rapid orice versiune a adversarului
calculator. Similar, pentru a testa AI-ul cu o versiune precedentă de a sa, vom încărca respectivele
fișiere bibliotecă de pe disk.

Pentru a dezvolta AI-ul și pentru a-l face mai rapid și mai bun, avem nevoie să cunoștem dacă o anumită
modificare asupra codului îmbunătățește sau nu ceva. De aceea, pentru a îmbunătăți constant AI-ul,
îl vom compara mereu cu ultima versiune stocată. Dacă schimbarea este bună, atunci o păstrăm.
De asemenea, vom ține evidența versiunilor stocate pentru a vizualiza mai târziu evoluția AI-ului.

Vom implementa o optimizare importantă numită (engl.) alpha-beta pruning, ideea căreia este de a
tăia multe calcule inutile în căutarea celei mai favorabile mutări. Vom implementa și un tabel de
transpunere, pentru a lua în considerare și situații similare precedente. AI-ul va fi configurabil.
Mulți parametri vor putea fi ajustați, ceea ce va face posibilă schimbarea abilității AI-ului și
chiar a comportamentului acestuia.

O problemă dificilă care trebuie rezolvată este că algoritmul, odată pornit, nu mai poate fi oprit
până nu își termină căutarea, iar dacă totuși este oprit prematur, înainte de a calcula toate pozițiile,
nu putem avea garantat rezultatul cel mai bun. Similar, dacă pe durata unui joc de dame, AI-ul este configurat să
caute mereu la aceeași adâncime, atunci câteodată îi va ține prea mult timp să caute, iar altădată
nu își va utiliza mai mult din timpul la dispoziție pentru a căuta o mutare mai bună. Cu alte cuvinte,
trebuie creat algoritmul în așa fel încât oricând este oprit, să returneze un rezultat bun, și să își
folosească cât mai mult din timpul pus la dispoziție, căutând cât mai adânc. Vom scrie algoritmul
constrâns și în adâncime, dar și în timp.

Calculatoarele de astăzi conțin procesoare cu mai multe nuclee, ceea ce face posibilă executarea
proceselor și a firelor de execuție cu adevărat în paralel. Pentru o performanță mai bună, în cele
din urmă ne vom folosi de mai multe fire de execuție pentru a căuta mult mai rapid cea mai favorabilă mutare.

<!-- TODO buy and read "Computer Systems: A Programmer's Perspective" -->

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
[2] <https://www.wcdf.net/rules/rules_of_checkers_english.pdf>  
[3] <https://en.wikipedia.org/wiki/AlphaZero>  
[4] <https://ro.wikipedia.org/wiki/Minimax>  
[5] <https://en.wikipedia.org/wiki/Minimax>
