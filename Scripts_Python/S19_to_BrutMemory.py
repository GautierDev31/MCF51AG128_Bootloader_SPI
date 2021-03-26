
f = open("code.txt", "r")
f2 = open("BrutMemory.txt", "w")
flag = 0
print("Converting S19 file to brut memory...")

# HEADER
f.read(10)
for i in range (115):
    f.read(23)


# MEMORY BLOCK
while(flag != 1):
    f.read(3)
    nb = int(f.read(2), 16)
    nb_caracters = (nb * 2) - 10
    if nb_caracters <= 1 :
        flag = 1
    f.read(8)
    if flag != 1:
        f2.write(f.read(nb_caracters))
    f.read(2)


print("File converted")
f.close()
f2.close()      
