Haiku I2C Bus Manager
Questo progetto implementa un bus manager I2C per Haiku OS, fornendo l'infrastruttura di base per le comunicazioni I2C. È progettato per facilitare lo sviluppo di driver per dispositivi I2C specifici su Haiku OS.
Caratteristiche

Implementazione delle operazioni I2C di basso livello (start, stop, invio e ricezione di byte)
Gestione del bus I2C a livello hardware
Interfaccia per lo sviluppo di driver di dispositivi I2C
Logging dettagliato per facilitare il debug

Requisiti

Haiku OS
Accesso ai pin GPIO per SCL e SDA

Compilazione
Per compilare il bus manager, esegui il seguente comando nella directory del progetto:
Copymake
