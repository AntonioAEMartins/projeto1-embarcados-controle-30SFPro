import pyautogui
from serial import Serial
import argparse
import time
import logging

class MyControllerMap:
    def __init__(self):
        self.button = {'A': 'L'} # Fast forward (10 seg) pro Youtube

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
        ## Sync protocol
        print("update")
        while self.incoming != b'X':
            self.incoming = self.ser.read()
            logging.debug("Received INCOMING: {}".format(self.incoming))
            print("lendo")

        data = self.ser.read()
        logging.debug("Received DATA: {}".format(data))

        # Botao 1 | Vermelho
        if data == b'1':
            print("datab1")
            logging.info("KEYDOWN A")
            pyautogui.keyDown(self.mapping.button['A'])
        elif data == b'0':
            print("datab0")
            logging.info("KEYUP A")
            pyautogui.keyUp(self.mapping.button['A'])
        # Botao 2 | Verde
        if data == b'3':
            print("datab2")
            logging.info("KEYDOWN A")
            pyautogui.keyDown(self.mapping.button['A'])
        elif data == b'2':
            print("datab0")
            logging.info("KEYUP A")
            pyautogui.keyUp(self.mapping.button['A'])
        # Botao 3 | Azul
        if data == b'5':
            print("datab3")
            logging.info("KEYDOWN A")
            pyautogui.keyDown(self.mapping.button['A'])
        elif data == b'4':
            print("datab0")
            logging.info("KEYUP A")
            pyautogui.keyUp(self.mapping.button['A'])
        # Botao 4 | Amarelo
        if data == b'7':
            print("datab4")
            logging.info("KEYDOWN A")
            pyautogui.keyDown(self.mapping.button['A'])
        elif data == b'6':
            print("datab0")
            logging.info("KEYUP A")
            pyautogui.keyUp(self.mapping.button['A'])
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

    print("Connection to {} using {} interface ({})".format("COM3",args.controller_interface, args.baudrate))
    if args.controller_interface == 'dummy':
        controller = DummyControllerInterface()
    else:
        controller = SerialControllerInterface(port="COM3", baudrate=args.baudrate)

    while True:
        controller.update()
L