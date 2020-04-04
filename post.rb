module AST
  OPS = ['+', '-', '*', '/']
  PREC = {'+' => 10, '-' => 10, '*' => 20, '/' => 20}

  class Node
    attr_reader :lhs, :op, :rhs

    def initialize(lhs, op, rhs)
      @lhs = lhs
      @op = op
      @rhs = rhs
    end

    def to_s
      l = lhs.to_s
      l = '(' + l + ')' if lhs.is_a?(Node) and PREC[lhs.op] < PREC[@op]
      r = rhs.to_s
      r = '(' + r + ')' if rhs.is_a?(Node) and PREC[rhs.op] <= PREC[@op]
      return l + op + r
    end
  end
end

i = 0

all = ARGF.readlines(chomp: true).map do |s|
  i += 1
  STDERR.printf "%d\r", i
  parts = s.split(' = ')
  tokens = parts[1].split(' ')
  stack = []
  for t in tokens
    if AST::OPS.include?(t)
      rhs = stack.pop
      lhs = stack.pop
      node = AST::Node.new(lhs, t, rhs)
      stack.push node
    else
      stack.push t
    end
  end
  node = stack.pop
  [parts[0].to_i, node.to_s]
end

STDERR.puts ''
STDERR.puts 'Sorting ...'

all.sort_by! {|x| x[0]}
all.each do |x|
  puts "#{x[0]} = #{x[1]}"
end