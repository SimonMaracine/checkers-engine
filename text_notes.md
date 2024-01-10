# Notițe

Text scris rapid, ca prototip. Ulterior trebuie integrat în documentație.

- Am avut nevoie de o bibliotecă de interfață grafică pentru programele GUI auxiliare.
- Inițial am ales FLTK, fiindcă mi-a plăcut faptul că este destul de mic acest framework, dar ulterior am trecut la
  wxWidgets, care este mai mare și mai popular. Nu mi-a plăcut API-ul lui FLTK.
- Primul program GUI care trebuia implementat a fost cel care să faciliteze, mai exact să facă posibil jocul dame
  dintre mine și calculator. L-am numit "Checkers Player". Programul trebuia să primească input de la jucătorul
  utilizator și jucătorul calculator.
- Am început cu desenarea tablei de dame în fereastră, cu detectarea poziției cursorului mouse-ului pe tablă pentru
  input și apoi cu implementarea regulilor jocului dame. Mouse-ul este singurul mod de a interacționa cu jocul.
- În centrul oricărui program software se află datele. Tot ce face un program este să proceseze date, să le manipuleze
  în vreun fel sau altul și să le afișeze. De aceea, știam că cel mai important aspect este să mă asigur că datele
  folosite nu sunt redundante, sunt de tipurile corecte și că sunt stocate în structurile de date potrivite. M-am
  folosit foarte mult de enumerații cu spații de nume pentru a reprezenta tabla, jucătorii, tipurile de mutări etc.
  Unele dintre enumerații conțin valori binare personalizate pentru folosirea operațiilor pe biți în logica codului.
  Fiindcă o mutare poate fi de două tipuri: normală și de captură, și fiindcă cele două tipuri necesită date diferite,
  m-am folosit de uniuni în C++ (engl. "union"), mai exact engl. tagged unions, pentru a face tipul de date mutare să fie
  polimorfic. Am utilizat în mod corespunzător multe tipuri de date și algoritmi din biblioteca standard C++ din
  antetele array, functional, vector, string și algorithm, utility. Astfel n-am avut nevoie să reinventez roata în vreun
  fel sau altul și codul scris este concis.
- Aș fi avut două diferite moduri de a implementa regulile jocului dame, mai exact mutarea pieselor. Atunci când
  utilizatorul încearcă să facă o anumită mutare, aș putea verifica prin multe condiții dacă acea singură
  mutare este validă, iar dacă este, să o permit și să manipulez datele. Ideea aceasta merge, dar ar exista alta mai bună.
  La fiecare tură aș putea, în schimb, să generez toate mutările posibile ale jucătorului la rând. Iar atunci când
  utilizatorul vrea să facă o mutare, pur și simplu să caut mutarea respectivă în lista de mutări generate anterior.
  Validarea devine astfel trivială. Un mare avantaj al acestei abordări este că, știind toate mutările posibile, le pot
  afișa oarecum grafic pe tablă, ceea ce îmbunătățește mult experiența utilizatorului. Am ales a doua variantă.
- După implementarea mutărilor, am implementat și condițiile de pierdere și de remiză, adică tot jocul dame.
- Programul GUI poate oricând reseta tot jocul dame și poate seta starea jocului la o anumită poziție folosind șirurile
  de caractere "FEN" ("Forsyth–Edwards Notation") din notația portabilă de dame ("Portable Draugts Notation", "PDN").
  Acest lucru este foarte important pentru dezvoltarea jocului și a AI-ului, fiindcă îmi permite să testez rapid orice
  fel de situație din joc, orice poziție. Pentru validarea șirurilor de caractere "FEN" am folosit expresiile regulate
  (engl. "regular expressions") implementate în biblioteca standard C++.

TODO structura și organizarea proiectului pe module, versiunea de wxWidgets

- Am început scrierea AI-ului (numit "engine", fiindcă acest termen este des utilizat, ex. "chess engine") prin
  implementarea comunicării dintre acesta și joc (interfața GUI).
- Inițial m-am gândit la un sistem destul de complex de fire de execuție task-uri, în care firul principal (main)
  citește într-o buclă infinită mesaje de la fișierul stdin și deleagă sarcinilie unui al doilea fir de execuție.
  Ideea este ca AI-ul tot timpul să poată citi și procesa mesaje, iar calculele de lungă durată să le facă pe
  un fir separat. Task-urile ar fi fost schimbări simple de stări sau algoritmul minimax etc.
- M-am lovit de problema citirii și scrierii concurente în fișierele stdin, respectiv stdout. Operațiile acestea
  de citire și scriere trebuie sincronizate. Problema mai mare, însă, este că operația de citire există numai cu
  blocare în biblioteca standard C++. Aș fi vrut să citesc fără blocare și să scriu de pe același fir.
- M-am gândit la soluția de a rula bucla infinită de citire pe propriul lui fir de execuție, astfel având
  un fir pentru citire, alt fir pentru scriere, altul pentru execuție a comenzilor lungi, și firul principal pentru
  coordonare între celelalte fire. Pentru aceasta este nevoie de două cozi sincronizate (LIFO) între ele.
- Însă am mai avut alte probleme: odată începută, citirea pe stdin nu poate fi oprită în niciun fel decât
  prin intervenție umană (apăsarea tastei Enter), și nici închiderea elegantă a programului (Ctrl+C) nu putea fi
  implementată.
- M-am întrebat cum au rezolvat alte motoare problema aceasta. Am aruncat o scurtă privire asupra codului
  sursă a proiectului Stockfish și m-a surprins soluția simplă implementată de ei: firul principal citește normal,
  cu blocare într-o buclă și tot ea execută toate comenzile și scrie înapoi în stdout. Iar singurul algoritm
  de lungă durată în execuție, minimax, este cel care rulează într-un fir separat. Sincronizarea trebuie făcută numai
  între acest fir și firul principal. Iar problema închiderii elegante cu Ctrl+C dispare, fiindcă nu (trebuie să)
  tratez acest semnal. Închiderea elegantă a motorului trebuie făcută numai prin comanda quit.
- Aș fi putut folosi funcții de citire fără blocare oferite de POSIX, însă acestea sunt prezente numai în Linux.
  Trebuie să folosesc numai funcții cross-platform (Linux și Windows).
