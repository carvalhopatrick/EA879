string1 = 'this string uses single quotes and can contain "double quotes" and escaped \'single quotes\''
string2 = "this string uses single quotes and can contain 'single quotes' and escaped \"double quotes\""
string3 = [[this string uses double brackets and can contain both "double quotes" and 'single quotes' unescaped, as well as [single brackets]. It cannot contain double closed brackets, but it can contain unescaped
newlines]]
string4 = [====[this string uses super long brackets and can contain "double quotes", 'single quotes', and a variety of closing brackets:
- ]
- ]]
- ]=]
- ]==]
- ]===]
It can also contain an open bracket as long as the starting one: [====[, but not a closing bracket as long as the starting one.]====]
-- This is a short comment, it ends at newline
--[=[ This is a long comment

It has the same logic as a string using long brackets:
]
]]

]=]
print(string1)
print(string2)
print(string3); print(string4)
