// Cases where ASI doesn't help
a = b + c
(d + e).print()  // Interpreted as: a = b + c(d + e).print()

var foo = "bar"
["red", "green"].foreach(...)  // Interpreted as: foo["red", "green"]