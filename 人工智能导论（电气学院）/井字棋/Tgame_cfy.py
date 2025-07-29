"""
井字棋AI算法说明

一、概述
本程序实现了一个基于完全博弈树搜索的井字棋AI，核心策略包括：
1. 预计算全局博弈树 - 通过Minimax算法预先计算所有可能局面的最优决策
2. 快速胜负检测 - 优先处理必胜走法和关键封堵
3. 内存缓存优化 - 使用字典缓存博弈树节点，避免重复计算

二、关键策略
1. 直接胜负检测（find_winning_move）
   - 优先检查是否存在立即获胜的走法
   - 检测对手的必胜走法并进行封堵

2. Minimax博弈树（minimax_tree）
   - 使用递归实现深度优先搜索
   - 价值评估：
     * AI获胜 = 1
     * 对手获胜 = -1
     * 平局 = 0
   - 极大节点（AI回合）选择最大价值分支
   - 极小节点（对手回合）选择最小价值分支

3. 双博弈树结构（TREE_AI_FIRST/TREE_AI_SECOND）
   - 预先生成先手/后手两棵完整博弈树
   - 先手树：AI为玩家1，初始空棋盘
   - 后手树：AI为玩家2，对手先走

三、实现要点
1. 状态表示
   - 博弈树节点用字典结构存储：
     {
         "state": 棋盘状态,
         "value": 评估值,
         "children": [子节点列表]
     }

2. 决策流程（ai_move）
   (1) 检测必胜走法 → (2) 检测必封锁 → (3) 博弈树查询 → (4) 随机应变（random）
   通过四层递进策略保证最优决策

3. 性能优化
   - 状态缓存：minimax_cache字典缓存已计算节点
   - 快速剪枝：优先处理必胜/必封走法
   - 树节点复用：对局中通过find_child_node快速定位子树

四、测试验证
通过2000局测试（1000先手+1000后手）验证
    - 理论上先手可能必胜，后手的任务是尽力拖到平局
    - 经过调试，目前先手胜率基本维持99%+；后手的胜率不稳定，但没出现过负率。
"""

import numpy as np
import random


# ------------------ 基本函数 --------------------------

# 创建棋盘
def create_board():
    return np.zeros((3, 3), dtype=int)


# 获取合法走子
def get_legal_moves(board):
    return [(i, j) for i in range(3) for j in range(3) if board[i, j] == 0]


# 检查胜负情况
def check_winner(board):
    for i in range(3):
        if board[i, 0] == board[i, 1] == board[i, 2] != 0:
            return board[i, 0]
        if board[0, i] == board[1, i] == board[2, i] != 0:
            return board[0, i]
    if board[0, 0] == board[1, 1] == board[2, 2] != 0:
        return board[0, 0]
    if board[0, 2] == board[1, 1] == board[2, 0] != 0:
        return board[0, 2]
    if not np.any(board == 0):
        return -1  # 平局
    return 0  # 继续游戏


# random对手
def random_move(board):
    moves = get_legal_moves(board)
    return random.choice(moves) if moves else None


# ------------------ 直接获胜或封堵 --------------------------
# 该部分用于寻找“必胜”或“必须阻止对手必胜”的落子
def find_winning_move(board, player):
    """
    对每个合法落子进行模拟：
      - 对于当前玩家（player），尝试在每个合法位置落子
      - 若模拟后能立刻获胜（check_winner返回当前玩家标记），则返回该移动
    该策略用于：
      1. 当有直接获胜的机会时，立即落子终结游戏
      2. 当对手有必胜走法时，采取封堵措施
    """
    for move in get_legal_moves(board):
        temp_board = board.copy()  # 复制棋盘，避免影响实际局面
        temp_board[move] = player  # 模拟在该位置落子
        if check_winner(temp_board) == player:
            return move  # 如果该步能直接获胜，则返回该落子位置
    return None


# ------------------ 完全博弈树（Minimax） --------------------------

# 该树用于在任意局面下预先计算出最佳走法，从而保证 AI 总能做出最优决策
minimax_cache = {}  # 全局缓存字典，避免重复计算相同局面，提高效率


def minimax_tree(board, current_player, ai_marker, opp_marker):
    """
    利用Minimax算法递归构造完整博弈树，并计算每个局面的最优价值
    参数说明：
      - board: 当前棋盘局面（NumPy 数组）
      - current_player: 当前该哪方落子
      - ai_marker: AI的棋子标记（1或2）
      - opp_marker: 对手的棋子标记（1或2）
    处理思路：
      1. 将当前棋盘状态转换成元组形式（用于缓存键）
      2. 如果该局面在缓存中，直接返回之前计算结果
      3. 调用 check_winner 判断局面是否结束：
         - 若结束，则返回对应的价值：AI 赢返回 1，对手赢返回 -1，平局返回 0
      4. 若游戏未结束，则递归遍历所有合法走法：
         - 当当前玩家为 AI（ai_marker）时，为极大化节点，选择最大价值
         - 当当前玩家为对手时，为极小化节点，选择最小价值
      5. 记录每个走法对应的子节点，并将当前局面的最优值及子树结构返回
    """
    # 将棋盘状态展平并转换为元组，作为缓存的键
    state = tuple(board.flatten().tolist())
    key = (state, current_player, ai_marker, opp_marker)
    if key in minimax_cache:
        return minimax_cache[key]

    # 判断当前局面是否结束
    winner = check_winner(board)
    if winner != 0:
        if winner == ai_marker:
            result = {"state": list(state), "value": 1, "children": []}
        elif winner == opp_marker:
            result = {"state": list(state), "value": -1, "children": []}
        elif winner == -1:  # 平局
            result = {"state": list(state), "value": 0, "children": []}
        minimax_cache[key] = result
        return result

    # 如果游戏未结束，则遍历所有可能的落子
    children = []
    if current_player == ai_marker:
        # 当前为 AI 回合：极大化策略
        best_value = -float('inf')
        for move in get_legal_moves(board):
            new_board = board.copy()
            new_board[move] = current_player
            # 递归计算对手回合的局面价值
            child = minimax_tree(new_board, opp_marker, ai_marker, opp_marker)
            children.append({"move": move, "node": child})
            best_value = max(best_value, child["value"])
        result = {"state": list(state), "value": best_value, "children": children}
    else:
        # 当前为对手回合：极小化策略
        best_value = float('inf')
        for move in get_legal_moves(board):
            new_board = board.copy()
            new_board[move] = current_player
            # 递归计算 AI 回合的局面价值
            child = minimax_tree(new_board, ai_marker, ai_marker, opp_marker)
            children.append({"move": move, "node": child})
            best_value = min(best_value, child["value"])
        result = {"state": list(state), "value": best_value, "children": children}

    # 将计算结果保存到缓存中，避免重复计算
    minimax_cache[key] = result
    return result


def find_child_node(tree, move):
    """
    根据给定的走法，从当前博弈树中查找对应的子节点
    - tree 为当前局面的博弈树（字典结构），其中包含所有可能的后续走法
    - 若找到匹配的走法，则返回对应的子树；否则返回 None
    """
    for child in tree.get("children", []):
        if child["move"] == move:
            return child["node"]
    return None


def ai_move_tree(current_tree):
    """
    从当前局面对应的博弈树中选择最佳走法
    - 遍历当前局面的所有子节点，选择使得局面价值最大的移动
    - 返回最佳走法坐标
    """
    best_value = -float('inf')
    best_move = None
    for child in current_tree.get("children", []):
        if child["node"]["value"] > best_value:
            best_value = child["node"]["value"]
            best_move = child["move"]
    return best_move


def ai_move(board, current_tree, ai_marker, opp_marker):
    """
    AI 走棋决策函数
    思路：
      1. 优先检查是否存在直接获胜的走法（能立刻使 AI 获胜）
      2. 再检查是否需要封堵对手必胜的走法
      3. 若上述均不存在，则利用当前局面对应的博弈树选择最佳走法
      4. 若博弈树中未能选择走法，则采取随机落子
    参数：
      - board: 当前棋盘局面
      - current_tree: 当前局面在全局博弈树中的子树
      - ai_marker: AI 的棋子标记
      - opp_marker: 对手的棋子标记
    """
    # 检查是否存在直接获胜的机会
    win_move = find_winning_move(board, ai_marker)
    if win_move is not None:
        return win_move
    # 检查是否需要封堵对手的必胜走法
    block_move = find_winning_move(board, opp_marker)
    if block_move is not None:
        return block_move
    # 利用博弈树选择最佳走法
    move = ai_move_tree(current_tree)
    if move is not None:
        return move
    # 若以上均无效，则随机落子（一般不可能出现这种情况）
    return random_move(board)


# ------------------ 输出函数 --------------------------

def print_board(board):
    """
    打印当前棋盘状态
    - 利用字典将数字转换为棋子符号（0: 空，1: X，2: O）
    - 同时打印行号和列号，便于观察棋局
    """
    symbols = {0: ' ', 1: 'X', 2: 'O'}
    print("  0 1 2")
    for i in range(3):
        row = f"{i} " + "|".join(symbols[board[i, j]] for j in range(3))
        print(row)
        if i < 2:
            print("  -----")
    print()


def print_results(results, title):
    """
    打印对局统计结果
    - results 是包含胜、负、平局数目的字典
    - title 为结果标题
    """
    total = sum(results.values())
    print(f"\n===== {title} =====")
    print(f"对局总数: {total}")
    print(f"胜率: {results['win'] / total:.1%}")
    print(f"负率: {results['lose'] / total:.1%}")
    print(f"平局率: {results['draw'] / total:.1%}")


# ------------------ 构造全局博弈树 --------------------------
def build_game_trees():
    """
    构造两个全局博弈树：
      1. 当 AI 先手时的博弈树（AI为1，对手为2，从初始局面开始）
      2. 当 AI 后手时的博弈树（对手先手为1，AI为2，从初始局面开始）
    这样在模拟对局时可以根据 AI 先后手情况选择不同的预计算决策树
    """
    board = create_board()
    # AI 先手：当前先手玩家为 1
    tree_first = minimax_tree(board, current_player=1, ai_marker=1, opp_marker=2)
    # AI 后手：对手先手（玩家 1），AI 为 2
    tree_second = minimax_tree(board, current_player=1, ai_marker=2, opp_marker=1)
    return tree_first, tree_second


# 全局构造两棵博弈树，便于后续对局时直接调用
TREE_AI_FIRST, TREE_AI_SECOND = build_game_trees()


# ------------------ 对战模拟 --------------------------
def run_test(num_games=1000, ai_first=True):
    """
    模拟多局对战，并统计 AI 的胜、负、平局数目
    参数说明：
      - num_games: 对局数量
      - ai_first: 若为 True，则 AI 先手（使用标记 1），否则 AI 后手（使用标记 2）
    对局过程：
      1. 根据 ai_first 参数确定 AI 和对手的棋子标记及初始回合
      2. 每局开始时，重置棋盘和当前博弈树指针
      3. 在每步落子后，根据走法更新博弈树（即找到对应的子节点），保证决策处于正确的分支
      4. 游戏结束后根据最终结果更新统计
    """
    results = {'win': 0, 'lose': 0, 'draw': 0}
    if ai_first:
        ai_marker = 1
        opp_marker = 2
    else:
        ai_marker = 2
        opp_marker = 1

    for _ in range(num_games):
        board = create_board()
        # 根据 AI 是否先手选择对应的全局博弈树
        current_tree = TREE_AI_FIRST if ai_first else TREE_AI_SECOND
        # 重置当前回合：
        # - 如果 AI 先手，则当前回合为 AI（ai_marker）
        # - 如果 AI 后手，则先由对手走棋（opp_marker）
        current_turn = ai_marker if ai_first else opp_marker

        while True:
            if current_turn == ai_marker:
                move = ai_move(board, current_tree, ai_marker, opp_marker)
                if move is None:
                    break
                board[move] = ai_marker
            else:
                move = random_move(board)
                if move is None:
                    break
                board[move] = opp_marker

            # 检查当前落子后游戏是否结束（有胜者或平局）
            winner = check_winner(board)
            if winner != 0:
                break

            # 根据本次走法在博弈树中寻找对应的子树
            next_tree = find_child_node(current_tree, move)
            if next_tree is None:
                # 理论上不会发生，因为博弈树应覆盖所有合法走法；若出现则结束本局
                break
            current_tree = next_tree
            # 切换回合：交替进行
            current_turn = ai_marker if current_turn == opp_marker else opp_marker

        # 根据最终棋盘结果更新统计数据
        final_winner = check_winner(board)
        if final_winner == ai_marker:
            results['win'] += 1
        elif final_winner == opp_marker:
            results['lose'] += 1
        else:
            results['draw'] += 1
    return results


# ------------------ 主程序 --------------------------
if __name__ == '__main__':
    # 运行 AI 先手测试：AI 为先手，使用 TREE_AI_FIRST
    first_hand_results = run_test(num_games=1000, ai_first=True)
    print_results(first_hand_results, "AI先手测试结果")
    # 运行 AI 后手测试：AI 为后手，使用 TREE_AI_SECOND
    second_hand_results = run_test(num_games=1000, ai_first=False)
    print_results(second_hand_results, "AI后手测试结果")
