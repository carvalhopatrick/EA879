-- examples from https://en.wikibooks.org/wiki/Lua_Programming/Tables
array = {5, "text", {"more text", 463, "even more text"}, "more text"}
array = {
	"text";
	{
		"more text, in a nested table";
		1432
	};
	true -- booleans can also be in tables
}
dictionary = {
	"text";
	text = "more text";
	654;
	[8] = {}; -- an empty table
	func = function() print("tables can even contain functions!") end;
	["1213"] = 1213
}
dictionary = {number = 6}
print(dictionary["number"])

dictionary = {["number"] = 6}
print(dictionary.number)
nested_table = {
	[6] = {
		func = function() print("6") end
	}
}
nested_table[6].func()

array = {5, 2, 6, 3, 6}
for index, value in next, array do
	print(index .. ": " .. value + 1)
end

local object = {}
function object.func(self, arg1, arg2)
	print(arg1, arg2)
end

local object = {}
function object:func(arg1, arg2)
	print(arg1, arg2)
end

-- examples from https://en.wikibooks.org/wiki/Lua_Programming/Statements

local number = 0
while number < 10 do
	print(number)
	number = number + 1 -- Increase the value of the number by one.
end

local number = 0
repeat
	print(number)
	number = number + 1
until number >= 10

for number = 0, 9, 1 do
	print(number)
end

for n = 1, 2, 0.1 do
	print(n)
	if n >= 1.5 then
		break -- Terminate the loop instantly and do not repeat.
	end
end

