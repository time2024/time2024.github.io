// ASI with return statement
function test1() {
    return
    42;  // Should return undefined, not 42
}

function test2() {
    return 42;  // Should return 42
}

function test3() {
    var x = 1
    var y = 2  // ASI inserts semicolon
    return x + y
}