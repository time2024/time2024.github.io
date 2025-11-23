// Basic JavaScript syntax
var x = 42;
let y = "hello";
const z = true;

function add(a, b) {
    return a + b;
}

if (x > 10) {
    console.log("x is large");
} else {
    console.log("x is small");
}

while (y < 100) {
    y = y + 1;
}
// 条件语句
function testIfElse(num) {
    if (num > 100) {
        return "large";
    } else if (num > 50) {
        return "medium";
    } else {
        return "small";
    }
}

// 循环语句
function testLoops() {
    let sum = 0;
    let i = 0;
    // for 循环
    for (i = 0; i < 10; i++) {
        sum += i;
    }
    
    // while 循环
    let j = 0;
    while (j < 5) {
        sum += j;
        j++;
    }
    
    // do-while 循环
    let k = 0;
    do {
        sum += k;
        k++;
    } while (k < 3);
    
    return sum;
}
// 对象字面量
let person = {
    name: "John",
    age: 30,
    address: {
        street: "123 Main St",
        city: "Boston"
    },
    hobbies: ["reading", "swimming"],
};

// 数组操作
let numbers = [1, 2, 3, 4, 5];

let a=prompt("请输入一个数字")
if(a>=0){
    alert("数字>=0")
}else{
    alert("数字<0")
}
