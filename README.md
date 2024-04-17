README - TEMA2 - PCOM

Implementare: (rezumat)
- folosind materialele si bucati de schelet din LABORATOR,
am initializat pentru inceput o conexiune intre un client
care se va conecta la server (atat pentru TCP cat si pentru
UDP) folosind socketii intr un mod corespunzator
- de asemenea, dezactivez algoritmul lui Nagle de fiecare
data cand creez un socket (atat in server, cat si in client)
- urmeaza o bucla while unde tratez special urmatoarele
cazuri (folosind poll pentru multiplexarea I/O) :
1) serverul doreste sa se inchida si trimite clientilor
conectati un semnal pentru a se deconecta
2) primim comenzi de la clientii TCP si le analizam in
functie de cerinta
3) se posteaza topicuri de la clientii UDP
4) conectam noi clienti TCP la server
5) trimitem topurile primite in mod corespunzator conform
cerintei
- ca si protocol de nivel aplicatie, trimit de fiecare data
o structura block care va anunta clientul ce comanda primeste
(exit sau receive a message) si apoi fie se va deconecta, fie
va fi pregatit sa primeasca o structura de date specifica
fiecarui mesaj personalizat intr un topic cu dimensiunea potrivita
- pe langa structurile mentionate mai sus care fac incadrarea
mesajelor trimise sa fie eficienta, pentru fiecare client si 
topic creez 2 structuri specifice cu scopul de a le gestiona
corespunzator (mai multe detalii in comentarii)
- pentru ideea de subscribe unsubscribe ma folosesc de o masca
reprezentata sub forma de vector pentru a face activ/inactiv
topicul la care este abonat un client
- pentru ideea de topic uri trimise unui client, generez o cheie
unica fiecarui utilizator conectat si o adaug in topic ul trimis
- pentru ideea de store and forward, creez ideea de monitorizare
a timpului folosind o variabila globala care se incrementeaza la
diferite actiuni, astfel, fiecare topic si fiecare client va avea
o quanta de timp pentru a realiza cu succes trimiterea mesajelor
- utilizez de asemenea foarte multe verificari pentru conceptul de
cod defensiv, iesind din program cu statusul EXIT_FAILED la o 
anomalie