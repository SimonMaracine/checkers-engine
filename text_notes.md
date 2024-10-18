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

<!-- TODO structura și organizarea proiectului pe module, versiunea de wxWidgets -->

- Am început scrierea AI-ului (numit "engine", fiindcă acest termen este des utilizat, ex. "chess engine") prin
  implementarea comunicării dintre acesta și joc (interfața GUI).
- Inițial m-am gândit la un sistem destul de complex de fire de execuție cu task-uri, în care firul principal (main)
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
  tratez acest semnal. Închiderea elegantă a motorului trebuie făcută numai printr-o comandă.
- Aș fi putut folosi funcții de citire fără blocare oferite de POSIX, însă acestea sunt prezente numai în Linux.
  Trebuie să folosesc numai funcții cross-platform (Linux și Windows).

- Am scris restul AI-ului prin implementarea crudă și incompletă a algoritmului minimax pentru dame. Algoritmul este
  constituit din funcția principală recursivă, din generarerea mutărilor posibile și din evaluarea statică a poziției.
- Codul pentru generarea mutărilor l-am putut lua și adapta de la GUI, fiindcă intenționat îl scrisesem astfel încât
  să poată fi refolosit.
- Nu am stat mult timp ca să scriu o funcție de evaluare foarte bună, fiindcă nu acesta este scopul în această etapă.
  De aceea am luat în calcul doar numărul de piese ale celor doi jucători astfel: fiecare piesă valorează un punct;
  piesele celui negru fiind negative (-1), iar celui alb fiind pozitive. Astfel, la calcularea evaluării unei poziții,
  jucătorul în avantaj este cel cu evaluarea negativă pentru negru, și pozitivă pentru alb, adică cel cu numărul cel
  mai mare de piese.
- Am știut de la bun început cât de important este ca datele pe care le procesez și le manipulez în algoritm să nu fie
  redundante, să fie compacte și cât de des posibil prezente în memoria cache a procesorului. Astfel, la fiecare
  invocare a algorimtului de căutare, aloc toate variabile necesare pe stiva firului de execuție secundar. Am folosit
  tipuri de date de dimensiuni reduse pentru a reduce numărul de octeți necesari pentru a reprezenta aceleași
  informații, asigurându-mă astfel că mai multe date încap în memoria cache. Mai multe optimizări voi face mai târziu,
  într-un alt capitol.
- Crearea unui fir de execuție pentru fiecare invocare a algoritmului nu este un lucru bun, pentru că, de regulă,
  firele de execuție implementate în biblioteca standard C++ sunt native (engl. "native threads", "system threads").
  Acestea sunt obiecte ale sistemului de operare și sunt administrate de către acesta. Firele de execuție verzi
  (engl. "green threads", "managed threads") diferă de cele native prin faptul că sunt administrate de o mașină
  virtuală sau un alt "runtime". Mai multe fire verzi pot rula concurent pe un singur fir nativ. Acestea pot fi
  planificate după un alt sau mai mulți algoritmi, pe când firele native sunt planificate de sistemul de operare.
  Iar motivul pentru care trebuie evitată crearea multor fire de execuție native este că această operație este "grea"
  și cere intervenția sistemului de operare prin apeluri sistem. În comparație, firele verzi pot fi foarte ușor create
  și distruse în număr mare.
- Creez la început un singur fir de execuție pentru algoritmul de căutare și îl folosesc pe toată durata de viață a
  programului AI. În execuția firului se află o buclă principală care așteaptă să primească de lucru, dacă nu are ceva
  de făcut. Codul arată cam așa (pseudocod):

  ```c++
  while (true) {
    wait_sleeping();

    if (!running) {
      break;
    }

    Move best_move {search()};

    // Do something with best_move...
  }
  ```

  Aș fi putut elimina apelul funcției wait_sleeping și, în schimb, să tot verifice la începutul buclei dacă este ceva de
  căutat. Însă această variantă ar fi saturat complet un nucleu al procesorului. M-am folosit în schimb de anumite
  obiecte de sincronizare din biblioteca standard pentru a face procesorul să doarmă în timp ce așteaptă și pentru
  a-l trezi imediat când trebuie să ruleze algoritmul. Mai exact, am utilizat clasele std::condition_variable și
  std::mutex. Funcția wait_sleeping returnează doar atunci când firul de execuție trebuie terminat sau atunci când
  trebuie să execute funcția de căutare.

- Pentru structurarea codului motorului AI am folosit spații de nume în fiecare fișier de cod sursă. M-am folosit
  extensiv de clasele din biblioteca standard C++ pentru a-mi facilita implementarea tuturor lucrurilor. Am utilizat
  excepții și am scris clase acolo unde am considerat că a avut mai mult sens.
- Am inventat un protocol de comunicare între interfața grafică (GUI) și motor, pe care aceștia să-l implementeze.
  Ei comunică prin mesaje text, prin fișierele stdin și stdout ale motorului. Există două clase de mesaje: mesaje pe
  care le transmite GUI către motor, și mesaje pe care le transmite înapoi motorul către GUI.
- Un scenariu de comunicare dintre GUI și motor arată cam așa: GUI creează subprocesul motor și îi transmite primul
  mesaj, numit și comandă: INIT, după care înteroghează motorul despre parametrii configurabili pe care îi oferă acesta
  prin comanda GETPARAMETERS. Motorul răspunde interfeței grafice cu mesajul PARAMETERS, care conține o listă de
  parametri, fiecare cu tipul acestuia. Interfața grafică poate configura oricare parametru setându-i o altă valoare
  decât cea prestabilită prin comanda SETPARAMETER. Interfața grafică vrea acum să înceapă jocul și trimite comanda
  GO motorului ca să facă prima mutare. Motorul răspunde înapoi cu mesajul BESTMOVE care conține mutarea pe care tocmai
  a jucat-o pe tablă. Interfața grafică joacă pe tabla sa mutarea motorului, după care face ea o mutare, transmițând-o
  ulterior înapoi motorului prin comanda MOVE, ca s-o joace și el pe tabla sa. În acest fel, se poate desfășura jocul
  dame între un calculator și o ființă umană sau între calculator și calculator, prin intermediul acestui protocol.
- Intenționat am specificat protocolul cât mai simplu, lăsând cât mai multă libertate programatorului să o implementeze
  cum dorește acesta. Există cazuri de comportament nedefinit, dar protocolul este destul de simplu de a fi
  utilizat corect, evitându-se astfel probleme de comportament nedefinit.
- A fost nevoie ca protocolul să nu specifice o limită asupra dimensiunii mesajelor, fiindcă comanda NEWGAME poate
  specifica un număr foarte mare de mutări jucate inițial, iar impunând o limită mare de 16384 de octeți, spre exemplu,
  nu ajută prea mult.
- Am specificat mesajul WARNING, pe care îl poate trimite motorul oricând interfeței grafice pentru cazuri
  excepționale. Însă nu este obligat să îl folosească vreodată. Folosirea acestei facilități de debugging este la
  alegerea celui care scrie motorul folosind acest protocol.

- Aplicația Checkers Player are sarcina de a crea un subproces, care să fie motorul și să comunice cu acesta. Inițial
  am vrut să utilizez o bibliotecă cross-platform pentru a-mi facilita această funcționalitate în limbajul C++
  pentru platformele Linux și Windows. Am găsit o bibliotecă pe GitHub care făcea exact ce-mi trebuia, dar am avut
  ulterior problema că citirea de la proces era cu blocare, însă îmi trebuia neapărat să fie fără blocare. Am
  semnalat acest lucru dezvoltatorilor, însă între timp am decis să scriu direct apelurile sistem necesare pentru
  crearea suprocesului și comunicarea cu acesta. Nu am putut folosi funcțiile standard popen și pclose, fiindcă
  popen îmi crea o singură cale de comunicare, însă aveam nevoie de comunicare bidirecțională. De aceea a trebuit
  să utilizez apelurile sistem pipe, fork, dup2, execv, select, read, write, waitpid, close. Câteva dintre aceste
  funcții sunt definite și în standardul POSIX, însă acest fapt nu ajută la nimic. Singura consecință este
  că aplicația GUI nu mai este cross-platform și că trebuie rescrisă porțiunea aceasta de cod cu apelurile
  sistem din Windows.

- Motorul de joc dame primește comenzi de la interfața grafică GUI și le execută fie setând o anumită stare, fie
  trimițând înapoi mesaje interfeței grafice.
- Algoritmul minimax este singurul proces, singura comandă care rulează într-un fir de execuție separat. Fiindcă
  în acest fir trebuie trimise mesaje, scriindu-se în fișierul stdout, și fiindcă între timp și firul principal poate
  trimite mesaje, operație aceasta de scriere trebuie protejată de un simplu mutex.

- Inițial am scris algoritmul minimax într-un fel foarte simplu. De date am avut nevoie doar de un tablou de
  treizeci și două de elemente reprezentând tabla cu piesele ei. Fiecare pătrățel negru al tablei poate avea una
  din cinci stări: poate fi gol, poate avea o piesă neagră simplă, o piesă neagră rege, o piesă albă simplă sau
  o piesă albă rege.
- Algoritmul arăta cam așa (pseudocod):

  ```c++
  Eval minimax(Player player, int depth, int plies_from_root) {
    if (depth == 0 || is_game_over(board)) {
      return static_evaluation(board);
    }

    if (player == Player::Black) {
      Eval min_evaluation {MAX_EVALUATION};

      auto moves {generate_moves(board, player)};

      if (moves.empty()) {
        return static_evaluation(board);
      }

      for (Move& move : moves) {
        make_move(board, move);

        Eval evaluation {minimax(Player::White, depth - 1, plies_from_root + 1)};

        unmake_move(board, move);

        if (evaluation < min_evaluation) {
          min_evaluation = evaluation;

          if (plies_from_root == 0) {
            best_move = move;
          }
        }
      }

      return min_evaluation;
    } else {
      Eval max_evaluation {MIN_EVALUATION};

      auto moves {generate_moves(board, player)};

      if (moves.empty()) {
        return static_evaluation(board);
      }

      for (Move& move : moves) {
        make_move(board, move);

        Eval evaluation {minimax(Player::Black, depth - 1, plies_from_root + 1)};

        unmake_move(board, move);

        if (evaluation > max_evaluation) {
          max_evaluation = evaluation;

          if (plies_from_root == 0) {
            best_move = move;
          }
        }
      }

      return max_evaluation;
    }
  }
  ```

- Se observă că toate operațiile se fac pe variabila board, dintre care cele care modifică starea sunt make_move și
  unmake_move. Funcția unmake_move anulează modificarea făcută de make_move. Momentan nu este nicio problemă cu
  această abordare, însă dificultăți apar în momentul în care vreau să implementez regula (engl.) threefold
  repetition, care necesită să rețin istoricul jocului.
- Threefold repetition implică compararea poziției curente cu toate pozițiile precedente pentru a confirma dacă această
  poziție a apărut sau nu pentru a treia oară.
- Fiindcă nu a existat niciun mod simplu sau cel puțin eficient de a implementa threefold repetition în codul de
  minimax curent, am decis să merg în altă direcție.
- Am legat conceptul de poziție în jocul dame cu conceptul de nod din arborele apărut în algoritmul minimax.

  ```c++
  Eval minimax(Player player, int depth, int plies_from_root, SearchNode& current_node) {
    if (depth == 0 || is_game_over(current_node)) {
      return static_evaluation(current_node);
    }

    if (player == Player::Black) {
      Eval min_evaluation {MAX_EVALUATION};

      auto moves {generate_moves(current_node, player)};

      if (moves.empty()) {
        return static_evaluation(current_node);
      }

      for (Move& move : moves) {
        SearchNode new_node;
        fill_node(new_node, current_node);

        play_move(new_node, move);

        Eval evaluation {minimax(Player::White, depth - 1, plies_from_root + 1, new_node)};

        if (evaluation < min_evaluation) {
          min_evaluation = evaluation;

          if (plies_from_root == 0) {
            best_move = move;
          }
        }
      }

      return min_evaluation;
    } else {
      Eval max_evaluation {MIN_EVALUATION};

      auto moves {generate_moves(current_node, player)};

      if (moves.empty()) {
        return static_evaluation(current_node);
      }

      for (Move& move : moves) {
        SearchNode new_node;
        fill_node(new_node, current_node);

        play_move(new_node, move);

        Eval evaluation {minimax(Player::Black, depth - 1, plies_from_root + 1, new_node)};

        if (evaluation > max_evaluation) {
          max_evaluation = evaluation;

          if (plies_from_root == 0) {
            best_move = move;
          }
        }
      }

      return max_evaluation;
    }
  }

  struct SearchNode {
    Board board;
    const SearchNode* previous;
  };
  ```

- De data aceasta, starea este reținută în structura SearchNode, unde se află tabla. Algoritmul începe cu un nod
  inițial cu poziția pentru care să returneze o evaluare și o mutare. Apoi, pentru fiecare mutare generată se crează
  un nou nod cu o copie a poziției precedente și cu un pointer către nodul precedent. Mutările jucate nu sunt anulate.
  Se formează astfel un arbore cu noduri legate între ele prin pointeri. Memoria nu este gestionată prea ineficient,
  întrucât nodurile sunt alocate pe stiva cadrului curent al algoritmului recursiv.
- În acest mod este mult mai ușor să implementez threefold repetition.
- Această idee de abordare mi-a venit atunci când mă uitam în mare la codul motorului de șah Stockfish.

<!-- TODO de asemenea, este posibilă rularea algoritmului pe mai multe fire de execuție -->

- Ca să pot testa anumite funcții din motorul de dame, am transformat aproape tot codul motorului într-o bibliotecă
  statică, pe care o leg cu executabilul, iar apoi am scris executabile separate pentru testare. Acestea, desigur,
  sunt legate cu biblioteca statică a motorului și se folosesc de framework-ul Google Test.
- Cel mai important lucru de testat în acest moment este generarea mutărilor. Am scris o funcție care pentru o anumită
  adâncime generează toate posibilele poziții ale jocului și returnează numărul acestora. Apoi am comparat câteva
  astfel de numere cu ceea ce alții au calculat deja înaintea mea: C. A. Pickover, The Math Book, Sterling, NY, 2009;
  see p. 512.

- La un moment dat nu am fost mulțumit de programele de interfață grafică pe care le scrisesem. Aveam de terminat
  programul care compară două motoare. Am decis atunci să rescriu interfața grafică de la zero în limbajul Python,
  folosind TKinter ca framework. Motivele acestei decizii au fost că wxWidgets în C++ era dificil de folosit și
  că gestionarea subproceselor în C++ necesita apeluri sistem, ceea ce însemna să scriu două diferite implementări,
  pentru Linux și Windows.
- Ceea ce Python cu TKinter mi-a oferit în cele din urmă este un cod mai ușor de citit și de modificat, o dezvoltare
  mult mai accelerată și o bibliotecă cross-platform foarte robustă și simplă pentru gestionarea subproceselor.
- Am recscris primul program, care acum este mult mai bun decât cel precedent.
- Am decis ca programul care compară două motoare să fie un simplu program CLI (Command Line Interface), care să
  ruleze în fundal. Acesta atunci generează la final un raport cu toate meciurile jucate, cu rezultatele, parametrii
  folosiți etc. împreună cu toate mutările jucate, pentru redarea meciurilor. Astfel, al doilea program de interfață
  grafică este devine mai simplu, trebuind doar să redea fiecare moment al unui meci.
