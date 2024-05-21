import dataclasses
import random
import pygame

@dataclasses.dataclass
class Point:
    x: int
    y: int

    def __add__(self, other):
        return Point(self.x + other.x, self.y + other.y)

@dataclasses.dataclass
class Cell:
    rule_number: int
    value: int
    velocity: Point

class Neighborhood:
    def __init__(self, array, array_width, array_height, center):
        self.array = array
        self.array_width = array_width
        self.array_height = array_height
        self.center = center

    def get(self,p):
        loc = self.center + p
        return self.array[loc.x + (loc.y * self.array_width)]


class Automata:

    def __init__(self, width, height, rules):
        self.width = width
        self.height = height
        self.data = [Cell(0, 0, Point(0,0))] * (width * height)
        self.rules = rules

    def index(self, p: Point):
        return (p.y % self.height) * self.width + (p.x % self.width)

    def get(self, p: Point):
        return self.data[self.index(p)]

    def set(self, p: Point, value: Cell):
        self.data[self.index(p)] = value

    def next(self, skip_borders = False):
        result = Automata(self.width, self.height, self.rules)

        for y in range(0, self.height):
            for x in range(0, self.width):
                p = Point(x, y)
                cell = self.get(p)
                if not skip_borders or (skip_borders and not (y == 0 or x == 0 or y == self.height-1 or x == self.width-1)):
                    result.set(p, self.rules[cell.rule_number](cell, Neighborhood(self.data, self.width, self.height, p)))
                else:
                    result.set(p, cell)



        return result

    def randomize_values(self, value_min, value_max):
        self.data = [Cell(c.rule_number, random.randint(value_min, value_max), Point(0,0)) for c in self.data]





def gravity(cell : Cell, neighbors : Neighborhood):
    if cell.value == 0 and neighbors.get(Point(0, -1)).value == 1:
        return Cell(cell.rule_number, 1)
    if cell.value == 1 and neighbors.get(Point(0, 1)).value == 0:
        return Cell(cell.rule_number, 0)

    return cell

def towards_me(p: Point, v :Point):
    if p.x < 0 and p.y < 0 and v.x >0 and v.y>0:
        return True
    if p.x == 0 and p.y < 0 and v.x == 0 and v.y >0:
        return True
    if p.x > 0 and p.y < 0 and v.x < 0 and v.y > 0:
        return True
    if p.x < 0 and p.y == 0 and v.x > 0 and v.y == 0:
        return True
    if p.x > 0 and p.y == 0 and v.x < 0 and v.y == 0:
        return True
    if p.x < 0 and p.y > 0 and v.x > 0 and v.y < 0:
        return True
    if p.x == 0 and p.y > 0 and v.x == 0 and v.y < 0:
        return True
    if p.x > 0 and p.y > 0 and v.x < 0 and v.y < 0:
        return True

    return False


def fire(cell : Cell, neighbors : Neighborhood):
    falloff = .99
    new_value = int(cell.value * (random.random()*falloff))
    new_velocity = cell.velocity

    for x in [-1,0,1]:
        for y in [-1,0,1]:
            if not (x == 0 and y == 0):
                point = Point(x,y)
                neighbor = neighbors.get(point)

                if towards_me(point, neighbor.velocity):
                    new_value += int(neighbor.value * (random.random()*falloff))
                    #new_velocity += neighbor.velocity
                    new_velocity += Point((neighbor.velocity.x * (random.random()*falloff)), (neighbor.velocity.y * (random.random()*falloff)))


    new_value = new_value if new_value <= 255 else 255

    return Cell(cell.rule_number, new_value, new_velocity)


width = 320
height = 240

obj = Automata(
    width,
    height,
    [fire]
)

obj.randomize_values(0, 255)

#Top and bottom borders
for x in range(0, obj.width):
    obj.set(Point(x, 0), Cell(0, 0, Point(0,0)))
    obj.set(Point(x, obj.height-1), Cell(0, 255, Point(10,-10)))

# Left and right borders
for y in range(0, obj.height):
    obj.set(Point(0, y), Cell(0, 0, Point(0,0)))
    obj.set(Point(obj.width-1, y), Cell(0, 0, Point(0,0)))

for y in range(obj.height-300, obj.height):
    obj.set(Point(obj.width-1, y), Cell(0, 0, Point(-10, 0)))

pygame.init()
screen = pygame.display.set_mode((width, height))

while True:
    pygame.display.get_surface().fill((0,0,0))
    for y in range(0, obj.height):
        for x in range(0, obj.width):
            value = obj.get(Point(x,y)).value
            screen.set_at((x,y), (value, value, value))

    pygame.display.flip()
    obj = obj.next(skip_borders = True)

