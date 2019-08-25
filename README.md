# Kowalski

## Analisi

### Descrizione del sistema
Bot per la gestione di trasporto collettivo.

Il bot simula una parte delle funzionalità di una piattaforma per la prenotazione di trasporto collettivo.
Il bot consente a chi offre un passaggio di poter inserire la disponibilità di posti, verso una determinata destinazione in una specifica data.

Nello specifico Kowalski si occupa di gestire le prenotazioni, aggiunte, modifiche e cancellazione dei passaggi e guidatori nel sistema. Il bot si avvale di [uTron](https://gitlab.com/NicoNex/utron) per le comunicazioni con la [Telegram Bot API](https://core.telegram.org/bots/api).
La gestione degli utenti avviene tramite l'utilizzo del `chat_id`, variabile intera a 64 bit che definisce ciascun utente univocamente.

Ad ogni nuova connessione con un utente Telegram la funzione `run_dispatcher` (definita in `utron/dispatcher.h`) provvederà a creare una nuova istanza di tipo `struct bot *` gestita tramite l'uso di una linked list e di una hash table che funge da meccanismo di caching per le sessioni più attive in modo da ottimizzare il tempo di accesso alla memoria.

Il comportamento del bot è definito dal codice in `main.c`.
Nel `main` sono implementati la `struct bot`, la funzione `new_bot` e la procedura `update_bot` definiti in `utron/bot.h`.

La funzione `new_bot` si occupa di allocare in memoria e inizializzare la `struct bot`, infine di restituirne il puntatore.

La procedura `update_bot` è il cuore pulsante del bot. Essa infatti viene invocata dalla procedura `run_dispatcher` ogni qualvolta ci sarà un update sull'API di Telegram e dovrà sempre terminare l'esecuzione. Il bot quindi può essere descritto come un automa a stati finiti che in base agli input di utenti da Telegram modifica lo stato in cui si trova in modo da gestire gli input successivi in maniera definita.
Questo comportamento si ottiene mediante la continua modifica di variabili quali: `bot.mode` e `bot.state` implementate nella `struct bot`.
Gli stati e modalità in cui il bot può trovarsi, sono definiti da `enum modes` per `bot.mode` e `enum commands` per `bot.state`.

Per il corretto funzionamento di Kowalski inoltre sono stati implementati i moduli `drivers.c` e `travels.c` definiti rispettivamente in `include/drivers.h` e `include/travels.h`.
Questi moduli si occupano di caricare da disco, aggiornare e gestire la lista di guidatori e passaggi.
Tali liste sono salvate sul disco come file `json` in `res/drivers.json` e `res/travels.json`. 
Durante l'esecuzione del bot tali file vengono costantemente aggiornati con i valori presenti in memoria, con politiche di write-through.
In questi moduli sono anche presenti tutte le funzioni base per la rimozione, aggiunta e modifica dei rispettivi oggetti (`struct driver` e `struct travel`) nelle linked list usate.

Il modulo `list.c` definito in `include/list.h` contiene tutte le funzioni e procedure necessare per una corretta gestione della lista.
Il tipo definito in `include/list.h` chiamato `list_t` infatti è fondamentale per la gestione di tutte le liste presenti nel bot.

In `main.c` vengono usate due variabili globali 
```c
volatile list_t drivers;
volatile list_t travels;
```
in modo che siano condivise da tutte le istanze di `struct bot`; inoltre dato che `run_dispatcher` chiama la funzione `update_bot` su un nuovo thread ad ogni update da telegram, sono state usate variabili `volatile` per una maggiore thread-safety.

I guidatori in drivers.c sono contraddistinti da un token univoco rappresentato da un numero intero a 64 bit.
Tale numero rappresenta lo UNIX Epoch time, quindi il numero di secondi trascorsi dal 1 gennaio 1970 ore 00:00, assegnato automaticamente in fase di aggiunta del guidatore all'elenco, viene usato anche per collegare i passaggi ai rispettivi guidatori.

Infine il modulo `filehandler.c` definito in `include/filehandler.h` racchiude la funzione responsabile del caricamento dei json contenenti le informazioni su guidatori e passaggi dal disco in memoria.


### Requisiti funzionali

| Codice | Nome                    | Descrizione                                                                   |
|--------|-------------------------|-------------------------------------------------------------------------------|
| R01    | Bot                     | Implementazione dell'oggetto bot.                                             |
| R02    | Passaggi                | Elenco di tutti i passaggi disponibili.                                       |
| R03    | Guidatori               | Elenco di tutti i guidatori.                                                  |
| R04    | I/O                     |  Gestione dell'I/O e interpretazione dello stesso.                            |
| R05    | Strutturazione dei dati |  Strutturazione dei dati in maniera funzionale alle specifiche del programma. |

### Strumenti di sviluppo
Il bot è stato realizzato su ambiente GNU/Linux con CPU Intel i7 di terza generazione, architettura x86_64 e 12GB di RAM.
L'ambiente di sviluppo adottato è stato Sublime Text 3 come editor di testo semplice, e gcc come compilatore versione 9.1.0.

Per compilare il sistema è necessario innanzitutto di clonare questo repo con
```sh
git clone --recursive https://github.com/NicoNex/kowalski
```
facendo attenzione che ci siano tutti i file in `utron/`.

Successivamente verificare che le dipendenze **json-c** e **libcurl** siano presenti nel sistema, altrimenti installarle.

Per installarle su **Ubuntu** e derivate basterà il comando:
```sh
sudo apt install libcurl-dev libjson-c-dev
```

Una volta installate basterà digitare nel terminale `make` trovandosi nella root del progetto. Verrà così generato il binario `kowalski` che potrà essere eseguito.

Una volta eseguito il bot sarà in funzione e potrà essere usato contattandolo allo username @crplng_bot collegato al token di telegram scritto nel file *TOKEN* presente nella root del progetto.
Di conseguenza il file *TOKEN* è necessario per l'esecuzione del programma.

## Progettazione

### Progettazione dei tipi e strutture dati
| Nome                | Tipologia             | Descrizione                                                                 | Campi                                                                                                                                                                                                                           |
|---------------------|-----------------------|-----------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| bot                 | struct                |  Descrive il contesto di esecuzione di una sessione con un utente Telegram. |  mode: int,  state: int,  seatstmp: int,  sort_mode: int,  chat_id long,  drvtmp: struct driver *,  trvtmp: struct travel *                                                                                                     |
| modes               | enum                  |  Contiene tutte le modalità in cui il bot si può trovare.                   | NO_OP,  SELECT_DRIVER,  SELECT_TRAVEL,  RATE,  GET_NAME,  GET_AGE,  GET_VEHICLE,  DEL_DRIVER,  GET_DESTINATION,  GET_DATE,  GET_DRIVER_ID,  GET_PRICE,  GET_SEATS,  MOD_TRAVEL,  DEL_TRAVEL,  GET_SORT_MODE,  GET_QRY,  CONFIRM |
| commands            | enum                  |  Contiene la classificazione dei comandi   in esecuzione.                   | DEFAULT,  RATE_DRV,  ADD_DRV,  MOD_DRV,  DEL_DRV,  ADD_TRV,  MOD_TRV,  DEL_TRV,  SRC_TRV,  BOOK_TRV                                                                                                                             |
| search_travel_modes | enum                  |  Contiene le modalità di ordinamento dei   passaggi cercati.                | BY_PRICE,  BY_RATING                                                                                                                                                                                                            |
| list_t              | typedef struct node * |  Il tipo list_t rappresenta il puntatore alla testa della lista             | struct node *                                                                                                                                                                                                                   |
| node                | struct                |  Il tipo che rappresenta un nodo in una   lista puntata.                    |  ptr: void *,  next: struct node *                                                                                                                                                                                              |
| driver              | struct                | Il tipo che rappresenta un guidatore.                                       |  id: int, age: int,   rating: int, name: char *,   vehicle: char *, token: long                                                                                                                                                 |
| travel              | struct                | Il tipo che rappresenta un passaggio.                                       |  id: int, date: char *,   price: float, destination: char *,   seats: int, token: long                                                                                                                                          |

### Progettazione di librerie e funzioni
Le librerie progettate sono `drivers.h`, `filehandler.h`, `list.h`, `travels.h` e [uTron](https://gitlab.com/NicoNex/utron) per Telegram.

Per una documentazione completa fai riferimento a ciascun header file:
- [drivers.h](https://github.com/NicoNex/kowalski/blob/master/include/drivers.h)
- [filehandler.h](https://github.com/NicoNex/kowalski/blob/master/include/filehandler.h)
- [list.h](https://github.com/NicoNex/kowalski/blob/master/include/list.h)
- [travels.h](https://github.com/NicoNex/kowalski/blob/master/include/travels.h)
