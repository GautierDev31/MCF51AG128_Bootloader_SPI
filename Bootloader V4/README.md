
Nom | Commande | Nb de mots | Description
----|--------|------------|-------------
Save_memory_sector | 300 |N | Command + Address MSB + Address LSB + Length + Word MSB[N] + Word LSB[N]
Erase_flash_sector | 305 |4 | Command + Address MSB + Address LSB + Length
Flash_memory_sector | 310 |1 |Command 
Read_memory_word | 400 |5 | Command + Address MSB + Address LSB + Read msb* + Read lsb*
Read_status | 410 |2 | Command + Read*
Read_checksum |420 |2 | Command + Read*
Load_vectors|600| 1 | Command
Jump | 700 |3 | Command + Address MSB + Address LSB

*N'importe quelle valeur possible car c'est la valeur lu qui va être importante.