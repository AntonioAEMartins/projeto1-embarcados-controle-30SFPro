import pyautogui
from serial import Serial
import argparse
import time
import logging

class MyControllerMap:
    def __init__(self):
        self.button = {'B1': '',
                        'B2':'',
                        'B3':'',
                        'B4':'',
                        'B5': '',
                        'B6':'',
                        'B7':'',
                        'B8':'',
                        'Pause':'KK'} # Fast forward (10 seg) pro Youtube

class SerialControllerInterface:
    # Protocolo
    # byte 1 -> Botão 1 (estado - Apertado 1 ou não 0)
    # byte 2 -> EOP - End of Packet -> valor reservado 'X'

    def __init__(self, port, baudrate):
        self.ser = Serial(port, baudrate=baudrate)
        self.mapping = MyControllerMap()
        self.incoming = '0'
        pyautogui.PAUSE = 0  ## remove delay
    
    def update(self):
        estado = 0
        ## Sync protocol
       # print("update")
        # while self.incoming != b'X':
        #     self.incoming = self.ser.read()
        #     logging.debug("Received INCOMING: {}".format(self.incoming))
        #     print("lendo")
        data = self.ser.read()
        if data == b'X':
            estado = 1
        recebido = []
        while (estado):
            data = self.ser.read()
            if data == b'X':
                estado = 0
                break
            recebido.append(data)
            logging.debug("Received DATA: {}".format(data))
        if len(recebido) > 0:
            print(recebido)
            
        
        # Buttons Detection
        # if data == b'b11':
        #     print("datab1")
        #     logging.info("Button 1")
        #     pyautogui.keyDown(self.mapping.button['B1'])
        # elif data == b'10':
        #     print("datab2")
        #     logging.info("Button 2")
        #     pyautogui.keyDown(self.mapping.button['B2'])
        # elif data == b'11':
        #     print("datab3")
        #     logging.info("Button 3")
        #     pyautogui.keyDown(self.mapping.button['B3'])
        # elif data == b'100':
        #     print("datab4")
        #     logging.info("Button 4")
        #     pyautogui.keyDown(self.mapping.button['B4'])
        # elif data == b'101':
        #     print("datab5")
        #     logging.info("Button 5")
        #     pyautogui.keyDown(self.mapping.button['B5'])
        # elif data == b'110':
        #     print("datab6")
        #     logging.info("Button 6")
        #     pyautogui.keyDown(self.mapping.button['B6'])
        # elif data == b'111':
        #     print("datab7")
        #     logging.info("Button 7")
        #     pyautogui.keyDown(self.mapping.button['B7'])
        # elif data == b'1000':
        #     print("datab8")
        #     logging.info("Button 8")
        #     pyautogui.keyDown(self.mapping.button['B8'])
        # #Buttons Actions
        # elif data == b'p':
        #     print("datapause")
        #     logging.info("Pause")
        #     pyautogui.keyDown(self.mapping.button['Pause'])
        # # Buttons Error Detection
        # elif data == b'e':
        #     print("Erro ao Detectar Botao")
        #     logging.info("Error")

        # elif data == b'0':
        #     print("datab0")
        #     logging.info("KEYUP A")

        self.incoming = self.ser.read()


class DummyControllerInterface:
    def __init__(self):
        self.mapping = MyControllerMap()

    def update(self):
        pyautogui.keyDown(self.mapping.button['A'])
        time.sleep(0.1)
        pyautogui.keyUp(self.mapping.button['A'])
        logging.info("[Dummy] Pressed A button")
        time.sleep(1)


if __name__ == '__main__':
    interfaces = ['dummy', 'serial']
    argparse = argparse.ArgumentParser()
    argparse.add_argument('-b', '--baudrate', type=int, default=115200)
    argparse.add_argument('-c', '--controller_interface', type=str, default='serial', choices=interfaces)
    argparse.add_argument('-d', '--debug', default=False, action='store_true')
    args = argparse.parse_args()
    if args.debug:
        logging.basicConfig(level=logging.DEBUG)

    print("Connection to {} using {} interface ({})".format("COM4",args.controller_interface, args.baudrate))
    if args.controller_interface == 'dummy':
        controller = DummyControllerInterface()
    else:
        controller = SerialControllerInterface(port="COM4", baudrate=args.baudrate)

    while True:
        controller.update()
