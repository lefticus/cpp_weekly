class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def __add__(self, other):
        return Point(self.x + other.x, self.y + other.y)


class Automata:
    neighbors = (
        Point(-1, -1),
        Point(0, -1),
        Point(1, -1),
        Point(-1, 0),
        Point(1, 0),
        Point(-1, 1),
        Point(0, 1),
        Point(1, 1),
    )

    def __init__(self, width, height, born, survives):
        self.width = width
        self.height = height
        self.born = born
        self.survives = survives
        self.data = [False] * (width * height)

    def index(self, p: Point):
        return (p.y % self.height) * self.width + (p.x % self.width)

    def get(self, p: Point):
        return self.data[self.index(p)]

    def set(self, p: Point):
        self.data[self.index(p)] = True

    def count_neighbors(self, p: Point):
        return sum(1 for loc in self.neighbors if self.get(loc + p))

    def next(self):
        result = Automata(self.width, self.height, self.born, self.survives)

        for y in range(0, self.height):
            for x in range(0, self.width):
                p = Point(x, y)
                neighbors = self.count_neighbors(p)
                if self.get(p):
                    if self.survives[neighbors]:
                        result.set(p)
                else:
                    if self.born[neighbors]:
                        result.set(p)

        return result

    def add_glider(self, p: Point):
        self.set(p)
        self.set(p + Point(1, 1))
        self.set(p + Point(2, 1))
        self.set(p + Point(0, 2))
        self.set(p + Point(1, 2))


obj = Automata(
    40,
    20,
    (False, False, False, True, False, False, False, False, False),
    (False, False, True, True, False, False, False, False, False),
)

obj.add_glider(Point(0, 18))

for i in range(0, 10000):
    obj = obj.next()

for y in range(0, obj.height):
    for x in range(0, obj.width):
        if obj.get(Point(x, y)):
            print("X", end="")
        else:
            print(".", end="")
    print()
