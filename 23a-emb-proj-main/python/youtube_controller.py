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
                        'Pause':''} # Fast forward (10 seg) pro Youtube

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
        data = self.ser.read()
        # == Checando Analogico
        estado_analogico = 0
        if data == b'A':
            estado_analogico = 1
        recebido_analogico = []
        while (estado_analogico):
            data = self.ser.read()
            if data == b'A':
                estado_analogico = 0
                break
            recebido_analogico.append(data)
            logging.debug("Received DATA: {}".format(data))

        if len(recebido_analogico) > 0:
            if data == b'1':
                print("datab1")
                logging.info("KEYDOWN A")
                pyautogui.keyDown(self.mapping.button['A'])
            elif data == b'0':
                print("datab0")
                logging.info("KEYUP A")
                pyautogui.keyUp(self.mapping.button['A'])

        # == Checando Botões
        estado = 0
        if data == b'B':
            estado = 1
        recebido = []
        while (estado):
            data = self.ser.read()
            if data == b'B':
                estado = 0
                break
            recebido.append(data)
            logging.debug("Received DATA: {}".format(data))
        if len(recebido) > 0:
            #Buttons Detection
            if recebido[0] == b'1':
                print("datab1")
                logging.info("Button 1")
                pyautogui.keyDown(self.mapping.button['B1'])
            if recebido[1] == b'1':
                print("datab2")
                logging.info("Button 2")
                pyautogui.keyDown(self.mapping.button['B2'])
            if recebido[2] == b'1':
                print("datab3")
                logging.info("Button 3")
                pyautogui.keyDown(self.mapping.button['B3'])
            if recebido[3] == b'1':
                print("datab4")
                logging.info("Button 4")
                pyautogui.keyDown(self.mapping.button['B4'])
            if recebido[4] == b'1':
                print("datab5")
                logging.info("Button 5")
                pyautogui.keyDown(self.mapping.button['B5'])
            if recebido[5] == b'1':
                print("datab6")
                logging.info("Button 6")
                pyautogui.keyDown(self.mapping.button['B6'])
            if recebido[6] == b'1':
                print("datab7")
                logging.info("Button 7")
                pyautogui.keyDown(self.mapping.button['B7'])
            if recebido[7] == b'1':
                print("datab8")
                logging.info("Button 8")
                pyautogui.keyDown(self.mapping.button['B8'])
            if recebido[8] == b'1':
                print("pause")
                logging.info("Pause")
                pyautogui.keyDown(self.mapping.button['B9'])

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
