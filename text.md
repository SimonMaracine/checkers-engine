# Inteligență artificială pentru jocul dame

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
  să o facă. O piesă poate fi sărită o singură dată.
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
un scor negativ semnifică avantaj pentru negru. Algoritmul se poate vizualiza sub forma unui arbore,
numit arborele jocului.

<!-- more and better explanation needed -->
<!-- better image and pseudocode -->
În imaginea de mai jos [4] se află arborele unui joc în care fiecare jucător are maxim două posibile mutări
la dispoziție în fiecare tură. Nodurile cerc reprezintă pozițiile în care la rând este jucătorul care
maximizează scorul, iar nodurile pătrat reprezintă pozițiile jucătorului care minimizează scorul.
De regulă, arborele este limitat în înălțime, datorită numărului imens de poziții ce trebuie calculate.
Astfel, nodurile terminale dintr-un astfel de arbore, fie că sunt nodurile finale ale jocului sau nu,
reprezintă pozițiile care trebuie analizate static cu ajutorul unei funcții euristice de evaluare. În imagine,
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

Noi vom alege o altă abordare pentru scrierea adversarului pentru jocul dame și ne vom focaliza pe câteva
obiective, nu doar pe o implementare superficială. În secțiunea următoare vom vorbi despre ideile pe
care vrem să le aplicăm, lucrurile pe care vrem să le implementăm și țintele la care vrem să ajungem.

### Implementarea programului AI

În loc să dezvoltăm AI-ul legat de vreo anumită aplicație de interfață grafică pe
Linux, vrem să îl creăm separat de orice interfață grafică, astfel încât să fie reutilizabil.
Avantajul acestei abordări este că oricine dorește să scrie un joc de dame, fie pe desktop, fie pe telefon mobil
sau pe orice altă platformă, poate cu ușurință să integreze acest AI în aplicația lor, fără să-și
creeze propriul AI, dacă aceasta este de dorit. Ar exista două metode pentru implementarea acestui lucru.

#### AI sub formă de bibliotecă

Prima și cea mai lesne metodă este dezvoltarea AI-ului sub forma unei biblioteci statice sau dinamice
(în limbajul C++). Aplicația dame cu interfață grafică ar utiliza această bibliotecă, compilând-o
cu executabilul sau încărcând-o la rulare ca și bibliotecă dinamică.

Biblioteca ar trebui să expună un API, un set de definiții, structuri și funcții, pentru a
face posibilă utilizarea acesteia. Ca și o idee, funcțiile dintr-un astfel de API ar arăta cam așa:

```c++
void initialize();
void uninitialize();
void search(const SearchInput& input, Result& result);
void set_parameter(std::string_view parameter, int value);
```

În esență, AI-ul ar trebui inițializat și finalizat, fiindcă conține stare persistentă între
invocări ale acestuia. Ar avea funcția de cea mai mare interes, adică cea de căutare a unei mutări bune
pentru o anumită poziție. Astfel, structura SearchInput de mai sus ar conține toate datele necesare
pentru identificarea clară a poziției în joc, iar structura Result ar conține datele de ieșire. Și
AI-ul ar trebui să fie configurabil în diferite feluri.

Din perspectiva dezvoltării bibliotecii, această metodă este simplă. Avantajele acestei abordări sunt,
în primul rând, simplitatea (nu este dificil de creat o bibliotecă) și faptul că AI-ul bibliotecă,
fiind compilat cu executabilul, rămâne oarecum invizibil sau cel puțin inutilizabil
pentru alte scopuri decât pentru acel specific executabil. Însă, dezavantajul mare este că aplicația
de interfață grafică dame trebuie scrisă în limbajul C++ pentru a utiliza biblioteca, ceea ce
pune greutate pe cel care scrie aplicația dame. Pentru bibliotecă ar putea fi
scrise legături (engl. "bindings") pentru alte limbaje de programare, de exemplu Python
(utilizând API-ul Python/C), ceea ce ar face posibilă scrierea jocului dame în limbajul Python,
însă aceasta nu este o soluție universală, pentru toate limbajele de programare, și nu este nici
una foarte simplă.

#### AI sub formă de aplicație plus un protocol de comunicare

O altă metodă pentru crearea AI-ului separat de o interfață grafică este dezvoltarea acesteia sub
forma unui simplu executabil. Ideea este ca aplicația dame de interfață grafică să lanseze AI-ul
executabil într-un proces copil și să comunice cu acesta prin fluxuri IO. AI-ul executabil primește
comenzi și răspunde înapoi cu rezultate. Similar cu API-ul unei biblioteci, AI-ul executabil
trebuie să definească un protocol pentru comunicare, care ar putea arăta cam așa:

```text
init <args...>
set_position <position>
search
set_parameter <parameter> <value>
is_ready

result <move>
info <information>
ready
```

Primele cinci ar fi comenzi sub formă de text trimise de aplicația dame către AI, iar ultimele trei
ar fi răspunsuri către aplicația dame. Aplicația de interfață grafică ar trebui să citească și
să proceseze răspunsurile. De asemenea, ar trebui să existe și o formă de sincronizare.

Avantajul acestei metode este că aplicația dame de interfață grafică poate fi scrisă în orice
limbaj de programare. Trebuie numai să aibă acces la un mod de a lansa procese și de a comunica cu ele,
ceea ce nu este dificil.

Ideea aceasta este inspirată de la Stockfish, un motor de șah foarte popular (engl. chess engine),
care în esență este un executabil. Rudolf Huber și Stefan Meyer-Kahlen au inventat un protocol
universal de comunicare dintre interfețe grafice și motoare de șah numit "UCI"
("Universal Chess Interface"). [6] Un protocol similar poate fi definit și pentru jocul dame.

### Implementarea AI-ului în această lucrare

<!-- here I present what I want to do and hopefully accomplish -->
<!-- I create the AI as a separate, usable library in C++, I use alpha-beta pruning, multithreading and much more -->
<!-- I create testing and developing tools for the AI, I improve the AI by comparing it with himself -->
<!-- unit testing -->
În această lucrare noi vom implementa a doua metodă, fiindcă este mai flexibilă pentru cel care vrea să
utilizeze AI-ul nostru și fiindcă prezintă niște probleme interesante de rezolvat. Alegerea de a
scrie AI-ul ca un program individual aduce cu sine și niște beneficii mari precum faptul că
în acest fel e simplu să creăm aplicații de interfață grafică auxiliare
cu scopul de a testa AI-ul în toate felurile și de al compara cu alte versiuni de ale sale.
AI-ul executabil va fi încărcat și rulat de către o aplicație de testare.
Astfel, păstrând executabilele compilate, vom putea testa rapid orice versiune a adversarului
calculator. Similar, pentru a compara AI-ul cu o versiune precedentă de a sa, vom rula respectivele
fișiere executabile.

Pentru facilitarea dezvoltării AI-ului și pentru a-l face mai rapid și mai bun, avem nevoie să cunoaștem
dacă o anumită modificare asupra codului îmbunătățește sau nu ceva. De aceea, pentru a îmbunătăți constant AI-ul,
îl vom compara mereu cu ultima versiune stocată. Dacă schimbarea este bună, atunci o păstrăm.
De asemenea, vom ține evidența versiunilor stocate pentru a vizualiza mai târziu evoluția AI-ului.

Vom implementa o optimizare importantă numită (engl.) alpha-beta pruning, ideea căreia este de a
elimina multe calcule inutile în căutarea celei mai favorabile mutări. Vom implementa și un tabel de
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

## Proiectarea și dezvoltarea programului AI

<!-- I present how I do the things I proposed earlier -->
<!-- I also present the failures -->
### Aplicațiile auxiliare de interfață grafică

<!-- FIXME nope, use first person singular -->
Primul pas pentru dezvoltarea AI-ului a fost să avem la dispoziție o aplicație grafică pentru a ne juca
jocul dame, ceea ce a dus la nevoia unui framework de interfață grafică. Pentru că scriem programele
auxiliare în limbajul preferat C++, avem o gamă destul de largă de framework-uri de unde putem alege.
Însă, avem nevoie de o bibliotecă cross-platform, pentru a rula aplicațiile pe Linux și pe Windows,
și una gratuită. Câteva biblioteci populare de acest fel sunt: Qt, wxWidgets, Dear ImGui, FLTK, U++, Nana. [7]
Inițial am ales să încercăm FLTK, fiindcă nu este o bibliotecă uriașă și fiindcă nu avem nevoie de una
sofisticată. Dar am schimbat ulterior biblioteca cu wxWidgets, pentru că nu ne-a plăcut interfața și inflexibilitatea
acesteia. wxWidgets este, într-adevăr, o bibliotecă mult mai mare, dar este și foarte populară. Este o alegere mai bună,
fiindcă avem deja experiențe din trecut cu această bibliotecă.

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
[6] <https://www.shredderchess.com/chess-features/uci-universal-chess-interface.html>  
[7] <https://terminalroot.com/the-7-best-cpp-frameworks-for-creating-graphical-interfaces/>
