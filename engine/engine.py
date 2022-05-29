from stockfish import Stockfish
import string
import serial
import time
import serial.tools.list_ports as ports

north_limit = 1425
south_limit = 45
ns_square = (north_limit - south_limit) / 7
east_limit = 1975
west_limit = 575
ew_square = (east_limit - west_limit) / 7
next_black_death_pos = [north_limit, (west_limit - ew_square)]
black_death_count = 0
play_against_self = True
move_count = 0


def make_ai_move():
    move = stockfish.get_best_move()
    print("AI move: ")
    print(move)
    instructions = []
    if check_if_move_kills_piece(move):
        instructions += get_death_move(move)
        #send_to_arduino(kill_instruction)
        update_death_placement()
    instructions += convert_move_to_coordinates(move)
    print(instructions)
    send_to_arduino(instructions)
    stockfish.make_moves_from_current_position([move])
    print(stockfish.get_board_visual())


def send_to_arduino(instructions):
    for instruction in instructions:
        instruction = '<' + instruction + '>'
        print(instruction)
        # arduino.write(bytes(instruction, 'utf-8'))
        arduino.write(instruction.encode())
        time.sleep(1)
        response = False
        while response == False:
            response = arduino.readline().decode()
            if len(response) == 0: response = False
        print(response)


def make_player_move():
    playerMove = input('Please Enter Your Move (or 0 for calibration) \n')
    if(playerMove == '0'):
        print("recalibrating, please wait")
        send_to_arduino("4")
        playerMove = input('Please enter your move \n')
    if stockfish.is_move_correct(playerMove):
        stockfish.make_moves_from_current_position([playerMove])
        print(stockfish.get_board_visual())
    else:
        print("invalid move, try again")
        make_player_move()


def get_death_move(move):
    from_square = move[2:4]

    # there are 8 squares north to south and west to east
    ns_from_pos = int(((int(from_square[1]) - 1) * ns_square) + south_limit)
    ew_from_pos = int(int(string.ascii_lowercase.index(from_square[0].lower())) * ew_square + west_limit)

    ns_to_pos = int(next_black_death_pos[0])
    ew_to_pos = int(next_black_death_pos[1])

    print(ns_from_pos)
    print(ew_from_pos)
    print(ns_to_pos)
    print(ew_to_pos)

    return ["1 " + (str(ns_from_pos) + " " + str(ew_from_pos)), "2", "1 " + (str(ns_to_pos) + " " + str(ew_to_pos)),
            "3"]


def convert_move_to_coordinates(move):
    from_square = move[0:2]
    to_square = move[2:4]

    # there are 8 squares north to south and west to east
    ns_from_pos = int(((int(from_square[1]) - 1) * ns_square) + south_limit)
    ew_from_pos = int(int(string.ascii_lowercase.index(from_square[0].lower())) * ew_square + west_limit)

    ns_to_pos = int(((int(to_square[1]) - 1) * ns_square) + south_limit)
    ew_to_pos = int(int(string.ascii_lowercase.index(to_square[0].lower())) * ew_square + west_limit)

    print(ns_from_pos)
    print(ew_from_pos)
    print(ns_to_pos)
    print(ew_to_pos)

    return ["1 " + (str(ns_from_pos) + " " + str(ew_from_pos)), "2", "1 " + (str(ns_to_pos) + " " + str(ew_to_pos)),
            "3"]


def connect_to_arduino():
    relevant_ports = []
    com_ports = list(ports.comports())  # create a list of com ['COM1','COM2']
    for i in com_ports:
        if "usb" in i.device:
            relevant_ports.append(i.device)

    if len(relevant_ports) == 0:
        print("No arduino connection found")
        return;
    if len(relevant_ports) > 1:
        print("choose port (0,1,2...)")
        print(relevant_ports)
        choice = input()
        return relevant_ports[int(choice)]
    else:
        return relevant_ports[0]


def update_death_placement():
    global black_death_count
    global next_black_death_pos
    #print(next_black_death_pos)
    #print(black_death_count)
    black_death_count += 1

    if black_death_count % 8 == 0:
        next_black_death_pos = [north_limit, west_limit - 2 * ew_square]
    else:
        next_black_death_pos = [next_black_death_pos[0] - ns_square, next_black_death_pos[1]]


def check_if_move_kills_piece(move):
    # Get FEN position and split into array
    fen_position = stockfish.get_fen_position()
    arr = fen_position.replace(' ', '/').split('/')

    move_to = move[2:4]

    # convert 8 to 0, 7 to 1, 6 to 2 etc = (8 - x)
    row = arr[8 - (int(move_to[1]))]
    board_column_as_int = int(string.ascii_lowercase.index(move_to[0].lower()))

    x = []
    for y in row:
        if y.isdigit():
            x.extend([None] * int(y))
        else:
            x.append(y)

    if x[board_column_as_int] is not None:
        print("Remove piece")
        return True
    else:
        print("No piece to remove");
        return False


stockfish = Stockfish()
port = connect_to_arduino()
print(port)
arduino = serial.Serial(port=port, baudrate=115200, timeout=1)
time.sleep(1)

while True:
    mode = int(input("Choose your option: \n 0 - play \n 1 - calibrate \n 2 - Manual control \n"))
    if mode == 0:
        while True:
            if play_against_self:
                make_ai_move()
                move_count += 1
                if move_count % 5 == 0:
                    send_to_arduino("4")
            else:
                make_player_move()

            make_ai_move()
            move_count += 1
            if move_count % 5 == 0:
                send_to_arduino("4")

    elif mode == 1:
        send_to_arduino("4")
    elif mode == 2:
        while True:
            instruction = input("instruction: \n 1 x y (move to x,y) \n 2 (grab piece) \n 3 (place piece) \n 4 (calibrate) \n 5 (open and lower) \n 6 (open grabber) \n 7 (close grabber) \n 8 (lower grabber) \n 10 (raise grabber) \n")
            send_to_arduino([instruction])
            #print(instruction)
