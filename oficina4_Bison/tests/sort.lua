print("Enter the first string:")
value1 = io.read()
print("Enter the second string:")
value2 = io.read()
print("Enter the third string:")
value3 = io.read()
if value1 <= value2 then
    if value2 <= value3 then
        -- value1 <= value2 <= value3 -- do nothing
    elseif value3 < value1 then
        -- value3 < value1 <= value2
        value1, value2, value3 = value3, value1, value2
    else
        -- value1 <= value3 < value2
        value2, value3 = value3, value2
    end
else
    -- value2 < value1
    if value1 <= value3 then
        -- value2 < value1 <= value3
        value1, value2 = value2, value1
    elseif value3 < value2 then
        -- value3 < value2 < value1
        value1, value2, value3 = value3, value2, value1
    else
        -- value2 <= value3 < value1
        value1, value2, value3 = value2, value3, value1
    end
end
print("The strings in lexicographic order are:")
print(value1)
print(value2)
print(value3)
