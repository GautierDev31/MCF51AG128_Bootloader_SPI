import spidev
import time

spi = spidev.SpiDev()
spi.open(0, 0)
spi.mode = 0b01
spi.max_speed_hz = 976000
f = open("BrutMemory.txt", "r")
i = 1
value_from_spi = []

def SPI_send_return_input(input, last_send):
    msb = input >> 8
    lsb = input & 0xFF
    response = spi.xfer([msb, lsb])
    Value = response[1] + (response[0] << 8);
    if (Value != last_send) :
        print ("Error")
        print("Expectect value :", last_send)
        print("Value received :",Value)  
        print ("---------")
        
        
def SPI_send(input):
    msb = input >> 8
    lsb = input & 0xFF
    response = spi.xfer([msb, lsb])
    Value = response[1] + (response[0] << 8);
    print("Value sended : ", input)
    print("Response :", Value)
        
def SPI_check_memory(length):
    Errors_check_memory = 0
    cmd = 81
    msb = cmd >> 8
    lsb = cmd & 0xFF
    response = spi.xfer([msb, lsb])
    
    for i in range (length):
        if (i < 2):
            cmd = 50
            msb = cmd >> 8
            lsb = cmd & 0xFF
            response = spi.xfer([msb, lsb])
            Value = response[1] + (response[0] << 8);
            value_from_spi.append(Value)
            
        else :
            cmd = 100
            msb = cmd >> 8
            lsb = cmd & 0xFF
            response = spi.xfer([msb, lsb])
            Value = response[1] + (response[0] << 8);
            value_from_spi.append(Value)
            # Reading brut memory
            Brut_memory = f.read(4)
            Brut_memory_int = int(Brut_memory, 16)
            # Comparaison
            if (Brut_memory_int != value_from_spi[i]):
                print("Error")
                print("ID :", i)
                print("Expectect value :", Brut_memory_int)
                print("Value received :",value_from_spi[i])            
                print("----")
                Errors_check_memory += 1

            
    print("CHECK MEMORY")
    print(Errors_check_memory , "errors")
    cmd = 81
    msb = cmd >> 8
    lsb = cmd & 0xFF
    response = spi.xfer([msb, lsb])
    Value = response[1] + (response[0] << 8);
    print("End check memory")

def SPI_write_memory(length):
    
    cmd = 80
    msb = cmd >> 8
    lsb = cmd & 0xFF
    response = spi.xfer([msb, lsb])
    
    cmd = length
    msb = cmd >> 8
    lsb = cmd & 0xFF
    response = spi.xfer([msb, lsb])
    
    for i in range (length):
            Brut_memory = f.read(4)
            Brut_memory_int = int(Brut_memory, 16)
            cmd = Brut_memory_int
            msb = cmd >> 8
            lsb = cmd & 0xFF
            response = spi.xfer([msb, lsb])
            Value = response[1] + (response[0] << 8);
            value_from_spi.append(Value)
            print("Value sended : ", cmd)
            print("Response :", Value)
            time.sleep(0.01)
    cmd = 81
    msb = cmd >> 8
    lsb = cmd & 0xFF
    response = spi.xfer([msb, lsb])
            
    
while True :
    print("----Commands-----")
    print("1: Write memory")
    print("2: Jump")
    cmd = input()
    
    if(cmd == "1"):
        print("Length of 16 bits words :")
        length = input()
        SPI_write_memory(int(length))
    
    if(cmd == "2"):
        SPI_send(120)
        
