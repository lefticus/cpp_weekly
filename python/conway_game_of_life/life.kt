/*
 * Same as script version (.kts), but this file is ready to be compiled into JAR first (so running time does not include
 * compilation).  As with the script version, Kotlin is needed; install it by following the instructions from here:
 * https://kotlinlang.org/docs/command-line.html#install-the-compiler
 *
 * Then compile into JAR by executing
 *
 *     $ kotlinc life.kt -include-runtime -d life.jar
 *
 * After that, the JAR can be run as any other JAR would:
 *
 *     $ java -jar life.jar
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

fun main() {
    var aut = Automata(
        40,
        20,
        listOf(false, false, false, true, false, false, false, false, false),
        listOf(false, false, true, true, false, false, false, false, false),
    )

    aut.addGliderAt(Point(0, 18))
    repeat(10_000) { aut = aut.next() }

    println(aut)
}
