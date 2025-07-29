from copy import deepcopy, copy
import numpy as np
import random
from time import time


class GoBoard:
    def __init__(self,size,verbose=False):
        self.n_move = 0
        self.side = None
        self.board = None
        self.previous_board = None
        self.max_move = 24
        self.size = size
        self.rival_pass = False
        self.double_pass =False
        self.verbose = verbose
        self.komi = 2.5

    def set_side(self,side):
        self.side = side

    def load_from_file(self, host_file, move_file):
        board_info = host_file.readlines()
        side = int(board_info[0].strip())
        self.set_side(side)
        temp_previous_board = [[0 for x in range(self.size)] for y in range(self.size)]
        temp_board = [[0 for x in range(self.size)] for y in range(self.size)]
        for i in range(6,6+self.size):
            a,b,c,d,e = int(board_info[i][0]),int(board_info[i][1]),int(board_info[i][2]),int(board_info[i][3]),int(board_info[i][4])
            temp_board[i-6][0] = a
            temp_board[i-6][1] = b
            temp_board[i-6][2] = c
            temp_board[i-6][3] = d
            temp_board[i-6][4] = e

        cnt = self.count_pieces(temp_board)
        if cnt == 0 and self.side == 1:
            self.n_move = 0
        elif cnt == 1 and self.side == 2:
            self.n_move = 1
        else:
            for i in range(1,self.size+1):
                a, b, c, d, e = int(board_info[i][0]),int(board_info[i][1]),int(board_info[i][2]),int(board_info[i][3]),int(board_info[i][4])
                temp_previous_board[i-1][0] = a
                temp_previous_board[i-1][1] = b
                temp_previous_board[i-1][2] = c
                temp_previous_board[i-1][3] = d
                temp_previous_board[i-1][4] = e
            self.n_move = int(move_file.readline().strip())
            if temp_board != temp_previous_board:
                self.n_move += 1
            else:
                self.rival_pass = True
                self.n_move +=1

        self.board = temp_board
        self.previous_board = temp_previous_board
        host_file.close()
        move_file.close()
        return

    def count_pieces(self,board):
        cnt = 0
        for i in range(self.size):
            for j in range(self.size):
                if board[i][j] != 0:
                    cnt += 1
        return cnt

    def at_edge(self, i, j):
        if i == 0 or i == self.size-1 or j == 0 or j == self.size-1:
            return True
        return False

    def at_corner(self,i,j):
        if i == 0 or i == self.size-1:
            if j == 0 or j == self.size-1:
                return True
        return False

    def detect_neighbor(self, i, j):
        '''
        Detect all the neighbors of a given stone.
        :param i: row number of the board.
        :param j: column number of the board.
        :return: a list containing the neighbors row and column (row, column) of position (i, j).
        '''
        board = self.board
        neighbors = []
        # Detect borders and add neighbor coordinates
        if i > 0: neighbors.append((i-1, j))
        if i < len(board) - 1: neighbors.append((i+1, j))
        if j > 0: neighbors.append((i, j-1))
        if j < len(board) - 1: neighbors.append((i, j+1))
        return neighbors

    def detect_diagonal(self,i,j):
        diagonals = []
        if i>0 and j>0:
            diagonals.append((i-1,j-1))
        if i>0 and j<self.size-1:
            diagonals.append((i-1,j+1))
        if i<self.size-1 and j>0:
            diagonals.append((i+1,j-1))
        if i<self.size-1 and j<self.size-1:
            diagonals.append((i+1,j+1))
        return diagonals

    def is_eyeish(self,i,j,piece_type):
        neighbor = self.detect_neighbor(i, j)
        for pos in neighbor:
            if self.board[pos[0]][pos[1]] != piece_type:
                return False
        return True

    def is_eye(self,i, j, piece_type):
        if not self.is_eyeish(i,j,piece_type):
            return False
        diagonal = self.detect_diagonal(i,j)
        if self.at_edge(i,j):
            for pos in diagonal:
                if self.board[pos[0]][pos[1]] == 3 - piece_type:
                    return False
                elif self.board[pos[0]][pos[1]] and not self.is_eyeish(pos[0],pos[1],piece_type):
                    return False
                return True
        else:
            cnt = 0
            for pos in diagonal:
                if self.board[pos[0]][pos[1]] == piece_type:
                    cnt += 1
                elif self.board[pos[0]][pos[1]] == piece_type and self.is_eyeish([pos[0]][pos[1]],piece_type):
                    cnt +=1
                if cnt > 2:
                    return True
            return False

    def detect_neighbor_ally(self, i, j):
        '''
        Detect the neighbor allies of a given stone.
        :param i: row number of the board.
        :param j: column number of the board.
        :return: a list containing the neighbored allies row and column (row, column) of position (i, j).
        '''
        board = self.board
        neighbors = self.detect_neighbor(i, j)  # Detect neighbors
        group_allies = []
        # Iterate through neighbors
        for piece in neighbors:
            # Add to allies list if having the same color
            if board[piece[0]][piece[1]] == board[i][j]:
                group_allies.append(piece)
        return group_allies

    def ally_dfs(self, i, j):
        '''
        Using DFS to search for all allies of a given stone.
        :param i: row number of the board.
        :param j: column number of the board.
        :return: a list containing the all allies row and column (row, column) of position (i, j).
        '''
        stack = [(i, j)]  # stack for DFS serach
        ally_members = []  # record allies positions during the search
        while stack:
            piece = stack.pop()
            ally_members.append(piece)
            neighbor_allies = self.detect_neighbor_ally(piece[0], piece[1])
            for ally in neighbor_allies:
                if ally not in stack and ally not in ally_members:
                    stack.append(ally)
        return ally_members

    def find_liberty(self, i, j):
        '''
        Find liberty of a given stone. If a group of allied stones has no liberty, they all die.
        :param i: row number of the board.
        :param j: column number of the board.
        :return: boolean indicating whether the given stone still has liberty.
        '''
        board = self.board
        ally_members = self.ally_dfs(i, j)
        for member in ally_members:
            neighbors = self.detect_neighbor(member[0], member[1])
            for piece in neighbors:
                # If there is empty space around a piece, it has liberty
                if board[piece[0]][piece[1]] == 0:
                    return True
        # If none of the pieces in a allied group has an empty space, it has no liberty
        return False

    def find_liberty_for_all(self):
        '''
        Find exact liberty number for every stone on the board
        Return to a liberty board
        -1 means not applicable. Means this is a empty place
        '''
        board = self.board
        liberty_board = [[0 for x in range(self.size)] for y in range(self.size)]
        visited = set()
        for i in range(self.size):
            for j in range(self.size):
                if (i,j) in visited:
                    continue
                if board[i][j] == 0:
                    liberty_board[i][j]=-1
                else:
                    ally_members = self.ally_dfs(i,j)
                    qi = set()
                    for member in ally_members:
                        neighbors = self.detect_neighbor(member[0],member[1])
                        for piece in neighbors:
                            if board[piece[0]][piece[1]] == 0:
                                qi.add(piece)
                    total_qi = len(qi)
                    for member in ally_members:
                        liberty_board[member[0]][member[1]]=total_qi
                        visited.add(member)
        return liberty_board

    def find_died_pieces(self, piece_type):
        board = self.board
        died_pieces = []

        for i in range(len(board)):
            for j in range(len(board)):
                # Check if there is a piece at this position:
                if board[i][j] == piece_type:
                    # The piece die if it has no liberty
                    if not self.find_liberty(i, j):
                        died_pieces.append((i, j))
        return died_pieces

    def remove_died_pieces(self, piece_type):
        '''
        Remove the dead stones in the board.
        :param piece_type: 1('X') or 2('O').
        :return: locations of dead pieces.
        '''
        died_pieces = self.find_died_pieces(piece_type)
        if not died_pieces: return []
        self.remove_certain_pieces(died_pieces)
        return died_pieces

    def remove_certain_pieces(self, positions):
        '''
        Remove the stones of certain locations.
        :param positions: a list containing the pieces to be removed row and column(row, column)
        :return: None.
        '''
        board = self.board
        for piece in positions:
            board[piece[0]][piece[1]] = 0
        self.board = board

    def visualize_board(self,board):
        '''
        Visualize the board.
        :return: None
        '''
        print('-' * len(board) * 2)
        for i in range(len(board)):
            for j in range(len(board)):
                if board[i][j] == 0:
                    print(' ', end=' ')
                elif board[i][j] == 1:
                    print('X', end=' ')
                else:
                    print('O', end=' ')
            print()
        print('-' * len(board) * 2)

    def valid_place_check(self, i, j, piece_type):
        board = self.board
        verbose = False

        # Check if the place is in the board range
        if not (i >= 0 and i < len(board)):
            if verbose:
                print(('Invalid placement. row should be in the range 1 to {}.').format(len(board) - 1))
            return False
        if not (j >= 0 and j < len(board)):
            if verbose:
                print(('Invalid placement. column should be in the range 1 to {}.').format(len(board) - 1))
            return False

        # Check if the place already has a piece
        if board[i][j] != 0:
            if verbose:
                print('Invalid placement. There is already a chess in this position.')
            return False

        # Copy the board for testing
        test_go = deepcopy(self)
        test_board = test_go.board

        # Check if the place has liberty
        test_board[i][j] = piece_type
        test_go.board = test_board
        if test_go.find_liberty(i, j):
            return True

        # If not, remove the died pieces of opponent and check again
        test_go.remove_died_pieces(3 - piece_type)
        if not test_go.find_liberty(i, j):
            if verbose:
                print('Invalid placement. No liberty found in this position.')
            return False

        # Check special case: repeat placement causing the repeat board state (KO rule)
        else:
            # print("Check for KO")
            # print("potential board")
            # self.visualize_board(test_go.board)
            # print("previous board")
            # self.visualize_board(self.previous_board)
            if self.rival_pass:
                # print("Rival Pass")
                return True
            if test_go.board == self.previous_board:
                if verbose:
                    print('Invalid placement. A repeat move not permitted by the KO rule.')
                return False
        return True

    def place_chess(self, i, j, piece_type, check=True):
        board = self.board
        if i == 'P':
            if self.rival_pass:
                self.double_pass = True
            self.previous_board = deepcopy(self).board
            self.rival_pass = True
            self.n_move += 1
            return True
        if check:
            valid_place = self.valid_place_check(i, j, piece_type)
            if not valid_place:
                raise RuntimeError("Invalid Move!!!!")
                return False
        temp = deepcopy(self)
        self.previous_board = temp.board
        # print("Before")
        # self.visualize_board(self.previous_board)
        board[i][j] = piece_type
        self.board = board
        self.remove_died_pieces(3 - piece_type)
        self.n_move += 1
        self.rival_pass = False
        # print("After")
        # self.visualize_board(self.previous_board)
        return True

    def game_end(self,double_pass=False):
        if self.n_move >= self.max_move or double_pass:
            return True
        return False

    def score(self, piece_type):
        '''
        Get score of a player by counting the number of stones.
        :param piece_type: 1('X') or 2('O').
        :return: boolean indicating whether the game should end.
        '''

        board = self.board
        cnt = 0
        for i in range(self.size):
            for j in range(self.size):
                if board[i][j] == piece_type:
                    cnt += 1
        return cnt

    def advanced_evaluation(self,piece_type):
        eyes_info = {}
        board = self.board
        cnt = 0
        factor = 1
        if piece_type == 2:
            factor = 1.2

        for i in range(self.size):
            for j in range(self.size):
                if board[i][j] == 0 and self.is_eye(i, j, piece_type):
                    stone = self.detect_neighbor(i, j)[0]
                    allies = self.ally_dfs(stone[0], stone[1])
                    for single in allies:
                        if single not in eyes_info:
                            eyes_info[single] = 1
                        else:
                            eyes_info[single] += 1
        for element in eyes_info:
            if eyes_info[element] == 1:
                cnt += factor * 1.0 + 1
            if eyes_info[element] > 1:
                cnt += factor * 1.5 + 1

        weight = [-0.5,0.2,0.4,0.6,0.9,1.1,1.2] # more liberty more value, -0.5 for the ones only have 1 qi
        liberty_map = self.find_liberty_for_all()
        for i in range(self.size):
            for j in range(self.size):
                if board[i][j] == piece_type and (i,j) not in eyes_info:
                    qi = liberty_map[i][j]
                    qi = min(7,qi) # maximum number of qi is 8, set 7 weight values
                    cnt += factor * weight[qi-1] + 1
                    if self.at_corner(i,j):
                        cnt -= 0.2
                    elif self.at_edge(i,j):
                        cnt -= 0.1
        return cnt


def find_possible(board, piece_type, max_branching, clean_up=True):
    '''find the possible move of current board'''
    possible_placements = []
    for i in range(board.size):
        for j in range(board.size):
            if board.valid_place_check(i, j, piece_type):
                possible_placements.append((i, j))
    possible_placements.append(('P', 'A'))
    if clean_up:
        # def distance(pos):
        #     return ((pos[0] - 2) ** 2 + (pos[1] - 2) ** 2)
        possible_placements.remove(('P','A'))
        # sorted(possible_placements, key = distance)
        # possible_placements.append(('P', 'A'))
        # return possible_placements

        normal = []
        edge = []
        corner = []
        for element in possible_placements:
            if not board.at_edge(element[0],element[1]):
                normal.append(element)
            elif not board.at_corner(element[0],element[1]):
                edge.append(element)
            else:
                corner.append(element)
        # random.shuffle(normal)
        # random.shuffle(edge)
        # random.shuffle(corner)
        final_possible = normal+edge+corner+[('P','A')]
        return final_possible[:max_branching]

    # print("Possible placement:", possible_placements)
    return possible_placements


class ABGo:
    def __init__(self, max_depth=4, max_branching=25, side=None, load=False, size=5):
        self.size = size
        self.max_depth = max_depth
        self.max_branching = max_branching
        self.side = side
        self.cur_depth = 0
        self.cur_step = 0

    def set_side(self, side):
        self.side = side

    def AB_search(self, board):
        self.cur_step = board.n_move
        if self.cur_step < 10:
            self.max_depth = 3
        elif self.cur_step < 18:
            self.max_depth = 3
        else:
            self.max_depth = 5
        self.cur_depth = 0
        temp_board = deepcopy(board)
        v, place = self.max_value(temp_board, -np.inf, np.inf)
        return place

    def max_value(self, board, alpha, beta):
        # print("Max_value called: ")
        # print("Current depth: ", self.cur_depth)
        # print("Current step: ",self.cur_step)
        komi = -2.5 if self.side == 1 else 2.5

        if board.game_end():
            return 50*(board.score(self.side) - board.score(3 - self.side)+komi), (-1, -1)

        if self.cur_depth >= self.max_depth:
            return board.advanced_evaluation(self.side) - board.advanced_evaluation(3 - self.side), (-1, -1)
            # return 50*(board.score(self.side) - board.score(3 - self.side)+komi), (-1, -1)

        possible_pos = find_possible(board,self.side,self.max_branching,clean_up=True)
        # print("Possible Pos: ",possible_pos)

        v = -np.inf
        max_pos = (-1,-1)
        for pos in possible_pos:
            temp_board = deepcopy(board)
            self.cur_depth += 1
            temp_board.place_chess(pos[0],pos[1],self.side,check=False)
            v = max(v, self.min_value(temp_board, alpha, beta))
            self.cur_depth -= 1
            if v >= beta:
                del temp_board
                return v, max_pos
            if v > alpha:
                max_pos = pos
                alpha = v

        del temp_board
        return v, max_pos

    def min_value(self,board,alpha,beta):
        # print("Min_value called")
        # print("Current depth: ", self.cur_depth)
        # print("Current step: ",self.cur_step)
        komi = -2.5 if self.side ==1 else 2.5
        if board.game_end():
            return 50 * (board.score(self.side) - board.score(3 - self.side) +komi)

        if self.cur_depth >= self.max_depth:
            # return board.weighted_score(self.side) - board.weighted_score(3 - self.side)
            # return 50 * (board.score(self.side) - board.score(3 - self.side) +komi)
            return board.advanced_evaluation(self.side) - board.advanced_evaluation(3 - self.side)

        possible_pos = find_possible(board, 3-self.side, self.max_branching, clean_up=True)
        # print("Possible Pos: ", possible_pos)

        v = np.inf
        for pos in possible_pos:
            temp_board = deepcopy(board)
            self.cur_depth += 1
            temp_board.place_chess(pos[0],pos[1],3-self.side,check=False)
            v_2, dummy = self.max_value(temp_board, alpha, beta)
            self.cur_depth -= 1
            v = min(v, v_2)
            if v <= alpha:
                del temp_board
                return v
            beta = min(beta,v)

        return v

    def move(self,board):
        if board.n_move < 2:
            if board.valid_place_check(2,2,self.side):
                return (2,2)
            if board.valid_place_check(1, 1, self.side):
                return (1,1)
            elif board.valid_place_check(3, 1, self.side):
                return (3,1)
            elif board.valid_place_check(1, 3, self.side):
                return (1,3)
            elif board.valid_place_check(3, 3, self.side):
                return (3,3)
        if board.n_move == 2 and self.side==1:
            if board.board[3][1]==2 or board.board[3][3]==2:
                return (3,2)
            elif board.board[1][1] == 2 or board.board[1][3] ==2:
                return (1,2)

        if board.game_end():
            return
        place = self.AB_search(board)
        return place






def writefile(placement):
    file=open('output.txt','w')
    if placement[0]=='P':
        file.write("PASS")
        file.close()
        return
    file.write("{},{}".format(placement[0],placement[1]))
    file.close()
    
    

#
# infile=open('input.txt','r')
# board = GoBoard(5)
# writefile(ABGo.move(board))



## For test
# board = GoBoard(5)
# board.board = [[0 for x in range(5)] for y in range(5)]
# board.place_chess(2,2,1)
# board.place_chess(2,1,2)
# board.place_chess(3,3,1)
# board.place_chess(3,0,2)
# board.place_chess(4,2,1)
# board.place_chess(4,1,2)
# board.place_chess(3,1,1)
# board.place_chess(3,2,2)
# board.place_chess(3,1,1)
# board.place_chess(3,2,2)


# player = ABGo()
# player.set_side(1)
# board.visualize_board(board.board)
# print(board.game_end())
# move = player.move(board)
# print(move)
# writefile(move)
# movefile = open('move.txt','w')
# movefile.write(str(board.n_move))


## For Homework

import os
if not os.path.exists('move.txt'):
    movefile = open('move.txt','w')
    movefile.write('0')
    movefile.close()

time1=time()
board = GoBoard(5)
infile = open('input.txt','r')
movefile = open('move.txt','r')
board.load_from_file(infile,movefile)
#print(board.n_move)
player = ABGo()
player.set_side(board.side)
writefile(player.move(board))
movefile = open('move.txt','w')
movefile.write(str(board.n_move+1))
time2=time()
# print(time2-time1)