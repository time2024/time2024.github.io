// ASI between statements
var a = 1
var b = 2
var c = 3

// ASI with expressions
a = b + c
a++
b--

// ASI with break/continue
for (var i = 0; i < 10; i++) {
    if (i === 5) {
        continue
    }
    if (i === 8) {
        break
    }
}