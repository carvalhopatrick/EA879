print("Enter a code between 0 and 127 (inclusive):")
value = io.read()
if value  == "" then
    print("you must enter something!")
else
    value = value + 0
    is_digit = false
    is_letter = false
    symbol_string_msg = 'this is a code for a printable symbol in the ASCII table'
    if value >= 0 and value <= 127 then
        if value < 64 then
            if value < 32 then
                print("this is a non-printable control code in the ASCII table")
            else
                if value < 48 then
                    print(symbol_string_msg)
                elseif value < 58 then
                    print("this is a code for a digit in the ASCII table")
                    is_digit = true
                    digit = value - 48
                else
                    print(symbol_string_msg)
                end
            end
        else
            if value < 96 then
                if value < 65 then
                    print(symbol_string_msg)
                elseif value < 91 then
                    print("this is a code for an upper-case letter in the ASCII table")
                    is_letter = true
                    letter = value - 64
                else
                    print(symbol_string_msg)
                end
            else
                if value < 97 then
                    print(symbol_string_msg)
                elseif value < 123 then
                    print("this is a code for a lower-case letter in the ASCII table")
                    is_letter = true
                    letter = value - 96
                elseif value < 127 then
                    print(symbol_string_msg)
                else
                    print(
[[this is the code for the "delete" control character ('\x7F' or '\127') in the ASCII
table --- curiosity: the delete code is 127, because in binary this number is
represented with all bits set, and in a teletype tape, characters already typed and
punched could be deleted by punching all the other holes in the tape. Neat, ain't it?]])
                end
            end
        end
    else
        print("this code is invalid!")
    end

    if is_digit then
        print("the digit that the code represents is", digit)
    elseif is_letter then
        print("the index of the letter that the code represent in the English alphabet is", letter)
    end
end
