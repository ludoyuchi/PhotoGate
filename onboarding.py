import serial
import time

# Configure a porta serial com a mesma configuração do Arduino
serial_port = "COM1"
baud_rate = 9600
ser = serial.Serial(serial_port, baud_rate)

# As informações sobre o comprimento do carro, comprimento da pista e número de voltas serão fornecidas pelo usuário
Length = float(input("Comprimento do carro:"))
Track = float(input("Comprimento da pista:"))
N = int(input("Quantas voltas serão dadas:"))
avgs = []
insts = []

# Receber os dados de tempo enviados pelo Arduino
for i in range(N):
    avgtime = float(ser.readline().decode().strip())
    avgs.append(Track/avgtime)
    insttime = float(ser.readline().decode().strip())
    insts.append(Length/insttime)

def slowprint(texto, atraso=0.02):
    for c in texto:
        print(c, end="", flush=True)
        time.sleep(atraso)

print(2 * '\n')
a = 1
for i in avgs:
    slowprint(f"A velocidade média da {a}º volta foi de {round(i*3.6, 2)} km/h")
    a += 1
    print('\n')

b = 1
for i in insts:
    slowprint(f"A velocidade instantânea da {b}º volta foi de {round(i*3.6, 2)} km/h")
    b += 1
    print('\n')

avgs.sort()
insts.sort()
slowprint(f"A maior velocidade média foi: {round(avgs[-1]*3.6, 2)} km/h")
print('\n')
slowprint(f"A maior velocidade instantânea foi: {round(insts[-1]*3.6, 2)} km/h")
print('\n')
