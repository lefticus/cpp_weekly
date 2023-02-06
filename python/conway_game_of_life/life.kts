#!/bin/env kotlin
/*
 * To run this, you'll have to have Kotlin installed.  Follow these simple instructions here to install it:
 * https://kotlinlang.org/docs/command-line.html#install-the-compiler
 */

data class Point(val x: Int, val y: Int) {
    operator fun plus(other: Point) = Point(x + other.x, y + other.y)
}

val neighbors = listOf(
    Point(-1, -1),
    Point(0, -1),
    Point(1, -1),
    Point(-1, 0),
    Point(1, 0),
    Point(-1, 1),
    Point(0, 1),
    Point(1, 1),
)

data class Automata(val width: Int, val height: Int, val born: List<Boolean>, val survives: List<Boolean>) {

    private val field = BooleanArray(width * height)

    private fun Point.asIndex() = ((y + height) % height) * width + ((x + width) % width)

    operator fun get(p: Point) = field[p.asIndex()]
    operator fun set(p: Point, value: Boolean) {
        field[p.asIndex()] = value
    }

    private fun countNeighborsOf(p: Point) = neighbors.count { this[it + p] }

    fun next() = copy().apply {
        for (y in 0 until height) {
            for (x in 0 until width) {
                val pt = Point(x, y)
                this[pt] = (if (this@Automata[pt]) survives else born)[this@Automata.countNeighborsOf(pt)]
            }
        }
    }

    fun addGliderAt(p: Point) {
        this[p] = true
        this[p + Point(1, 1)] = true
        this[p + Point(2, 1)] = true
        this[p + Point(0, 2)] = true
        this[p + Point(1, 2)] = true
    }

    override fun toString() = buildString {
        for (y in 0 until height) {
            for (x in 0 until width) {
                append(if (this@Automata[Point(x, y)]) 'X' else '.')
            }
            appendLine()
        }
    }
}

var aut = Automata(
    40,
    20,
    listOf(false, false, false, true, false, false, false, false, false),
    listOf(false, false, true, true, false, false, false, false, false),
)

aut.addGliderAt(Point(0, 18))
repeat(10_000) { aut = aut.next() }

println(aut)
