// Function declarations
function regular() {
    return 1;
}

async function asyncFunc() {
    return 2;
}

// Nested functions
function outer() {
    function inner() {
        return 42;
    }
    return inner();
}

// 递归版求最大公约数（GCD）
function gcd(a, b) {
  a = Math.abs(a);
  b = Math.abs(b);

  // 递归终止条件
  if (b === 0) return a;

  // 递归调用
  return gcd(b, a % b);
}

// 示例
console.log(gcd(48, 18));   // 6
console.log(gcd(100, 25));  // 25
console.log(gcd(-24, 60));  // 12
console.log(gcd(0, 7));     // 7
console.log(gcd(0, 0));     // 0（约定：两个 0 的 gcd 为 0）

