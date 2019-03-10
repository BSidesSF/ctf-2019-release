WINS = [
  [0, 1, 2],
  [3, 4, 5],
  [6, 7, 8],

  [0, 3, 6],
  [1, 4, 7],
  [2, 5, 8],

  [0, 4, 8],
  [2, 4, 6]
]

HUMAN = 'X'
COMPUTER = 'O'
WIN = 1
LOSS = -1
DRAW = 0

def open_moves(board)
  return board.select do |x|
    x == ' '
  end.count
end

def swap(p)
  return p == COMPUTER ? HUMAN : COMPUTER
end


def try(board, c, player)
  test = board.clone()
  test[c] = player
  return test
end

def next_move(board, player=COMPUTER, depth = 0)
  wins = []
  losses = []
  draws = []

  0.upto(8) do |i|
    # Ignore bad moves
    if(board[i] != ' ')
      next
    end

    # Create a new board
    test = try(board, i, player)

    # Check if it's over
    board_state, winner = Board.state(test)

    if(board_state == WIN && winner == player)
      wins << i
    elsif(board_state == WIN && winner != player)
      losses << i
    elsif(board_state == DRAW)
      draws << i
    else
      result, _ = next_move(test, swap(player), depth + 1)
      if(result == WIN)
        losses << i
      elsif(result == LOSS)
        wins << i
      elsif(result == DRAW)
        draws << i
      else
        puts("Unknown result: #{result}")
      end
    end
  end

  if(wins.length > 0)
    return WIN, wins.shuffle()[0]
  elsif(draws.length > 0)
    return DRAW, draws.shuffle()[0]
  else
    return LOSS, losses.shuffle()[0]
  end
end

def out(board)
  puts(" %c | %c | %c" % [board[0], board[1], board[2]])
  puts("---+---+---")
  puts(" %c | %c | %c" % [board[3], board[4], board[5]])
  puts("---+---+---")
  puts(" %c | %c | %c" % [board[6], board[7], board[8]])
end

class Board
  attr_accessor :board

  def initialize()
    @board = [' '] * 9
  end

  def computer_play()
    _, move = next_move(@board)
    if(move)
      @board[move] = COMPUTER
    end
  end

  def play(space)
    if(space < 0 || space > 8)
      raise(Exception, "Impossible space!")
    end

    @board[space] = HUMAN
  end

  def self.state(board)
    # Check for a win
    WINS.each do |w|
      if(board[w[0]] == board[w[1]] && board[w[0]] == board[w[2]])
        if(board[w[0]] == 'X' || board[w[0]] == 'O')
          return WIN, board[w[0]]
        end
      end
    end

    if(open_moves(board) == 0)
      return DRAW, nil
    end

    return nil
  end
end

if __FILE__ == $0
  board = Board.new
  out(board.board)

  loop do
    board_state, winner = Board.state(board.board)

    if(board_state == WIN)
      puts("WINNER: #{winner}")
      exit
    end

    if(board_state == DRAW)
      puts("DRAW!")
      exit
    end

    print "Move -> "
    move = gets().to_i
    #move = 0

    board.play(move)
    out(board.board)

    board.computer_play()
    out(board.board)
  end
end
