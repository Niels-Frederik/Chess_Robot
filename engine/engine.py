from stockfish import Stockfish
import string
import serial
import time
import serial.tools.list_ports as ports

ns_revolutions = 1200
ew_revolutions = 1200
chess_square_revolutions = 20

def make_ai_move():
    move = stockfish.get_best_move()
    instructions = convert_move_to_coordinates(move)
    print(instructions)
    send_to_arduino(instructions)
    stockfish.make_moves_from_current_position([move])
    print(stockfish.get_board_visual())

def send_to_arduino(instructions):
    for instruction in instructions:
        instruction = '<' + instruction + '>'
        #print(instruction)
        #arduino.write(bytes(instruction, 'utf-8'))
        arduino.write(instruction.encode())
        time.sleep(1)
        response = False
        while(response == False):
            response = arduino.readline().decode()
            if (len(response) == 0): response = False
        print(response)

def make_player_move():
    playerMove = input('Please Enter Your Move \n')
    if(stockfish.is_move_correct(playerMove)):
        stockfish.make_moves_from_current_position([playerMove])
        print(stockfish.get_board_visual())
    else:
        print("invalid move, try again")
        make_player_move()

def convert_move_to_coordinates(move):
    from_square = move[0:2]
    to_square = move[2:4]

    #there are 8 squares north to south and west to east
    ns_from_pos = int((int(from_square[1]) / 8) * ns_revolutions)
    ew_from_pos = int((int(string.ascii_lowercase.index(from_square[0].lower())) / 8) * ew_revolutions)

    ns_to_pos = int((int(to_square[1]) / 8) * ns_revolutions)
    ew_to_pos = int((int(string.ascii_lowercase.index(to_square[0].lower())) / 8) * ew_revolutions)

    return["1 " + (str(ns_from_pos) + " " + str(ew_from_pos)), "2", "1 " + (str(ns_to_pos) + " " + str(ew_to_pos)), "3"]

def connect_to_arduino():
    connection_port = "";
    relevant_ports = []
    com_ports = list(ports.comports()) # create a list of com ['COM1','COM2']
    for i in com_ports:
        if("usb" in i.device):
            relevant_ports.append(i.device)

    if(len(relevant_ports) > 1):
        print("choose port (0,1,2...)")
        print(relevant_ports)
        choice = input()
        return relevant_ports[int(choice)]
    else:
        return (relevant_ports[0])


stockfish = Stockfish()
port = connect_to_arduino()
print(port)
arduino = serial.Serial(port=port, baudrate=115200, timeout=1)
time.sleep(1)

while(True):
    mode = int(input("Choose your option: \n 0 - play \n 1 - calibrate \n"))

    if(mode == 0):
        while(True):
            make_ai_move()
            make_player_move()
    elif(mode == 1):
        send_to_arduino("4")
