unterminated_string1 = 'this string uses single quotes and can contain "double quotes" and escaped \'single quotes\'

unterminated_string2 = "this string uses single quotes and can contain 'single quotes' and escaped \"double quotes\"

unterminated_string3 = [[this string uses double brackets and can contain both "double quotes" and 'single quotes' unescaped, as well as [single brackets] and, [[double open brackets. It cannot contain double closed brackets, but it can contain unescaped
newlines

unterminated_string4 = [====[this string uses super long brackets and can contain "double quotes", 'single quotes', and a variety of closing brackets:
- ]
- ]]
- ]=]
- ]==]
- ]===]
It can also contain an open bracket as long as the starting one: [====[, but not a closing bracket as long as the starting one.]===]

--[==[ This is a long unterminated comment

It has the same logic as a string using long brackets:
]
]]

]=]
print(string1)
print(string2)
print(string3); print(string4)

-- Those symbols are okay in a comment: | & ! @
"And also inside a string: | & ! @"
but they should result in an error otherwise: | & ! @