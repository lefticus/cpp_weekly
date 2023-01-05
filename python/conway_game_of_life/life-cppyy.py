import cppyy
from array import array

cppyy.include("life-cppyy.hpp")

born = cppyy.gbl.std.vector[bool](
    (False, False, False, True, False, False, False, False, False)
)
survives = cppyy.gbl.std.vector[bool](
    (False, False, True, True, False, False, False, False, False)
)

obj = cppyy.gbl.Automata(40, 20, born, survives)

obj.add_glider(cppyy.gbl.Automata.Point(0, 18))

for i in range(10000):
    obj = obj.next()


for y in range(obj.height):
    for x in range(obj.width):
        if obj.get(cppyy.gbl.Automata.Point(x, y)):
            print("X", end="")
        else:
            print(".", end="")
    print()
